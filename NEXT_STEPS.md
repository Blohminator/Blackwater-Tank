# Next Steps - Relay Debugging

## Current Status

✅ **Working:**
- Alarm on GPIO 23 switches correctly at 85% (with hysteresis to 75%)
- Emergency Mode logic works
- OLED Display shows correct values
- TFmini-S LiDAR reads distance

❌ **Problem:**
- Relay on GPIO 18 does NOT switch

## What You Need to Do Now

### Step 1: Upload Firmware
```
pio run -t upload
```

### Step 2: Open Serial Monitor
```
pio device monitor
```

### Step 3: Check Output

At startup you'll see:
```
Testing Relay...
Relay HIGH, Pin18=?
Relay LOW, Pin18=?
Relay set to HIGH for normal operation, Pin18=?
```

**IMPORTANT:** Look at the Pin18 values!

### Step 4: Check During Operation

When fill level goes above 80%, you'll see:
```
NRM: 85% A:ON R:OFF (Pin18=? Pin23=?)
```

## Diagnosis

### Case A: Pin18 always shows 0
**Problem:** GPIO 18 hardware conflict
**Solution:** Use different pin (GPIO 25, 26, 27, 32, or 33)

### Case B: Pin18 switches between 0 and 1
**Problem:** Relay module doesn't respond
**Possible causes:**
- Relay needs more current (>40mA)
- Relay module defective
- Wrong wiring

**Solution:** 
- Transistor/MOSFET between ESP32 and relay
- Multimeter test: GPIO 18 should show 3.3V (HIGH) or 0V (LOW)

### Case C: Pin18 shows 1 but relay doesn't switch
**Problem:** Relay module hardware
**Solution:**
- Check wiring
- Check relay VCC (needs 5V?)
- Test relay with direct 3.3V connection

## Pin Change (if needed)

If GPIO 18 doesn't work, change in `src/main.cpp` line 26:

```cpp
#define RELAY_PIN 25         // Instead of 18
```

**Recommended pins:** 25, 26, 27, 32, 33

Then recompile and upload.

## Hardware Test with Multimeter

1. Turn on ESP32
2. Multimeter between GPIO 18 and GND
3. Fill level under 80%: should show ~3.3V (Relay ON)
4. Fill level over 80%: should show ~0V (Relay OFF)
5. Emergency Mode: should show ~3.3V (Relay ON)

## Send Me This Information

1. **Serial Monitor output** at startup (Relay Test)
2. **Serial Monitor output** when fill level exceeds 80%
3. **Multimeter measurement** at GPIO 18
4. **Photo** of your wiring (optional)

Then I can identify the problem exactly!
