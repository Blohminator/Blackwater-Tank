# Wiring Guide

## Component Connections

### ESP32 Wemos D1 Mini

```
                    ┌─────────────────┐
                    │   ESP32 D1 Mini │
                    │                 │
         5V ────────┤ 5V          GND ├──────── GND
                    │                 │
    LiDAR RX ───────┤ GPIO 16 (RX2)   │
    LiDAR TX ───────┤ GPIO 17 (TX2)   │
                    │                 │
    OLED SDA ───────┤ GPIO 32         │  (Software I2C)
    OLED SCL ───────┤ GPIO 33         │  (Software I2C)
                    │                 │
  Alarm Out ────────┤ GPIO 23         │  (Relay, active-high)
  Emergency In ─────┤ GPIO 19         │  (with internal pull-up)
                    │                 │
                    └─────────────────┘
```

## TFmini-S LiDAR Sensor

The TFmini-S has 4 wires:

| Wire Color | Function | Connect To |
|------------|----------|------------|
| Red | VCC (5V) | ESP32 5V or external 5V supply |
| Black | GND | ESP32 GND |
| Green | TX | ESP32 GPIO 16 (RX2) |
| White | RX | ESP32 GPIO 17 (TX2) |

**Important Notes:**
- TFmini-S requires 5V power (not 3.3V)
- TX/RX are 3.3V compatible, safe for ESP32
- Green wire (TFmini TX) → ESP32 RX
- White wire (TFmini RX) → ESP32 TX

### TFmini-S Mounting

```
        ┌─────────────────┐
        │   Tank Opening  │
        └─────────────────┘
                ↓
        ┌───────────────┐
        │  TFmini-S     │  ← Mount sensor pointing down
        │  (pointing ↓) │
        └───────────────┘
                ↓
        ┌─────────────────┐
        │                 │
        │                 │  ← Sensor offset distance
        │  ═════════════  │  ← Tank top edge
        │  ░░░░░░░░░░░░░  │
        │  ░░░░░░░░░░░░░  │  ← Liquid
        │  ░░░░░░░░░░░░░  │
        └─────────────────┘
```

## OLED Display (SSD1306 128x64, Software I2C)

The SSD1306 OLED has 4 connections:

| OLED Pin | Function | Connect To |
|---------|----------|------------|
| VCC | Power (3.3V) | ESP32 3.3V |
| GND | Ground | ESP32 GND |
| SDA | I2C Data | ESP32 GPIO 32 |
| SCL | I2C Clock | ESP32 GPIO 33 |

**Notes:**
- GPIO 32/33 are used for Software I2C to avoid conflict with SensESP hardware I2C on GPIO 21/22
- Most SSD1306 modules run on 3.3V (check your module's datasheet)
- I2C address is typically 0x3C

## Alarm Output (Relay, GPIO 23)

Connect to GPIO 23 (HW-482 relay module, active-high):

### Option 1: LED Indicator
```
GPIO 23 ──┬── 220Ω Resistor ──┬── LED (+) ──┬── GND
          │                    │             │
          └────────────────────┴─────────────┘
```

### Option 2: Active Buzzer
```
GPIO 23 ──┬── Buzzer (+)
          │
         GND ── Buzzer (-)
```

### Option 3: Relay Module
```
GPIO 23 ──── Relay IN
5V ────────── Relay VCC
GND ───────── Relay GND
```

## Emergency Input (GPIO 19)

Connect to GPIO 19 (has internal pull-up resistor):

### Option 1: Simple Switch
```
GPIO 19 ──┬── Switch ──┬── GND
```
When switch is CLOSED (connected to GND) AND fill level ≥ alarm threshold, emergency mode activates.
When switch is OPEN, system operates normally.

### Option 2: Float Switch
```
GPIO 19 ──┬── Float Switch ──┬── GND
          │                  │
          └──────────────────┘
```
Use a normally-open float switch that closes when water reaches critical level.

### Option 3: External Alarm System
```
GPIO 19 ──┬── Relay Contact (NO) ──┬── GND
          │                        │
          └────────────────────────┘
```
Connect to normally-open relay contact from external alarm system.

## Relay Output

The relay is controlled via GPIO 23 (same as alarm output — there is no separate relay pin).
The HW-482 relay module is active-high:
- GPIO 23 HIGH → Relay ON (alarm active)
- GPIO 23 LOW  → Relay OFF (safe state / emergency mode)

## Power Supply

### Option 1: USB Power
- Connect ESP32 via USB cable
- Provides 5V for ESP32, LCD, and LiDAR
- Simple for testing

### Option 2: External 5V Supply
```
5V Supply (+) ──┬── ESP32 5V
                ├── LCD VCC
                └── LiDAR VCC (Red)

5V Supply (-) ──┬── ESP32 GND
                ├── LCD GND
                └── LiDAR GND (Black)
```

### Option 3: 12V/24V Boat Power
Use a DC-DC buck converter:
```
12V/24V (+) ──┬── Buck Converter IN+
              │
12V/24V (-) ──┴── Buck Converter IN-

Buck Converter OUT+ (5V) ──── ESP32 5V, LCD VCC, LiDAR VCC
Buck Converter OUT- (GND) ─── Common GND
```

**Recommended:** Use a quality buck converter rated for at least 1A output.

## Complete Wiring Diagram

```
                    ┌──────────────────┐
    ┌───────────────┤  5V Power Supply │
    │               └──────────────────┘
    │                        │
    │                       GND
    │                        │
    ├────────┬───────────────┼────────────┬──────────┐
    │        │               │            │          │
    5V      GND             GND          GND        GND
    │        │               │            │          │
┌───┴────────┴───┐   ┌───────┴────┐  ┌───┴──────┐   │
│   ESP32 D1     │   │ TFmini-S   │  │  OLED    │   │
│     Mini       │   │   LiDAR    │  │ SSD1306  │   │
│                │   │            │  │ 128x64   │   │
│ GPIO16 ────────┼───┤ TX (Green) │  │          │   │
│ GPIO17 ────────┼───┤ RX (White) │  │          │   │
│ GPIO32 ────────┼───┼────────────┼──┤ SDA      │   │
│ GPIO33 ────────┼───┼────────────┼──┤ SCL      │   │
│ GPIO23 ────────┼───┼────────────┼──┼──────────┼───┤
│                │   │            │  │          │   │
└────────────────┘   └────────────┘  └──────────┘   │
                                                     │
                                                 ┌───┴────┐
                                                 │ Relay  │
                                                 │ HW-482 │
                                                 └────────┘
```

## Testing Connections

### 1. Test I2C OLED
Run an I2C scanner sketch to verify OLED address:
```cpp
#include <Wire.h>

void setup() {
  Wire.begin(32, 33);  // Software I2C on GPIO 32/33
  Serial.begin(115200);
  Serial.println("I2C Scanner");
}

void loop() {
  for(byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if(Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(i, HEX);
    }
  }
  delay(5000);
}
```

### 2. Test TFmini-S
Monitor Serial2 for data frames:
```cpp
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
}

void loop() {
  if(Serial2.available()) {
    Serial.print("0x");
    Serial.print(Serial2.read(), HEX);
    Serial.print(" ");
  }
}
```
You should see repeating pattern: `0x59 0x59 ...`

### 3. Test Alarm Output
```cpp
void setup() {
  pinMode(23, OUTPUT);
}

void loop() {
  digitalWrite(23, HIGH);
  delay(1000);
  digitalWrite(23, LOW);
  delay(1000);
}
```

## Troubleshooting

### OLED not working
- Check wiring: SDA → GPIO 32, SCL → GPIO 33
- Verify 3.3V power supply
- Check I2C address (SSD1306 is typically 0x3C)
- Run I2C scanner to detect address

### LiDAR not reading
- Swap RX/TX connections
- Check 5V power to LiDAR
- Verify baud rate (115200)
- Ensure clear line of sight

### Alarm not triggering
- Test GPIO 23 with LED
- Check alarm threshold setting
- Verify fill level calculation

## Safety Notes

- Use proper wire gauge for current requirements
- Secure all connections to prevent shorts
- Protect electronics from moisture
- Use appropriate enclosures for marine environments
- Add fuses for overcurrent protection
- Follow local electrical codes and regulations
