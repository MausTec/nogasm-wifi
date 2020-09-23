#include "../include/BluetoothServer.h"

BluetoothServer::BluetoothServer() {

}

void BluetoothServer::begin() {
  BLEDevice::init(BLUETOOTH_NAME);
  this->server = BLEDevice::createServer();
  this->service = this->server->createService(SERVICE_UUID);

  this->characteristic = this->service->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_WRITE
  );

  this->characteristic->setValue("What?");
}

void BluetoothServer::advertise() {
  this->advertising = this->server->getAdvertising();
  this->advertising->start();
}