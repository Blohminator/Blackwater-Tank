# Documentation Index

Complete documentation for the Blackwater Tank Level Monitor project.

## Quick Links

### Getting Started
- **[README.md](README.md)** - Project overview, features, and installation
- **[QUICKSTART.md](QUICKSTART.md)** - 15-minute setup guide
- **[WIRING.md](WIRING.md)** - Complete wiring diagrams and connections

### Configuration & Operation
- **[EMERGENCY_MODE_LOGIC.md](EMERGENCY_MODE_LOGIC.md)** - Emergency mode state machine and behavior
- **[platformio.ini](platformio.ini)** - Build configuration and dependencies

### Troubleshooting & Debugging
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - Common issues and solutions
- **[RELAY_TEST.md](RELAY_TEST.md)** - Relay debugging and diagnostics
- **[NEXT_STEPS.md](NEXT_STEPS.md)** - Current debugging steps
- **[EMERGENCY_MODE_FIX.md](EMERGENCY_MODE_FIX.md)** - Display update fix documentation

### Source Code
- **[src/main.cpp](src/main.cpp)** - Main application code

## Documentation Overview

### README.md
Complete project documentation including:
- Hardware requirements
- Pin configuration
- Signal K integration
- Installation instructions
- Configuration options
- LCD display modes
- Emergency operation details

### QUICKSTART.md
Step-by-step guide to get running in 15 minutes:
1. Wire components
2. Upload firmware
3. Connect to WiFi
4. Configure tank settings
5. Test and calibrate
6. Mount and deploy

### WIRING.md
Detailed wiring information:
- Component connections
- TFmini-S LiDAR wiring
- LCD I2C connections
- Alarm output options
- Alarm input (emergency override)
- Relay output for pump/valve control
- Power supply options
- Complete wiring diagrams
- Testing procedures

### EMERGENCY_MODE_LOGIC.md
Emergency mode state machine documentation:
- Normal mode behavior
- Emergency mode activation conditions
- State transitions
- Relay and alarm logic
- Example scenarios
- Testing procedures

**Key Points:**
- Emergency activates when: Level ≥80% AND Pin 19 closed
- Emergency stays active: As long as Pin 19 is closed
- Relay behavior:
  - Normal + <80%: Relay ON
  - Normal + ≥80%: Relay OFF (alarm state)
  - Emergency: Relay ON (safe state)

### TROUBLESHOOTING.md
Comprehensive troubleshooting guide:
- ESP32 upload issues
- LCD display problems
- LiDAR sensor issues
- WiFi connectivity
- Signal K integration
- Alarm output
- Volume calculations
- Compilation errors
- Memory issues
- Diagnostic commands

### RELAY_TEST.md
Relay debugging documentation:
- Hardware problem diagnosis
- Pin conflict detection
- Current requirements
- Alternative GPIO pins
- Transistor/MOSFET solutions
- Optocoupler isolation
- Hardware testing procedures

### NEXT_STEPS.md
Current debugging workflow:
- Firmware upload steps
- Serial monitor output interpretation
- Pin state diagnosis
- Hardware testing with multimeter
- Pin change instructions

### EMERGENCY_MODE_FIX.md
Historical fix documentation:
- Display update issue after emergency mode
- Root cause analysis
- Code changes made
- Testing recommendations

## System Architecture

### Hardware Components
- ESP32 (Wemos D1 Mini)
- TFmini-S LiDAR sensor (UART)
- OLED SSD1306 128x64 (Software I2C, GPIO 32/33)
- Alarm/relay output (GPIO 23, HW-482 active-high)
- Emergency input (GPIO 19, active-LOW)

### Software Stack
- PlatformIO build system
- Arduino framework
- SensESP v3.2.0 (Signal K integration)
- U8g2 (OLED driver)
- ArduinoJson v7.0.0

### Signal K Integration
Published paths:
- `tanks.blackWater.0.currentLevel` - Fill ratio (0.0-1.0)
- `tanks.blackWater.0.capacity` - Tank capacity (m³)
- `tanks.blackWater.0.currentVolume` - Current volume (m³)

## Configuration Parameters

### Tank Dimensions (in code)
```cpp
float length_cm = 50.0f;
float width_cm = 72.0f;
float height_cm = 87.0f;
float sensor_offset_cm = 5.0f;
int alarm_threshold_percent = 80;
```

### GPIO Pin Assignments
```cpp
#define TF_RX         16   // LiDAR RX
#define TF_TX         17   // LiDAR TX
#define ALARM_PIN     23   // Relay output (HW-482, active-high)
#define EMERGENCY_PIN 19   // Emergency input (active-LOW)
#define OLED_SDA      32   // OLED Software I2C data
#define OLED_SCL      33   // OLED Software I2C clock
```

## Current Status

✅ **Working:**
- TFmini-S LiDAR distance measurement
- OLED SSD1306 display with real-time updates
- Signal K integration
- Alarm relay at 85% threshold (with hysteresis to 75%)
- Emergency mode state machine

✅ **Tested:**
- Normal mode operation
- Alarm threshold triggering
- Relay switching in normal and emergency modes
- Display updates in both modes
- Emergency mode activation/deactivation

## Support Resources

### Online Resources
- [SensESP Documentation](https://signalk.org/SensESP/)
- [Signal K](https://signalk.org/)
- [TFmini-S Datasheet](https://www.benewake.com/en/tfmini-s.html)
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

### Community Support
- [SensESP GitHub Issues](https://github.com/SignalK/SensESP/issues)
- [Signal K Forum](https://github.com/SignalK/signalk-server/discussions)

## Version History

See [CHANGELOG.md](CHANGELOG.md) for detailed version history.

## License

This project is open source. Check individual library licenses for dependencies.
