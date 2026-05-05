/*============================================
  Blackwater Tank Level Monitor
  Hardware:  ESP32 + TFmini-S LiDAR
  Framework: SensESP 3.2.0 -> Signal K
  Display:   OLED SSD1306 128x64 (Software I2C)
  Features:  Relay output with hysteresis, emergency mode
============================================*/

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <memory>
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp_app_builder.h"
#include "sensesp/signalk/signalk_metadata.h"
#include "sensesp/ui/config_item.h"
#include "sensesp/sensors/constant_sensor.h"
#include "sensesp/transforms/lambda_transform.h"

using namespace sensesp;

// ============================================================
// PIN ASSIGNMENTS
// ============================================================
#define TF_RX         16   // ESP32 RX2  <- TFmini-S TX
#define TF_TX         17   // ESP32 TX2  -> TFmini-S RX
#define ALARM_PIN     23   // Relay output (HW-482 module, active-high)
#define EMERGENCY_PIN 19   // Emergency input (LOW = active, internal pull-up enabled)

// ============================================================
// OLED DISPLAY (SSD1306, Software I2C)
// Uses dedicated pins 32/33 to avoid bus conflict with
// SensESP hardware I2C on GPIO 21/22.
// ============================================================
#define OLED_SCL 33
#define OLED_SDA 32
U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);

// ============================================================
// TFmini-S PROTOCOL CONSTANTS
// ============================================================
const uint8_t TFMINI_HEADER     = 0x59;  // Frame start byte (appears twice)
const int     TFMINI_FRAME_SIZE = 9;     // Total bytes per frame

// ============================================================
// TANK CONFIGURATION
// All values are configurable via the SensESP web UI at
// http://blackwater-tank.local and are persisted in flash.
// ============================================================
float cfg_length     = 50.0f;   // Tank internal length (cm)
float cfg_width      = 72.0f;   // Tank internal width (cm)
float cfg_height     = 87.0f;   // Tank internal height (cm)
float cfg_offset     =  5.0f;   // Distance from sensor face to tank top edge (cm)
float cfg_alarm_high = 85.0f;   // Relay ON threshold  (% fill level)
float cfg_alarm_low  = 75.0f;   // Relay OFF threshold (% fill level, hysteresis)

// ============================================================
// TANK GEOMETRY HELPERS
// ============================================================

/** Returns total tank capacity in litres based on current config. */
float tank_capacity_liters() {
  return cfg_length * cfg_width * cfg_height / 1000.0f;
}

/** Returns total tank capacity in cubic metres based on current config. */
float tank_capacity_m3() {
  return tank_capacity_liters() / 1000.0f;
}

// ============================================================
// SIGNAL K OUTPUT HANDLES
// Initialised in setup(), written from TFminiSensor::update().
// ============================================================
SKOutputFloat* sk_level;     // tanks.blackWater.0.currentLevel  [ratio 0..1]
SKOutputFloat* sk_capacity;  // tanks.blackWater.0.capacity       [m³]
SKOutputFloat* sk_volume;    // tanks.blackWater.0.currentVolume  [m³]

// ============================================================
// SHARED DISPLAY STATE
// Written by TFminiSensor::update(), read by updateDisplay().
// ============================================================
int   disp_percent   = 0;
float disp_height_cm = 0.0f;
float disp_volume_l  = 0.0f;
bool  emergency_mode = false;  // true while emergency mode is active
bool  display_ok     = false;  // true after display initialised successfully

// ============================================================
// VERTICAL FILL BAR
// Draws a bordered rectangle on the right side of the display
// that fills from the bottom upward proportional to 'percent'.
// ============================================================
void drawVerticalBar(int percent) {
  const int BAR_X = 110;  // Left edge (pixels)
  const int BAR_W = 16;   // Width (pixels)
  const int BAR_Y = 2;    // Top edge (pixels)
  const int BAR_H = 60;   // Total height (pixels)

  display.drawFrame(BAR_X, BAR_Y, BAR_W, BAR_H);

  int fill_h = (BAR_H * percent) / 100;
  if (fill_h > 0) {
    display.drawBox(BAR_X + 1, BAR_Y + BAR_H - fill_h, BAR_W - 2, fill_h);
  }
}

// ============================================================
// DISPLAY UPDATE
// Renders the current state to the OLED buffer and flushes it.
// Shows a reduced emergency screen when emergency_mode is true,
// otherwise shows fill level, percentage and fill height.
// UI texts are in German; code comments are in English.
// ============================================================
void updateDisplay() {
  if (!display_ok) return;

  display.clearBuffer();
  drawVerticalBar(disp_percent);

  if (emergency_mode) {
    // ===== EMERGENCY MODE =====
    display.setFont(u8g2_font_ncenB12_tr);
    display.drawStr(4, 18, "NOT-");
    display.drawStr(4, 34, "BETRIEB");

    display.setFont(u8g2_font_ncenB10_tr);
    display.drawStr(4, 52, "Fuellstand:");
    display.setCursor(80, 52);
    display.print(disp_percent);
    display.print("%");

  } else {
    // ===== NORMAL MODE =====
    display.setFont(u8g2_font_ncenB10_tr);
    display.drawStr(4, 16, "Fuellstand");

    display.setFont(u8g2_font_ncenB14_tr);
    display.setCursor(4, 34);
    display.print(disp_percent);
    display.print("%");

    display.setFont(u8g2_font_ncenB10_tr);
    display.drawStr(4, 48, "Hoehe");

    display.setCursor(4, 62);
    display.print((int)disp_height_cm);
    display.print(" cm");
  }

  display.sendBuffer();
}

// ============================================================
// TFmini-S FRAME PARSER
// Drains Serial2, validates frames (dual-header + checksum) and
// returns the distance value [cm] of the last valid frame.
// Returns -1 if no valid frame was received since the last call.
// ============================================================
int read_tfmini() {
  static uint8_t buf[TFMINI_FRAME_SIZE * 4];  // Ring buffer for incoming bytes
  static int buf_len = 0;
  int last_valid = -1;

  // Append all available bytes to the buffer
  while (Serial2.available()) {
    if (buf_len >= (int)sizeof(buf)) buf_len = 0;  // Overflow guard: reset
    buf[buf_len++] = Serial2.read();
  }

  // Scan buffer for complete, valid frames
  int i = 0;
  while (i <= buf_len - TFMINI_FRAME_SIZE) {
    // Check dual-header bytes
    if (buf[i] != TFMINI_HEADER || buf[i+1] != TFMINI_HEADER) {
      i++; continue;
    }
    // Verify checksum (sum of bytes 0..7 == byte 8)
    uint8_t cs = 0;
    for (int j = i; j < i + 8; j++) cs += buf[j];
    if (cs != buf[i+8]) { i++; continue; }

    // Extract 16-bit distance (little-endian, bytes 2 and 3)
    last_valid = buf[i+2] + (buf[i+3] << 8);
    i += TFMINI_FRAME_SIZE;
  }

  // Discard consumed bytes, keep any partial frame at the front
  if (i > 0) {
    buf_len -= i;
    memmove(buf, buf + i, buf_len);
  }

  return last_valid;
}

// ============================================================
// TFminiSensor CLASS
// SensESP sensor that polls the TFmini-S every 100 ms via the
// ReactESP event loop.  On each tick it:
//   1. Reads the latest distance measurement.
//   2. Converts distance to fill height, ratio and volume.
//   3. Publishes values to Signal K.
//   4. Evaluates emergency mode (high fill + GPIO 19 pulled LOW).
//   5. Controls the alarm relay with hysteresis.
//   6. Refreshes the OLED display.
// ============================================================
class TFminiSensor : public Sensor<float> {
 public:
  TFminiSensor() : Sensor<float>("") {
    Serial2.begin(115200, SERIAL_8N1, TF_RX, TF_TX);
    Serial.println("TFmini-S UART started");
    event_loop()->onRepeat(100, [this]() { this->update(); });
  }

 private:
  bool alarm_on = false;  // Current relay state (hysteresis latch)

  void update() {
    int dist = read_tfmini();  // Distance from sensor to liquid surface [cm]

    if (dist > 0) {
      // Subtract sensor offset to get distance from tank top to liquid surface,
      // then invert to obtain fill height from the tank bottom.
      float empty  = constrain((float)dist - cfg_offset, 0.0f, cfg_height);
      float fill_h = cfg_height - empty;
      float ratio  = constrain(fill_h / cfg_height, 0.0f, 1.0f);

      // Update shared display state
      disp_height_cm = fill_h;
      disp_percent   = (int)roundf(ratio * 100.0f);
      disp_volume_l  = tank_capacity_liters() * ratio;

      // Publish to Signal K
      sk_level->set(ratio);
      sk_volume->set(tank_capacity_m3() * ratio);
    }

    // Emergency mode: fill level at or above alarm threshold AND
    // external emergency input pulled to GND (e.g. bilge float switch).
    bool emergency_input = (digitalRead(EMERGENCY_PIN) == LOW);
    emergency_mode = (disp_percent >= (int)cfg_alarm_high) && emergency_input;

    if (emergency_mode) {
      // Safe state: relay OFF regardless of alarm hysteresis
      digitalWrite(ALARM_PIN, LOW);
      Serial.printf("!! EMERGENCY !! Dist=%d cm | Fill=%d%% | Input=GND\n",
        dist, disp_percent);
    } else {
      // Normal operation: relay controlled by hysteresis thresholds
      if (!alarm_on && disp_percent >= (int)cfg_alarm_high) {
        alarm_on = true;   // Arm relay when fill reaches upper threshold
      } else if (alarm_on && disp_percent <= (int)cfg_alarm_low) {
        alarm_on = false;  // Release relay when fill drops to lower threshold
      }
      digitalWrite(ALARM_PIN, alarm_on ? HIGH : LOW);  // HW-482: active-high

      if (dist > 0) {
        Serial.printf("Dist=%d cm | Fill=%.1f cm (%d%%) | Vol=%.1f L | Alarm=%s\n",
          dist, disp_height_cm, disp_percent, disp_volume_l,
          alarm_on ? "ON" : "off");
      }
    }

    updateDisplay();
  }
};

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Blackwater Tank Monitor - TFmini-S");

  // --- GPIO initialisation ---
  pinMode(ALARM_PIN, OUTPUT);
  digitalWrite(ALARM_PIN, LOW);          // Relay off at boot (HW-482: active-high)
  pinMode(EMERGENCY_PIN, INPUT_PULLUP);  // Emergency input, active-LOW

  // --- Display: first init before SensESP ---
  // Software I2C on GPIO 32/33 avoids bus conflict with SensESP.
  display.begin();
  display.setContrast(255);
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 15, "Init...");
  display.sendBuffer();
  display_ok = true;
  Serial.println("Display OK");

  SetupLogging();

  // --- SensESP application ---
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
    ->set_hostname("blackwater-tank")
    ->get_app();

  // --- Display: re-init after SensESP ---
  // SensESP may reconfigure the I2C bus during get_app(); reinitialise
  // the software-I2C display to restore a known good state.
  if (display_ok) {
    display.begin();
    display.setContrast(255);
    display.clearBuffer();
    display.setFont(u8g2_font_ncenB08_tr);
    display.drawStr(0, 15, "Init...");
    display.sendBuffer();
  }

  // ============================================================
  // TANK CONFIGURATION – WEB UI
  // Each ConstantSensor exposes a field in the SensESP web UI at
  // http://blackwater-tank.local.  The LambdaConsumer writes the
  // value back to the corresponding global config variable so that
  // changes take effect immediately without a reboot.
  // ============================================================
  auto* sens_length = new ConstantSensor<float>(cfg_length, 0, "/Tank/Length_cm");
  ConfigItem(sens_length)
    ->set_title("Tank Length (cm)")
    ->set_description("Internal tank length in cm")
    ->set_sort_order(100);
  sens_length->connect_to(new LambdaConsumer<float>([](float v){ cfg_length = v; }));

  auto* sens_width = new ConstantSensor<float>(cfg_width, 0, "/Tank/Width_cm");
  ConfigItem(sens_width)
    ->set_title("Tank Width (cm)")
    ->set_description("Internal tank width in cm")
    ->set_sort_order(200);
  sens_width->connect_to(new LambdaConsumer<float>([](float v){ cfg_width = v; }));

  auto* sens_height = new ConstantSensor<float>(cfg_height, 0, "/Tank/Height_cm");
  ConfigItem(sens_height)
    ->set_title("Tank Height (cm)")
    ->set_description("Internal tank height in cm")
    ->set_sort_order(300);
  sens_height->connect_to(new LambdaConsumer<float>([](float v){ cfg_height = v; }));

  auto* sens_offset = new ConstantSensor<float>(cfg_offset, 0, "/Tank/Sensor_Offset_cm");
  ConfigItem(sens_offset)
    ->set_title("Sensor Offset (cm)")
    ->set_description("Distance from sensor face to tank top edge in cm")
    ->set_sort_order(400);
  sens_offset->connect_to(new LambdaConsumer<float>([](float v){ cfg_offset = v; }));

  auto* sens_alarm_high = new ConstantSensor<float>(cfg_alarm_high, 0, "/Tank/Alarm_ON_pct");
  ConfigItem(sens_alarm_high)
    ->set_title("Alarm ON threshold % (default: 85)")
    ->set_description("Fill level % at which the relay is activated")
    ->set_sort_order(500);
  sens_alarm_high->connect_to(new LambdaConsumer<float>([](float v){ cfg_alarm_high = v; }));

  auto* sens_alarm_low = new ConstantSensor<float>(cfg_alarm_low, 0, "/Tank/Alarm_OFF_pct");
  ConfigItem(sens_alarm_low)
    ->set_title("Alarm OFF threshold % (default: 75)")
    ->set_description("Fill level % below which the relay is deactivated (hysteresis)")
    ->set_sort_order(600);
  sens_alarm_low->connect_to(new LambdaConsumer<float>([](float v){ cfg_alarm_low = v; }));

  // ============================================================
  // SIGNAL K OUTPUTS
  // ============================================================
  sk_level = new SKOutputFloat(
    "tanks.blackWater.0.currentLevel", "/Tank/Level",
    new SKMetadata("ratio", "Blackwater tank fill level")
  );
  sk_capacity = new SKOutputFloat(
    "tanks.blackWater.0.capacity", "/Tank/Capacity",
    new SKMetadata("m3", "Blackwater tank total capacity")
  );
  sk_volume = new SKOutputFloat(
    "tanks.blackWater.0.currentVolume", "/Tank/Volume",
    new SKMetadata("m3", "Blackwater tank current volume")
  );

  // Publish capacity once after 5 s (Signal K server may not be ready at boot)
  event_loop()->onDelay(5000, []() {
    sk_capacity->set(tank_capacity_m3());
  });
  // Re-publish capacity every 60 s to keep the Signal K value fresh
  event_loop()->onRepeat(60000, []() {
    sk_capacity->set(tank_capacity_m3());
  });

  // Start sensor – registers the 100 ms onRepeat callback in the event loop
  new TFminiSensor();

  Serial.println("Setup complete!");
}

// ============================================================
// LOOP
// Delegates all work to the ReactESP event loop.
// Application logic runs exclusively via onRepeat / onDelay
// callbacks registered during setup().
// ============================================================
void loop() {
  event_loop()->tick();
}
