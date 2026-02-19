# Blackwater Tank Level Monitor

[🇩🇪 Deutsche Version](README_DE.md)

ESP32-based tank level monitoring system using TFmini-S LiDAR sensor and SensESP framework for Signal K integration.

## Features

- **TFmini-S LiDAR sensor** for non-contact distance measurement
- **Signal K integration** via SensESP v3 framework
- **LCD display** (16x2 I2C) showing fill level and volume
- **Configurable alarm** with adjustable threshold
- **Emergency operation mode** with manual override
- **Relay control** for pump or valve automation
- **Real-time monitoring** of tank level, volume, and capacity

## Hardware Requirements

- ESP32 board (Wemos D1 Mini ESP32 or compatible)
- TFmini-S LiDAR sensor
- 16x2 LCD with PCF8574 I2C backpack (address 0x27)
- Alarm output device (LED, buzzer, relay, etc.)
- 5V power supply

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

### Initial WiFi Setup

On first boot, the ESP32 creates a WiFi access point:
- SSID: `SensESP-blackwater-tank`
- Connect to this network and configure your WiFi credentials

### Web Configuration

Access the configuration interface at:
- `http://blackwater-tank.local/config` (mDNS)
- Or use the IP address shown on the LCD/serial monitor

Configure the following parameters:
- **Tank Length** (cm)
- **Tank Width** (cm)
- **Tank Height** (cm)
- **Sensor Offset** (cm) - Distance from sensor mounting to tank top
- **Alarm Threshold** (%) - Percentage at which alarm activates

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
1. Ensure clear line of sight to liquid surface
2. Avoid foam or turbulent surfaces for best accuracy
3. Set the **Sensor Offset** parameter to account for mounting distance

## Troubleshooting

### No LiDAR readings
- Check UART wiring (RX/TX may be swapped)
- Verify TFmini-S power supply (5V)
- Monitor serial output for frame errors

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

## Development

### Project Structure

```
├── platformio.ini          # PlatformIO configuration
├── src/
│   └── main.cpp           # Main application code
├── include/               # Header files (if needed)
└── lib/                   # Custom libraries (if needed)
```

### Dependencies

- [SensESP](https://github.com/SignalK/SensESP) v3.2.0+
- [LiquidCrystal_PCF8574](https://github.com/mathertel/LiquidCrystal_PCF8574) v2.2.0+
- [ArduinoJson](https://arduinojson.org/) v7.0.0+

## License

This project is open source. Check individual library licenses for dependencies.

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
