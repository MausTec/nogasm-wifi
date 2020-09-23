#ifndef __BluetoothServer_h
#define __BluetoothServer_h

#include <arduino.h>
#include "../../config.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define BLUETOOTH_NAME "EdgeLord 3000"
#define SERVICE_UUID "52ecdfe9-590c-4226-8529-dd6bf4d817a6"
#define CHARACTERISTIC_UUID "9f13cb96-7739-4558-8bd7-53654909962d"

class BluetoothServer {
public:
  BluetoothServer();
  void begin();
  void advertise();

private:
  BLEServer *server;
  BLEService *service;
  BLECharacteristic *characteristic;
  BLEAdvertising *advertising;
};

#endif