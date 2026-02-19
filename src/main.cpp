/*============================================
Blackwater Tank Level Monitor
ESP32 + TFmini-S LiDAR + SensESP v3
Signal K: tanks.blackWater.0.*
LCD display + configurable alarm
Web-UI: /config
============================================*/

#include <Arduino.h>
#include <memory>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>

#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "sensesp/signalk/signalk_output.h"

using namespace sensesp;

// ===== Hardware Configuration =====
#define RXD2 16              // LiDAR RX (ESP32 receives from TFmini TX)
#define TXD2 17              // LiDAR TX (ESP32 sends to TFmini RX)
#define ALARM_PIN 23         // Alarm output pin
#define ALARM_INPUT_PIN 19   // Emergency alarm input (active LOW when closed)
#define RELAY_PIN 18         // Relay control output
#define LCD_I2C_ADDR 0x27    // Freenove 1602 I2C address
#define I2C_SDA 21           // I2C data pin
#define I2C_SCL 22           // I2C clock pin

// ===== TFmini-S Protocol =====
const int TFMINI_HEADER = 0x59;
const int TFMINI_FRAME_SIZE = 9;

// ===== LCD Instance =====
LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDR);

// ===== Tank Configuration Structure =====
struct TankConfig {
  float length_cm = 50.0f;
  float width_cm = 72.0f;
  float height_cm = 87.0f;
  float sensor_offset_cm = 5.0f;
  int alarm_threshold_percent = 80;
  
  float capacity_liters() const {
    return (length_cm * width_cm * height_cm) / 1000.0f;
  }
  
  float capacity_m3() const {
    return capacity_liters() / 1000.0f;
  }
};

std::shared_ptr<TankConfig> tank_config;

// ===== Signal K Outputs =====
SKOutputFloat* sk_current_level;
SKOutputFloat* sk_capacity;
SKOutputFloat* sk_current_volume;

// ===== Runtime State =====
int fill_height_cm = 0;
int fill_percent = 0;
int remaining_cm = 0;
bool emergency_mode = false;  // Emergency operation mode flag

// ===== Function Prototypes =====
int read_tfmini_distance();
void compute_fill_level(int distance_cm);
void update_lcd_display();
void check_alarm_and_relay();
void update_capacity();

// ===== Helper: Flicker-free LCD printing =====
void lcd_print_line(uint8_t row, const char* text) {
  lcd.setCursor(0, row);
  lcd.print("                "); // Clear 16 chars
  lcd.setCursor(0, row);
  lcd.print(text);
}

/* ============================================================
   SETUP
   ============================================================ */
void setup() {
  // Initialize logging
  SetupLogging();
  
  Serial.begin(115200);
  delay(50);
  Serial.println("\n=== TFmini-S LiDAR Tank Monitor ===");
  Serial.println("SensESP v3 - Blackwater Tank");
  
  // Initialize I2C and LCD
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd_print_line(0, "Initializing...");
  lcd_print_line(1, "Please wait");
  
  // Initialize LiDAR UART
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("LiDAR UART initialized on Serial2");
  
  // Initialize alarm pin
  pinMode(ALARM_PIN, OUTPUT);
  digitalWrite(ALARM_PIN, LOW);
  
  // Initialize alarm input pin (with internal pull-up)
  pinMode(ALARM_INPUT_PIN, INPUT_PULLUP);
  
  // Initialize relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Set to ON for normal operation
  
  Serial.println("GPIO pins initialized");
  
  // Build SensESP application
  SensESPAppBuilder builder;
  sensesp_app = builder.set_hostname("blackwater-tank")->get_app();
  
  Serial.println("SensESP app created");
  
  // Create tank configuration object
  tank_config = std::make_shared<TankConfig>();
  
  // Note: For now, tank parameters are set in code
  // Future enhancement: Add web UI configuration using ConfigItem
  Serial.println("Tank configuration initialized");
  
  // Create Signal K outputs
  sk_current_level = new SKOutputFloat("tanks.blackWater.0.currentLevel");
  sk_capacity = new SKOutputFloat("tanks.blackWater.0.capacity");
  sk_current_volume = new SKOutputFloat("tanks.blackWater.0.currentVolume");
  
  // Initialize Signal K values
  sk_current_level->set(0.0f);
  sk_current_volume->set(0.0f);
  update_capacity();
  
  Serial.println("Signal K outputs initialized");
  
  // Display ready message
  lcd_print_line(0, "SensESP Ready");
  lcd_print_line(1, "TFmini-S LiDAR");
  delay(1000);
  
  Serial.println("Setup complete!");
}

/* ============================================================
   MAIN LOOP
   ============================================================ */
void loop() {
  // Read distance from LiDAR
  int distance = read_tfmini_distance();
  
  if (distance > 0) {
    // Compute fill level from distance
    compute_fill_level(distance);
    
    // Calculate level ratio (0.0 to 1.0)
    float level_ratio = (float)fill_height_cm / (float)tank_config->height_cm;
    level_ratio = constrain(level_ratio, 0.0f, 1.0f);
    
    // Update Signal K outputs
    sk_current_level->set(level_ratio);
    sk_current_volume->set(tank_config->capacity_m3() * level_ratio);
  }
  
  // Always update display and check alarm (even if no new LiDAR reading)
  update_lcd_display();
  check_alarm_and_relay();
  
  // Update capacity (in case config changed)
  sk_capacity->set(tank_config->capacity_m3());
  
  // Run SensESP event loop
  event_loop()->tick();
  
  delay(200);
}

/* ============================================================
   TFmini-S LiDAR Reading
   Frame: 9 bytes [0x59 0x59 DistL DistH StrengthL StrengthH Reserved Quality Checksum]
   ============================================================ */
int read_tfmini_distance() {
  static uint8_t buffer[TFMINI_FRAME_SIZE];
  static int buffer_index = 0;
  int distance = 0;
  
  while (Serial2.available()) {
    uint8_t byte = Serial2.read();
    buffer[buffer_index++] = byte;
    
    if (buffer_index >= TFMINI_FRAME_SIZE) {
      // Check for valid frame header
      if (buffer[0] == TFMINI_HEADER && buffer[1] == TFMINI_HEADER) {
        // Verify checksum
        uint8_t checksum = 0;
        for (int i = 0; i < 8; i++) {
          checksum += buffer[i];
        }
        
        if (checksum == buffer[8]) {
          // Extract distance (cm)
          distance = buffer[2] + (buffer[3] << 8);
          buffer_index = 0;
          return distance;
        }
      }
      
      // Shift buffer on invalid frame
      for (int i = 0; i < TFMINI_FRAME_SIZE - 1; i++) {
        buffer[i] = buffer[i + 1];
      }
      buffer_index = TFMINI_FRAME_SIZE - 1;
    }
  }
  
  return distance;
}

/* ============================================================
   Compute Fill Level from Distance
   ============================================================ */
void compute_fill_level(int distance_cm) {
  // Calculate remaining height (distance minus sensor offset)
  remaining_cm = distance_cm - (int)roundf(tank_config->sensor_offset_cm);
  
  // Clamp to valid range
  int tank_height = (int)roundf(tank_config->height_cm);
  remaining_cm = constrain(remaining_cm, 0, tank_height);
  
  // Calculate fill height
  fill_height_cm = tank_height - remaining_cm;
  
  // Calculate fill percentage
  fill_percent = (int)roundf(100.0f * (float)fill_height_cm / (float)tank_height);
  fill_percent = constrain(fill_percent, 0, 100);
}

/* ============================================================
   Update LCD Display
   ============================================================ */
void update_lcd_display() {
  if (emergency_mode) {
    // Emergency mode: Show fill level and emergency message
    char line1[17];
    snprintf(line1, sizeof(line1), "Fill:%2dcm %3d%%", fill_height_cm, fill_percent);
    lcd_print_line(0, line1);
    lcd_print_line(1, "EMERGENCY MODE");
  } else {
    // Normal mode: Show fill height, percentage, and volume
    char line1[17];
    snprintf(line1, sizeof(line1), "Fill:%2dcm %3d%%", fill_height_cm, fill_percent);
    lcd_print_line(0, line1);
    
    // Line 2: Current volume in liters
    float volume_liters = tank_config->capacity_liters() * 
                          ((float)fill_height_cm / (float)tank_config->height_cm);
    int volume_int = (int)roundf(volume_liters);
    
    char line2[17];
    snprintf(line2, sizeof(line2), "Vol:%6dL", volume_int);
    lcd_print_line(1, line2);
  }
}

/* ============================================================
   Check Alarm and Relay Control with Emergency Mode
   ============================================================ */
void check_alarm_and_relay() {
  bool threshold_exceeded = (fill_percent >= tank_config->alarm_threshold_percent);
  bool alarm_input_closed = (digitalRead(ALARM_INPUT_PIN) == LOW);
  
  // State tracking for debug output
  static bool last_emergency_mode = false;
  static bool last_threshold_exceeded = false;
  static bool last_relay_state = true;  // Track relay state
  
  // Emergency mode state machine
  if (emergency_mode) {
    // Currently IN emergency mode - only exit when pin 19 opens
    if (!alarm_input_closed) {
      emergency_mode = false;
      Serial.println("Emergency OFF");
      // Display update happens in main loop
    }
  } else {
    // NOT in emergency mode - enter if both conditions met
    if (threshold_exceeded && alarm_input_closed) {
      emergency_mode = true;
      Serial.println("Emergency ON");
      // Display update happens in main loop
    }
  }
  
  // Determine relay state
  // Emergency mode: Relay ON (safe/normal state)
  // Normal mode: Relay OFF when threshold exceeded (alarm state)
  bool relay_should_be_on = emergency_mode || !threshold_exceeded;
  
  // Set outputs
  digitalWrite(RELAY_PIN, relay_should_be_on ? HIGH : LOW);
  digitalWrite(ALARM_PIN, (emergency_mode || threshold_exceeded) ? HIGH : LOW);
  
  // Read back the actual pin state to verify
  int relay_pin_state = digitalRead(RELAY_PIN);
  int alarm_pin_state = digitalRead(ALARM_PIN);
  
  // Debug output on state changes only
  if (emergency_mode != last_emergency_mode || 
      threshold_exceeded != last_threshold_exceeded ||
      relay_should_be_on != last_relay_state) {
    
    // Single Serial.println with all info
    char debug_msg[80];
    snprintf(debug_msg, sizeof(debug_msg), 
             "%s: %d%% A:%s R:%s (Pin18=%d Pin23=%d)",
             emergency_mode ? "EMG" : "NRM",
             fill_percent,
             (emergency_mode || threshold_exceeded) ? "ON" : "OFF",
             relay_should_be_on ? "ON" : "OFF",
             relay_pin_state,
             alarm_pin_state);
    Serial.println(debug_msg);
    
    last_emergency_mode = emergency_mode;
    last_threshold_exceeded = threshold_exceeded;
    last_relay_state = relay_should_be_on;
  }
}

/* ============================================================
   Update Capacity Display and Signal K
   ============================================================ */
void update_capacity() {
  sk_capacity->set(tank_config->capacity_m3());
  
  char capacity_text[17];
  snprintf(capacity_text, sizeof(capacity_text), "Cap:%7.4f m3", tank_config->capacity_m3());
  lcd_print_line(1, capacity_text);
}
