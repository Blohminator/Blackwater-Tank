# Quick Start Guide

Get your blackwater tank monitor up and running in 15 minutes!

## What You Need

- ✅ ESP32 board (Wemos D1 Mini ESP32)
- ✅ TFmini-S LiDAR sensor
- ✅ 16x2 LCD with I2C backpack
- ✅ USB cable
- ✅ Jumper wires
- ✅ 5V power supply (or USB power)
- ✅ Computer with PlatformIO installed

## Step 1: Wire Everything (5 minutes)

### Minimal Wiring for Testing

```
TFmini-S LiDAR:
  Red (5V)    → ESP32 5V
  Black (GND) → ESP32 GND
  Green (TX)  → ESP32 GPIO 16
  White (RX)  → ESP32 GPIO 17

LCD Display:
  VCC → ESP32 5V
  GND → ESP32 GND
  SDA → ESP32 GPIO 21
  SCL → ESP32 GPIO 22

Alarm (Optional):
  LED + Resistor → ESP32 GPIO 23 → GND
```

**Quick Check:** Power on and verify LCD backlight turns on.

## Step 2: Upload Firmware (5 minutes)

### Using PlatformIO

1. **Open project in VS Code**
   ```
   File → Open Folder → Select this project folder
   ```

2. **Connect ESP32 via USB**

3. **Upload firmware**
   - Click the → (Upload) button in PlatformIO toolbar
   - Or press `Ctrl+Alt+U` (Windows/Linux) or `Cmd+Alt+U` (Mac)
   - Or use terminal: `pio run -t upload`

4. **Wait for upload to complete** (~30 seconds)

### First Boot

The LCD should show:
```
Initializing...
Please wait
```

Then:
```
SensESP Ready
TFmini-S LiDAR
```

## Step 3: Connect to WiFi (3 minutes)

### Option A: Web Configuration (Recommended)

1. **Find the WiFi network**
   - Look for: `SensESP-blackwater-tank`
   - Connect to it (no password)

2. **Configure WiFi**
   - Browser should auto-open to config page
   - If not, go to: `http://192.168.4.1`
   - Enter your WiFi SSID and password
   - Click Save

3. **ESP32 will reboot and connect**
   - LCD will show IP address
   - Note this IP for later

### Option B: Hardcode WiFi (Alternative)

Edit `src/main.cpp` around line 120:
```cpp
builder
  .set_hostname("blackwater-tank")
  .set_wifi_client("YOUR_SSID", "YOUR_PASSWORD")  // Add this line
  .get_app();
```

Re-upload firmware.

## Step 4: Configure Tank Settings (2 minutes)

1. **Access web interface**
   - Go to: `http://blackwater-tank.local/config`
   - Or use IP address: `http://192.168.1.XXX/config`

2. **Enter tank dimensions**
   - Length (cm): Your tank length
   - Width (cm): Your tank width
   - Height (cm): Your tank height
   - Sensor Offset (cm): Distance from sensor to tank top edge
   - Alarm Threshold (%): When to trigger alarm (default 80%)

3. **Click Save**

## Step 5: Test It! (1 minute)

### Check LCD Display

You should see:
```
Fill:XXcm YYY%
Vol:XXXXXXL
```

### Test LiDAR Reading

1. Point sensor at a surface
2. Move your hand closer/farther
3. Watch values change on LCD

### Test Alarm

1. Set alarm threshold to 0% in web config
2. Alarm should activate (LED lights up)
3. Set back to desired threshold (e.g., 80%)

## Step 6: Mount and Calibrate

### Mounting

1. **Position sensor above tank opening**
   - Point straight down
   - Ensure clear line of sight to liquid
   - Secure firmly (no vibration)

2. **Measure sensor offset**
   - Distance from sensor to tank top edge
   - Enter this value in web config

### Calibration

1. **Empty tank test**
   - LCD should show ~0% fill
   - If not, adjust sensor offset

2. **Full tank test**
   - LCD should show ~100% fill
   - Verify volume calculation is correct

3. **Adjust if needed**
   - Fine-tune sensor offset
   - Verify tank dimensions

## Step 7: Connect to Signal K (Optional)

If you have a Signal K server:

1. **Configure server address**
   - Edit `src/main.cpp` around line 120:
   ```cpp
   builder
     .set_hostname("blackwater-tank")
     .set_sk_server("192.168.1.50", 3000)  // Your Signal K IP
     .get_app();
   ```

2. **Re-upload firmware**

3. **Check Signal K**
   - Open Signal K dashboard
   - Look for: `tanks.blackWater.0.*`
   - You should see:
     - currentLevel (ratio)
     - capacity (m³)
     - currentVolume (m³)

## Troubleshooting Quick Fixes

### LCD is blank
- Adjust contrast screw on I2C backpack
- Try address 0x3F instead of 0x27

### No LiDAR readings
- **Swap RX/TX wires** (most common!)
- Check 5V power to LiDAR

### Can't find WiFi network
- Wait 30 seconds after boot
- Check serial monitor for status
- Reset ESP32

### Web page won't load
- Use IP address instead of hostname
- Check ESP32 is on same WiFi network
- Try different browser

## Serial Monitor

To see debug output:

1. **Open serial monitor**
   - Click 🔌 icon in PlatformIO toolbar
   - Or: `pio device monitor`

2. **Set baud rate to 115200**

3. **You should see:**
   ```
   === TFmini-S LiDAR Tank Monitor ===
   SensESP v3 - Blackwater Tank
   LiDAR UART initialized on Serial2
   SensESP app created
   Configuration UI registered
   Signal K outputs initialized
   Setup complete!
   ```

## Next Steps

- ✅ **Read the full [README.md](README.md)** for detailed information
- ✅ **Check [WIRING.md](WIRING.md)** for permanent installation
- ✅ **Review [TROUBLESHOOTING.md](TROUBLESHOOTING.md)** if issues arise
- ✅ **Customize settings** via web interface
- ✅ **Set up proper power supply** for permanent installation
- ✅ **Weatherproof enclosure** for marine environment

## Success Checklist

- [ ] LCD shows fill level and volume
- [ ] Values change when sensor distance changes
- [ ] Web interface accessible
- [ ] Tank dimensions configured correctly
- [ ] Alarm triggers at threshold
- [ ] Signal K receiving data (if configured)
- [ ] Sensor mounted securely
- [ ] Readings are stable and accurate

## Need Help?

- 📖 Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- 💬 [SensESP GitHub](https://github.com/SignalK/SensESP)
- 🌐 [Signal K Community](https://github.com/SignalK/signalk-server/discussions)

**Congratulations! Your tank monitor is ready! 🎉**
