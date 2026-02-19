# Blackwater Tank Level Monitor

[🇩🇪 Deutsche Version](README_DE.md)

ESP32-based blackwater tank level monitoring system using TFmini-S LiDAR sensor and SensESP framework for Signal K integration. Designed for marine applications with real-time monitoring, emergency override, and relay control for electric toilet systems.

## Overview

This project provides a complete solution for monitoring blackwater tank levels on boats and RVs. Using a non-contact LiDAR sensor, it accurately measures tank fill levels and integrates seamlessly with Signal K marine data systems. The system includes an LCD display for local monitoring, configurable alarms, and an emergency override feature for critical situations.

## Features

- **TFmini-S LiDAR sensor** for non-contact distance measurement
- **Signal K integration** via SensESP v3 framework
- **LCD display** (16x2 I2C) showing fill level and volume
- **Configurable alarm** with adjustable threshold
- **Emergency operation mode** with manual override
- **Relay control** for electric toilet system automation
- **Real-time monitoring** of tank level, volume, and capacity
- **Web-based configuration** interface for easy setup
- **WiFi connectivity** with mDNS support
- **Robust error handling** and automatic recovery

## Hardware Requirements

- **ESP32 board** - Wemos D1 Mini ESP32 or compatible (ESP32-WROOM-32)
- **TFmini-S LiDAR sensor** - Non-contact distance measurement (30-1200cm range)
- **16x2 LCD display** - With PCF8574 I2C backpack (I2C address 0x27)
- **Relay module** - For controlling electric toilet or pump (optional)
- **Alarm device** - LED, buzzer, or external alarm system (optional)
- **5V power supply** - Minimum 2A recommended for stable operation
- **Enclosure** - Waterproof housing recommended for marine environments

## Pin Configuration

| Component | ESP32 Pin | Description |
|-----------|-----------|-------------|
| LiDAR RX | GPIO 16 | Receives data from TFmini-S TX |
| LiDAR TX | GPIO 17 | Sends commands to TFmini-S RX |
| LCD SDA | GPIO 21 | I2C data line |
| LCD SCL | GPIO 22 | I2C clock line |
| Alarm Output | GPIO 23 | Alarm output (HIGH when threshold exceeded) |
| Alarm Input | GPIO 19 | Emergency alarm input (active LOW, internal pull-up) |
| Relay Output | GPIO 18 | Relay control (HIGH=ON in normal mode, LOW=OFF in emergency) |

## Signal K Paths

The system publishes to the following Signal K paths:

- `tanks.blackWater.0.currentLevel` - Tank fill ratio (0.0 to 1.0)
- `tanks.blackWater.0.capacity` - Tank capacity in cubic meters (m³)
- `tanks.blackWater.0.currentVolume` - Current volume in cubic meters (m³)

## Installation

### Prerequisites

- [PlatformIO](https://platformio.org/) installed
- USB cable for ESP32 programming

### Steps

1. Clone this repository:
   ```bash
   git clone https://github.com/Blohminator/Blackwater-Tank.git
   cd Blackwater-Tank
   ```

2. Open the project in PlatformIO (VS Code or CLI)

3. Connect your ESP32 board via USB

4. Build and upload:
   ```bash
   pio run -t upload
   ```

5. Monitor serial output:
   ```bash
   pio device monitor
   ```

## Configuration

### Tank Parameters

The default tank configuration in the code:
```cpp
float length_cm = 50.0f;   // Tank length in centimeters
float width_cm = 72.0f;    // Tank width in centimeters
float height_cm = 87.0f;   // Tank height in centimeters
float sensor_offset_cm = 5.0f;  // Distance from sensor to tank top
int alarm_threshold_percent = 80;  // Alarm triggers at 80% full
```

Modify these values in `src/main.cpp` to match your tank dimensions.

### Initial WiFi Setup

On first boot, the ESP32 creates a WiFi access point:
- **SSID:** `SensESP-blackwater-tank`
- **Password:** Check serial monitor or SensESP documentation
- Connect to this network and configure your WiFi credentials through the captive portal

### Web Configuration

After WiFi is configured, access the web interface:
- **mDNS:** `http://blackwater-tank.local/config`
- **Direct IP:** Check LCD display or serial monitor for IP address

The web interface provides:
- Real-time tank level visualization
- Signal K connection status
- System information and diagnostics
- Network configuration
- Firmware update capability

### Signal K Server

Configure your Signal K server address in the web interface or by modifying the code:
```cpp
builder
  .set_hostname("blackwater-tank")
  .set_sk_server("192.168.1.50", 3000)  // Add your Signal K server IP
  .get_app();
```

## LCD Display

The LCD shows real-time information in two modes:

### Normal Mode
**Line 1:** `Fill:XXcm YYY%`
- Fill height in centimeters
- Fill percentage (0-100%)

**Line 2:** `Vol:XXXXXXL`
- Current volume in liters

### Emergency Mode
**Line 1:** `Fill:XXcm YYY%`
- Fill height in centimeters
- Fill percentage (0-100%)

**Line 2:** `EMERGENCY MODE`
- Indicates emergency operation is active

## Emergency Operation

The system includes an emergency override feature:

**Normal Operation:**
- Relay output: HIGH (ON)
- Alarm output: Based on threshold
- Display: Shows fill level and volume

**Emergency Mode Activation:**
When BOTH conditions are met:
1. Fill level exceeds alarm threshold (default 80%)
2. Alarm input (GPIO 19) is closed (connected to GND)

**Emergency Mode Behavior:**
- Relay output: LOW (OFF) - switches to normal/safe state
- Alarm output: HIGH (active)
- Display: Shows "EMERGENCY MODE" message
- Serial: Logs emergency activation

**Exit Emergency Mode:**
When the alarm input (GPIO 19) is opened (disconnected), the system automatically returns to normal operation.

## TFmini-S LiDAR

The TFmini-S sensor measures distance using UART communication:
- Baud rate: 115200
- Frame format: 9 bytes with checksum
- Measurement range: 30cm to 1200cm
- Accuracy: ±5cm

### Mounting

Mount the sensor above the tank opening, pointing downward:

⚠️ **IMPORTANT: Moisture Protection Required!**
- The LiDAR sensor must NOT come into direct contact with water or moisture
- Install a protective glass or clear plastic barrier between the sensor and tank opening
- Use transparent materials (glass, acrylic, polycarbonate) that allow laser light to pass through
- Ensure the protective barrier is clean and free of condensation for accurate readings
- Seal the sensor housing to prevent humidity and gases from entering

Additional mounting guidelines:
1. Ensure clear line of sight to liquid surface through the protective barrier
2. Avoid foam or turbulent surfaces for best accuracy
3. Mount the protective barrier at an angle to prevent condensation buildup
4. Set the **Sensor Offset** parameter to account for mounting distance and barrier thickness
5. Regular cleaning of the protective barrier maintains measurement accuracy

## Troubleshooting

### No LiDAR readings
- Check UART wiring (RX/TX may be swapped)
- Verify TFmini-S power supply (5V)
- Monitor serial output for frame errors
- **Check protective barrier:** Ensure glass/plastic barrier is clean and transparent
- **Moisture damage:** If sensor was exposed to water, it may be permanently damaged

### Inaccurate or fluctuating readings
- Clean the protective glass/plastic barrier
- Check for condensation on the protective barrier
- Verify sensor is mounted perpendicular to liquid surface
- Ensure protective barrier material is suitable (clear glass or acrylic recommended)
- Adjust sensor offset parameter if barrier thickness changed

### LCD not displaying
- Verify I2C address (default 0x27, some use 0x3F)
- Check I2C wiring (SDA/SCL)
- Test with I2C scanner sketch

### Signal K not connecting
- Verify WiFi connection
- Check Signal K server IP and port
- Ensure Signal K server is running and accessible

### Alarm not working
- Check GPIO 23 connection
- Verify alarm threshold setting
- Test with multimeter or LED

## Technical Details

### System Architecture

The system operates on a 200ms update cycle:
1. Read distance from TFmini-S LiDAR sensor via UART
2. Calculate fill level, percentage, and volume
3. Update LCD display with current values
4. Check alarm conditions and emergency mode
5. Publish data to Signal K server
6. Process SensESP event loop

### Communication Protocols

- **UART:** TFmini-S LiDAR (115200 baud, 8N1)
- **I2C:** LCD display (100kHz, address 0x27)
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Signal K:** WebSocket over HTTP (default port 3000)

## Development

### Project Structure

```
├── platformio.ini          # PlatformIO configuration
├── src/
│   └── main.cpp           # Main application code
├── include/               # Header files
├── lib/                   # Custom libraries
├── DOCUMENTATION_INDEX.md  # Documentation overview
├── EMERGENCY_MODE_LOGIC.md # Emergency mode details
└── config_example.h       # Configuration template
```

### Dependencies

- [SensESP](https://github.com/SignalK/SensESP) v3.2.0+
- [LiquidCrystal_PCF8574](https://github.com/mathertel/LiquidCrystal_PCF8574) v2.2.0+
- [ArduinoJson](https://arduinojson.org/) v7.0.0+

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## License

This project is open source. Check individual library licenses for dependencies.

## Author

Created by Stefan Blohm (Blohminator)

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and updates.

## References

- [SensESP Documentation](https://signalk.org/SensESP/)
- [Signal K](https://signalk.org/)
- [TFmini-S Datasheet](https://www.benewake.com/en/tfmini-s.html)
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

## Support

For issues and questions:
- Open an issue on [GitHub](https://github.com/Blohminator/Blackwater-Tank/issues)
- SensESP: [GitHub Issues](https://github.com/SignalK/SensESP/issues)
- Signal K: [Community Forum](https://github.com/SignalK/signalk-server/discussions)
