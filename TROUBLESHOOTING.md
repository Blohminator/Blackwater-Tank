# Troubleshooting Guide

## Common Issues and Solutions

### 1. ESP32 Won't Upload

**Symptoms:**
- Upload fails with timeout errors
- "Failed to connect to ESP32" message

**Solutions:**
- Hold BOOT button while uploading
- Check USB cable (use data cable, not charge-only)
- Try different USB port
- Install/update CH340 or CP2102 drivers
- Reduce upload speed in platformio.ini:
  ```ini
  upload_speed = 115200
  ```

### 2. LCD Shows Nothing

**Symptoms:**
- LCD backlight on but no text
- Completely blank display

**Solutions:**
- Adjust contrast potentiometer on I2C backpack (small blue screw)
- Verify I2C address (try 0x3F if 0x27 doesn't work):
  ```cpp
  LiquidCrystal_PCF8574 lcd(0x3F);  // Change address
  ```
- Check wiring: SDA→21, SCL→22
- Run I2C scanner to detect address
- Verify 5V power supply

### 3. LCD Shows Garbage Characters

**Symptoms:**
- Random characters or blocks
- Flickering display

**Solutions:**
- Check I2C pull-up resistors (usually built into backpack)
- Reduce I2C speed if using long wires
- Check for loose connections
- Verify common ground between ESP32 and LCD

### 4. LiDAR Returns No Data

**Symptoms:**
- Distance always shows 0 or --
- Serial monitor shows no LiDAR frames

**Solutions:**
- **Swap RX/TX wires** (most common issue)
  - TFmini TX (green) → ESP32 GPIO 16
  - TFmini RX (white) → ESP32 GPIO 17
- Verify 5V power to LiDAR (red wire)
- Check baud rate is 115200
- Test with simple serial read:
  ```cpp
  void loop() {
    while(Serial2.available()) {
      Serial.printf("0x%02X ", Serial2.read());
    }
  }
  ```
- Ensure sensor has clear line of sight
- Check if sensor is within range (30-1200cm)

### 5. LiDAR Readings Unstable

**Symptoms:**
- Distance jumps erratically
- Inconsistent measurements

**Solutions:**
- Ensure stable mounting (no vibration)
- Avoid measuring foam or turbulent surfaces
- Clean sensor lens
- Check for reflective surfaces causing interference
- Increase averaging in code
- Verify sensor is perpendicular to surface

### 6. WiFi Won't Connect

**Symptoms:**
- Can't find SensESP access point
- Won't connect to home WiFi

**Solutions:**
- Look for AP named "SensESP-blackwater-tank"
- Reset WiFi settings by holding BOOT button for 5+ seconds
- Check WiFi credentials (case-sensitive)
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Move closer to router
- Check router allows new devices
- Try hardcoding WiFi in code:
  ```cpp
  builder.set_wifi_client("SSID", "PASSWORD")
  ```

### 7. Signal K Not Connecting

**Symptoms:**
- No data appearing in Signal K
- Connection timeout errors

**Solutions:**
- Verify Signal K server is running
- Check server IP address and port (default 3000)
- Ensure ESP32 and server on same network
- Check firewall settings
- Test server accessibility: `http://SERVER_IP:3000`
- Enable Signal K security token if required
- Check Signal K server logs for connection attempts

### 8. Web UI Not Accessible

**Symptoms:**
- Can't access http://blackwater-tank.local/config
- Page not found errors

**Solutions:**
- Try IP address instead of hostname
- Check serial monitor for IP address
- Ensure mDNS is supported on your network
- Try different browser
- Clear browser cache
- Verify ESP32 is connected to WiFi (check LCD/serial)

### 9. Alarm Not Working

**Symptoms:**
- Alarm output stays LOW
- Alarm doesn't trigger at threshold

**Solutions:**
- Test GPIO 23 with LED:
  ```cpp
  digitalWrite(ALARM_PIN, HIGH);
  delay(1000);
  digitalWrite(ALARM_PIN, LOW);
  ```
- Check alarm threshold setting (default 80%)
- Verify fill level is above threshold
- Check alarm device wiring
- For relay: ensure proper power supply
- For LED: add current-limiting resistor (220Ω)

### 10. Incorrect Volume Calculations

**Symptoms:**
- Volume doesn't match expected
- Capacity seems wrong

**Solutions:**
- Verify tank dimensions in web config
- Check sensor offset setting
- Ensure measurements are in centimeters
- Verify tank is rectangular (code assumes rectangular tank)
- For non-rectangular tanks, modify volume calculation
- Check for air pockets or obstructions

### 11. Compilation Errors

**Error: `'SetupLogging' was not declared`**
```
Solution: Update SensESP library to v3.0+
pio lib update
```

**Error: `'SKOutputFloat' does not name a type`**
```
Solution: Check includes:
#include "sensesp/signalk/signalk_output.h"
```

**Error: `'event_loop' was not declared`**
```
Solution: Use correct v3 syntax:
event_loop()->tick();
```

**Error: Library dependency issues**
```
Solution: Clean and rebuild:
pio run -t clean
pio run
```

### 12. Memory Issues

**Symptoms:**
- Random crashes or reboots
- "Guru Meditation Error"

**Solutions:**
- Reduce logging level in platformio.ini
- Check for memory leaks
- Reduce averaging buffer size
- Monitor free heap:
  ```cpp
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  ```
- Increase stack size if needed

### 13. Serial Monitor Shows Garbage

**Symptoms:**
- Unreadable characters in serial monitor
- Random symbols

**Solutions:**
- Set correct baud rate (115200)
- Check monitor_speed in platformio.ini
- Try different USB cable
- Reset ESP32
- Check for ground loops

### 14. OTA Updates Fail

**Symptoms:**
- Can't upload firmware over WiFi
- OTA update times out

**Solutions:**
- Ensure ESP32 and computer on same network
- Check firewall settings
- Use USB upload instead
- Verify sufficient free flash space
- Try smaller firmware (reduce features)

## Diagnostic Commands

### Check I2C Devices
```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Serial.println("\nI2C Scanner");
  
  for(byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if(Wire.endTransmission() == 0) {
      Serial.printf("Device found at 0x%02X\n", addr);
    }
  }
}
```

### Monitor LiDAR Raw Data
```cpp
void loop() {
  if(Serial2.available()) {
    uint8_t b = Serial2.read();
    Serial.printf("0x%02X ", b);
    if(b == 0x59) Serial.println();  // New frame
  }
}
```

### Test All Outputs
```cpp
void loop() {
  // Test LCD
  lcd.clear();
  lcd.print("Test 123");
  
  // Test Alarm
  digitalWrite(ALARM_PIN, HIGH);
  delay(500);
  digitalWrite(ALARM_PIN, LOW);
  
  // Test Serial
  Serial.println("All systems test");
  
  delay(2000);
}
```

## Getting Help

If you're still stuck:

1. **Check Serial Monitor Output**
   - Enable verbose logging
   - Look for error messages
   - Note any patterns

2. **Document Your Setup**
   - Hardware versions
   - Wiring configuration
   - Software versions
   - Error messages

3. **Community Support**
   - [SensESP GitHub Issues](https://github.com/SignalK/SensESP/issues)
   - [Signal K Forum](https://github.com/SignalK/signalk-server/discussions)
   - ESP32 forums and communities

4. **Provide Information**
   - Full error messages
   - Serial monitor output
   - Code modifications
   - Hardware setup photos

## Prevention Tips

- Use quality components and wiring
- Secure all connections
- Protect from moisture and vibration
- Test components individually before integration
- Keep firmware and libraries updated
- Document any modifications
- Use proper power supply (stable 5V, adequate current)
- Add decoupling capacitors for stable operation
