//
// Created by churunfa on 2026/2/19.
//

#ifndef SWITCHPROCONTROLLERESP32S3_NATIVEBLEREADER_H
#define SWITCHPROCONTROLLERESP32S3_NATIVEBLEREADER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class NativeBLEReader {
private:
    BLEServer* _pServer = nullptr;
    BLECharacteristic* _pTxChar = nullptr;
    bool _connected = false;

    const char* SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    const char* RX_UUID      = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    const char* TX_UUID      = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

    class MyServerCallbacks : public BLEServerCallbacks {
        NativeBLEReader* _outer;
    public:
        MyServerCallbacks(NativeBLEReader* outer) : _outer(outer) {}
        void onConnect(BLEServer* pServer) override { _outer->_connected = true; }
        void onDisconnect(BLEServer* pServer) override {
            _outer->_connected = false;
            // NimBLE 风格的自动广播处理通常在外部或通过 startAdvertising
            BLEDevice::startAdvertising();
        }
    };

    class MyCharCallbacks : public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic* pChar) override {
            // 修正：NimBLE 返回的是 String，直接用 c_str() 打印
            String value = pChar->getValue();
            if (value.length() > 0) {
                for (int i = 0; i < value.length(); i++) {
                    ReadStrategyProcess::getInstance().process(value[i]);
                }
            }
        }
    };

public:
    void begin(const char* deviceName) {
        BLEDevice::init(deviceName);
        _pServer = BLEDevice::createServer();
        _pServer->setCallbacks(new MyServerCallbacks(this));

        BLEService* pService = _pServer->createService(SERVICE_UUID);

        _pTxChar = pService->createCharacteristic(
            TX_UUID,
            BLECharacteristic::PROPERTY_NOTIFY
        );

        // 修正：NimBLE 会自动处理 2902，手动添加会报 Deprecated 警告
        // _pTxChar->addDescriptor(new BLE2902());

        BLECharacteristic* pRxChar = pService->createCharacteristic(
            RX_UUID,
            BLECharacteristic::PROPERTY_WRITE
        );
        pRxChar->setCallbacks(new MyCharCallbacks());

        pService->start();
        BLEAdvertising* pAdv = BLEDevice::getAdvertising();
        pAdv->addServiceUUID(SERVICE_UUID);
        pAdv->start();
    }

    bool isConnected() { return _connected; }

    void send(String data) {
        if (_connected && _pTxChar) {
            _pTxChar->setValue(data.c_str());
            _pTxChar->notify();
        }
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_NATIVEBLEREADER_H