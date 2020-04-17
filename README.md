# ESP32-temperature-logger
Temperature logger to InfluxDB database with LM75A sensor
## Before compiling
Before compiling you must create a new file called `credentials.h` in `src` folder
In this file there are typed things as a InfluxDB database name, IP addres of server, WiFi SSID and password and so on.  
__File structure:__
```cpp
const char WIFI_NAME[] = "";            // WiFi SSID
const char WIFI_PASS[] = "";            // Wifi password
const char INFLUX_DATABASE[] = "";      // InfluxDB database name
const char INFLUX_IP[] = "";            // IP address of InfluxDB server
const char INFLUX_USER[] = "";          // InfluxDB username
const char INFLUX_PASS[] = "";          // InfluxDB password
const char INFLUX_MEASUREMENT[] = "";   // InfluxDB name of column in database
```
