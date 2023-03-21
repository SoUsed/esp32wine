#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "types.hpp"
#include "model.hpp"

// BLE configuration
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0" // insert your esp32 device uuid
#define CHARACTERISTIC_UUID "87654321-4321-8765-4321-87654321fedc" // insert your esp32 device uuid

class SensorCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
    }
};

static SensorModel model;

void setup() {
  // Initialize BLE
  BLEDevice::init("Sensor Emulator");

  model = DEFAULT_MODEL;

  // Create BLE server
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE characteristic
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  pCharacteristic->addDescriptor(new BLE2902());

  // Set characteristic callbacks
  pCharacteristic->setCallbacks(new SensorCharacteristicCallbacks());

  // Start service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
}

void loop() {
  // Update sensor data
  model.update();
  
  // Pack sensor data into byte array
  model.step();
  sensor_payload_t data = model.current_data.hex();

  // Send sensor data via BLE
  BLECharacteristic *pCharacteristic = BLEDevice::getCharacteristic(CHARACTERISTIC_UUID);
  pCharacteristic->setValue(data, sizeof(data));
  pCharacteristic->notify();

  // Wait a few seconds before sending the next update
  delay(5000);
}
