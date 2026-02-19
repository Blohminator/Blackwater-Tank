# Relay Test and Diagnostics

## Problem
The relay on GPIO 18 doesn't switch, although the alarm on GPIO 23 works.

## Possible Causes

### 1. Hardware Problem
- **GPIO 18 is a special pin** on the ESP32
- During boot, GPIO 18 must not be HIGH (otherwise ESP32 enters Download Mode)
- Some relay modules have pull-up resistors that cause problems

### 2. Pin Conflict
- GPIO 18 is sometimes used for SPI (SCK)
- Check if other devices are using GPIO 18

### 3. Relay Module Requires More Current
- ESP32 GPIO can only supply ~40mA
- Some relay modules need more

## Diagnostics with New Code

After upload, you'll see in the Serial Monitor:

```
Testing Relay...
Relay HIGH, Pin18=1
Relay LOW, Pin18=0
Relay set to HIGH for normal operation, Pin18=1
```

If you see `Pin18=0` even though HIGH was set, there's a hardware problem.

## During Operation

```
NRM: 85% A:ON R:OFF (Pin18=0 Pin23=1)
```

This shows:
- **NRM** = Normal Mode (or EMG = Emergency)
- **85%** = Fill level
- **A:ON** = Alarm should be ON
- **R:OFF** = Relay should be OFF
- **Pin18=0** = Actual state of GPIO 18 (1=HIGH, 0=LOW)
- **Pin23=1** = Actual state of GPIO 23

## Solutions

### Solution 1: Use Different GPIO Pin

If GPIO 18 causes problems, use a different pin:

**Recommended alternative pins:**
- GPIO 25 (not a special boot pin)
- GPIO 26 (not a special boot pin)
- GPIO 27 (not a special boot pin)
- GPIO 32 (not a special boot pin)
- GPIO 33 (not a special boot pin)

**DO NOT use:**
- GPIO 0, 2, 5, 12, 15 (Boot pins)
- GPIO 6-11 (Flash pins)
- GPIO 34-39 (Input only)

### Solution 2: Use Transistor/MOSFET

If the relay module draws too much current:

```
GPIO 18 ──┬── 10kΩ ──┬── MOSFET Gate (e.g. 2N7000)
          │          │
         GND ────────┴── MOSFET Source

MOSFET Drain ──── Relay Coil (+)
Relay Coil (-) ── GND

Relay VCC ──── 5V (external power supply)
```

### Solution 3: Use Optocoupler

For galvanic isolation:

```
GPIO 18 ──┬── 220Ω ──┬── Optocoupler LED (+)
          │          │
         GND ────────┴── Optocoupler LED (-)

Optocoupler Transistor ──── Relay Module IN
```

## Hardware Test Without Code

1. **Turn off ESP32**
2. **Multimeter on GPIO 18 and GND**
3. **Turn on ESP32**
4. **Measure voltage:**
   - Should be ~3.3V when Relay ON
   - Should be ~0V when Relay OFF

## Code Change for Different Pin

If you want to use GPIO 25 for example:

```cpp
// In main.cpp line 26, change:
#define RELAY_PIN 25         // Instead of 18
```

Then recompile and upload.

## Next Steps

1. **Upload the new firmware** (with relay test at startup)
2. **Open Serial Monitor** (115200 baud)
3. **Send me the output** from:
   - "Testing Relay..." section
   - "NRM: XX% A:XX R:XX (Pin18=X Pin23=X)" lines
4. **Measure with multimeter** the voltage at GPIO 18

Then we can identify the problem exactly!
