# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2026-02-08

### Added
- Initial release of Blackwater Tank Level Monitor
- TFmini-S LiDAR sensor integration for non-contact distance measurement
- SensESP v3 framework integration for Signal K compatibility
- 16x2 LCD I2C display showing fill level and volume
- Web-based configuration interface for tank parameters
- Configurable alarm output with adjustable threshold
- Signal K outputs for:
  - `tanks.blackWater.0.currentLevel` (ratio 0-1)
  - `tanks.blackWater.0.capacity` (m³)
  - `tanks.blackWater.0.currentVolume` (m³)
- Real-time tank monitoring with 200ms update rate
- Automatic capacity calculation from tank dimensions
- Flicker-free LCD display updates
- UART frame parsing for TFmini-S protocol
- Checksum validation for reliable readings

### Configuration
- Tank length, width, height (cm)
- Sensor offset distance (cm)
- Alarm threshold percentage (0-100%)
- All parameters configurable via web UI at `/config`

### Hardware Support
- ESP32 boards (tested on Wemos D1 Mini ESP32)
- TFmini-S LiDAR sensor (UART, 115200 baud)
- LCD 16x2 with PCF8574 I2C backpack (address 0x27)
- Alarm output on GPIO 23

### Documentation
- Complete README with features and installation
- Detailed wiring guide (WIRING.md)
- Quick start guide (QUICKSTART.md)
- Comprehensive troubleshooting guide (TROUBLESHOOTING.md)
- Configuration examples (config_example.h)

### Dependencies
- SensESP ^3.2.0
- LiquidCrystal_PCF8574 ^2.2.0
- ArduinoJson ^7.0.0
- ESP32 Arduino framework

## [Unreleased]

### Planned Features
- Multiple tank support (fresh water, grey water, fuel)
- Temperature compensation for volume calculations
- Historical data logging
- Low battery warning
- Email/SMS notifications
- Mobile app integration
- Non-rectangular tank support (cylindrical, custom shapes)
- Averaging and filtering options
- Calibration wizard
- Backup/restore configuration
- Multi-language support

### Known Issues
- None reported yet

## Version History

- **1.0.0** - Initial release with core functionality
