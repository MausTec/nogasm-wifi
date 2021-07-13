#ifndef __BluetoothServer_h
#define __BluetoothServer_h

#include "config.h"

#include <NimBLEDevice.h>
//#include <NimBLEUtils.h>
//#include <NimBLEServer.h>

#define SERVICE_UUID "52ecdfe9-590c-4226-8529-dd6bf4d817a6"
#define CHARACTERISTIC_UUID "9f13cb96-7739-4558-8bd7-53654909962d"

class BluetoothServer {
public:
  BluetoothServer();
  ~BluetoothServer();
  void begin();
  void advertise();
  void disconnect();
  void stopAdvertising();

private:
  BLEServer *server;
  BLEService *service;
  BLECharacteristic *characteristic;
  BLEAdvertising *advertising;
};

extern BluetoothServer BT;

#endif
