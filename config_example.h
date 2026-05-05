/*
 * Configuration Example
 * Copy this to your main.cpp and customize as needed
 */

// WiFi Configuration (optional - can be set via web UI)
// Uncomment and modify these lines in setup() if you want hardcoded WiFi:
/*
builder
  .set_hostname("blackwater-tank")
  .set_wifi_client("YOUR_SSID", "YOUR_PASSWORD")
  .set_sk_server("192.168.1.50", 3000)  // Your Signal K server IP and port
  .get_app();
*/

// Tank Configuration Defaults
// These can be changed via web UI at runtime
const float DEFAULT_TANK_LENGTH_CM = 50.0f;
const float DEFAULT_TANK_WIDTH_CM = 72.0f;
const float DEFAULT_TANK_HEIGHT_CM = 87.0f;
const float DEFAULT_SENSOR_OFFSET_CM = 5.0f;
const float DEFAULT_ALARM_HIGH_PERCENT = 85.0f;  // Relay ON threshold
const float DEFAULT_ALARM_LOW_PERCENT  = 75.0f;  // Relay OFF threshold (hysteresis)

// Hardware Pin Configuration
// Modify these if your wiring is different
#define TF_RX         16   // LiDAR RX
#define TF_TX         17   // LiDAR TX
#define ALARM_PIN     23   // Relay output (HW-482, active-high)
#define EMERGENCY_PIN 19   // Emergency input (active-LOW)
#define OLED_SDA      32   // OLED Software I2C data
#define OLED_SCL      33   // OLED Software I2C clock

// Timing Configuration
#define READ_INTERVAL_MS 100    // How often to read LiDAR (milliseconds)

// Signal K Path Configuration
// Change these if you want different tank types or instances
#define SK_TANK_TYPE "blackWater"  // Options: freshWater, blackWater, greyWater, fuel, etc.
#define SK_TANK_INSTANCE 0         // Tank instance number (0, 1, 2, etc.)

// Advanced: TFmini-S Configuration
#define TFMINI_BAUD 115200
#define TFMINI_MIN_DISTANCE_CM 30
#define TFMINI_MAX_DISTANCE_CM 1200
