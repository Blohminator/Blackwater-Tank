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
const int DEFAULT_ALARM_THRESHOLD_PERCENT = 80;

// Hardware Pin Configuration
// Modify these if your wiring is different
#define RXD2 16              // LiDAR RX
#define TXD2 17              // LiDAR TX
#define ALARM_PIN 23         // Alarm output
#define LCD_I2C_ADDR 0x27    // LCD I2C address (try 0x3F if 0x27 doesn't work)
#define I2C_SDA 21           // I2C data
#define I2C_SCL 22           // I2C clock

// Timing Configuration
#define READ_INTERVAL_MS 200    // How often to read LiDAR (milliseconds)
#define LCD_UPDATE_MS 200       // How often to update LCD (milliseconds)

// Signal K Path Configuration
// Change these if you want different tank types or instances
#define SK_TANK_TYPE "blackWater"  // Options: freshWater, blackWater, greyWater, fuel, etc.
#define SK_TANK_INSTANCE 0         // Tank instance number (0, 1, 2, etc.)

// Advanced: TFmini-S Configuration
#define TFMINI_BAUD 115200
#define TFMINI_MIN_DISTANCE_CM 30
#define TFMINI_MAX_DISTANCE_CM 1200
