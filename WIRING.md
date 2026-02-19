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
     LCD SDA ───────┤ GPIO 21 (SDA)   │
     LCD SCL ───────┤ GPIO 22 (SCL)   │
                    │                 │
  Alarm Out ────────┤ GPIO 23         │
  Alarm In ─────────┤ GPIO 19         │ (with internal pull-up)
  Relay Out ────────┤ GPIO 18         │
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

## LCD Display (16x2 I2C)

The LCD with PCF8574 I2C backpack has 4 connections:

| LCD Pin | Function | Connect To |
|---------|----------|------------|
| VCC | Power (5V) | ESP32 5V |
| GND | Ground | ESP32 GND |
| SDA | I2C Data | ESP32 GPIO 21 |
| SCL | I2C Clock | ESP32 GPIO 22 |

**I2C Address:**
- Most common: 0x27
- Alternative: 0x3F
- Use I2C scanner if unsure

## Alarm Output

Connect to GPIO 23:

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

## Alarm Input (Emergency Override)

Connect to GPIO 19 (has internal pull-up resistor):

### Option 1: Simple Switch
```
GPIO 19 ──┬── Switch ──┬── GND
          │            │
          └────────────┘
```
When switch is CLOSED (connected to GND), emergency mode can activate.
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

## Relay Output (Pump/Valve Control)

Connect to GPIO 18:

### Option 1: Relay Module (Recommended)
```
GPIO 18 ──── Relay Module IN
5V ────────── Relay Module VCC
GND ───────── Relay Module GND

Relay Contacts:
  COM ──── Pump/Valve Power
  NO ───── Pump/Valve Device
  NC ───── (not used)
```

**Operation:**
- Normal mode: GPIO 18 = HIGH → Relay ON → Pump/Valve active
- Emergency mode: GPIO 18 = LOW → Relay OFF → Pump/Valve inactive

### Option 2: Solid State Relay (SSR)
```
GPIO 18 ──┬── 1kΩ Resistor ──┬── SSR (+)
          │                  │
         GND ──────────────── SSR (-)

SSR Load Contacts:
  Connect to pump or valve power circuit
```

### Option 3: MOSFET Driver (for DC loads)
```
GPIO 18 ──┬── 10kΩ Resistor ──┬── MOSFET Gate
          │                   │
         GND ──────────────────┴── MOSFET Source

MOSFET Drain ──── Load (+)
Load (-) ──────── Power Supply (-)
```

**Important Notes:**
- Use appropriate relay/driver for your load voltage and current
- Add flyback diode for inductive loads (pumps, solenoids)
- Ensure proper electrical isolation for high-voltage loads
- Follow local electrical codes

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
│   ESP32 D1     │   │ TFmini-S   │  │   LCD    │   │
│     Mini       │   │   LiDAR    │  │  16x2    │   │
│                │   │            │  │   I2C    │   │
│ GPIO16 ────────┼───┤ TX (Green) │  │          │   │
│ GPIO17 ────────┼───┤ RX (White) │  │          │   │
│ GPIO21 ────────┼───┼────────────┼──┤ SDA      │   │
│ GPIO22 ────────┼───┼────────────┼──┤ SCL      │   │
│ GPIO23 ────────┼───┼────────────┼──┼──────────┼───┤
│                │   │            │  │          │   │
└────────────────┘   └────────────┘  └──────────┘   │
                                                     │
                                                 ┌───┴────┐
                                                 │ Alarm  │
                                                 │ Device │
                                                 └────────┘
```

## Testing Connections

### 1. Test I2C LCD
Run an I2C scanner sketch to verify LCD address:
```cpp
#include <Wire.h>

void setup() {
  Wire.begin(21, 22);
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

### LCD not working
- Check I2C address (0x27 or 0x3F)
- Verify SDA/SCL connections
- Check contrast potentiometer on I2C backpack
- Verify 5V power supply

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
