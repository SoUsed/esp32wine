#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

#include <freertos/BLEDevice.h>
#include "esp_blufi_api.h"
#include "blufi_example.h"

#include "esp_blufi.h"
#include <freertos/HTTPClient.h>
#include <freertos/WiFi.h>
#include <freertos/WiFiClient.h>
#include <freertos/WiFiClientSecure.h>
#include <vector>

// Struct to hold temperature and humidity data
struct SensorData {
  float temperature;
  int humidity;
};

// Constants for report generation
const int REPORT_MINUTES = 30;
const int REPORT_HOURS = 24;
const int REPORT_SIZE = 5; // Event ID + 2 pairs of temperature/humidity data
const float MIN_TEMPERATURE = 20.0;
const float MAX_TEMPERATURE = 27.0;
const int MIN_HUMIDITY = 50;
const int MAX_HUMIDITY = 70;

// Global variables
std::vector<SensorData> dataBuffer;
unsigned long lastReportTime = 0;
unsigned long lastDailyReportTime = 0;

// Function to pack temperature and humidity data into a hex string
String packSensorData(float temperature, int humidity) {
  // Round temperature to 0.1 precision
  temperature = roundf(temperature * 10) / 10;
  // Convert temperature to 9-bit integer
  int tempInt = int((temperature + 5.5) * 10) & 0x1FF;
  // Convert humidity to 7-bit integer
  int humInt = humidity & 0x7F;
  // Pack temperature and humidity into 2 bytes
  uint16_t packedData = (tempInt << 7) | humInt;
  // Convert packed data to hex string
  return String(packedData, HEX);
}

// Function to generate a report
String generateReport(int eventId, bool includeMinMax = true) {
  String report;
  // Add event ID to report
  report += String(eventId, HEX);
  // Calculate average temperature and humidity
  float avgTemp = 0;
  int avgHum = 0;
  for (const auto &data : dataBuffer) {
    avgTemp += data.temperature;
    avgHum += data.humidity;
  }
  avgTemp /= dataBuffer.size();
  avgHum /= dataBuffer.size();
  // Pack average temperature and humidity into report
  report += packSensorData(avgTemp, avgHum);
  // If requested, calculate and pack highest and lowest temperature and
  // humidity
  if (includeMinMax) {
    float minTemp = MAX_TEMPERATURE;
    float maxTemp = MIN_TEMPERATURE;
    int minHum = MAX_HUMIDITY;
    int maxHum = MIN_HUMIDITY;
    for (const auto &data : dataBuffer) {
      if (data.temperature < minTemp) {
        minTemp = data.temperature;
      }
      if (data.temperature > maxTemp) {
        maxTemp = data.temperature;
      }
      if (data.humidity < minHum) {
        minHum = data.humidity;
      }
      if (data.humidity > maxHum) {
        maxHum = data.humidity;
      }
    }
    // Pack highest and lowest temperature and humidity into report
    report += packSensorData(maxTemp, maxHum);
    report += packSensorData(minTemp, minHum);
  }
  // Clear data buffer
  dataBuffer.clear();
  return report;
}

// Function to send a report via HTTP
void sendReport(String report) {
  // Connect to WiFi network
  WiFi.begin("exampleSSID", "examplePWD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  // Make HTTP request
  HTTPClient http;
  http.begin("http://example_endpoint.com");
  http.addHeader("Content-Type", "application/json");
  String requestBody = "{\"data\": \"" + report + "\"}";
  int httpResponseCode = http.POST(requestBody);
  // Check for HTTP response code and print debug output
  if (httpResponseCode == HTTP_CODE_OK) {
    Serial.println("Report sent successfully");
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  // Initialize BLE
  BLEDevice::init("");
  // Set up BLE scan filter
  BLEScanFilter filter;
  filter.setAddress("00:11:22:33:44:55");
  BLEScan filterScan;
  filterScan.setFilter(filter);
}

void loop() {
  // Scan for BLE devices and collect sensor data
  BLEScanResults scanResults = BLEDevice::getScanResults();
  for (int i = 0; i < scanResults.getCount(); i++) {
    BLEAdvertisedDevice device = scanResults.getDevice(i);
    if (device.getAddress().toString() == "00:11:22:33:44:55") {
      // Extract temperature and humidity data from BLE payload
      std::string payload = device.getPayload();
      float temperature = *(float *)(&payload[0]);
      int humidity = *(int *)(&payload[sizeof(float)]);
      // Check if data is within acceptable range
      if (temperature >= MIN_TEMPERATURE && temperature <= MAX_TEMPERATURE &&
          humidity >= MIN_HUMIDITY && humidity <= MAX_HUMIDITY) {
        // Add data to buffer
        SensorData data = {temperature, humidity};
        dataBuffer.push_back(data);
      } else {
        // Data is outside acceptable range, generate and send report with event
        // ID 3
        String report = generateReport(3, false);
        sendReport(report);
      }
    }
  }
  // Check if it's time to generate a 30-minute report
  unsigned long currentTime = millis();
  if (currentTime - lastReportTime >= REPORT_MINUTES * 60 * 1000) {
    String report = generateReport(1);
    sendReport(report);
    lastReportTime = currentTime;
  }
  // Check if it's time to generate a daily report
  if (currentTime - lastDailyReportTime >= REPORT_HOURS * 60 * 60 * 1000) {
    String report = generateReport(2);
    sendReport(report);
    lastDailyReportTime = currentTime;
  }
}
