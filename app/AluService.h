// Copyright (c) 2017 Akos Kiss.
//
// Licensed under the BSD 3-Clause License
// <LICENSE.md or https://opensource.org/licenses/BSD-3-Clause>.
// This file may not be copied, modified, or distributed except
// according to those terms.

#ifndef ALUSERVICE_H
#define ALUSERVICE_H

#include "mbed.h"
#include "ble/BLE.h"
#include "main.h"
#include <math.h>

// #include <string> 
// using namespace std;

// #include <string>

/**
 * A Ticker-less (partial) implementation of the BLE Current Time Service.
 *
 * Current Time Characteristic is R/W. Every read returns the current value of
 * the RTC, while a write sets the RTC.
 * Note: The value of the characteristic is NOT updated periodically, only on
 * read requests.
 */
class AluService {
public:
    AluService(BLE &ble)
        : _ble(ble)
        , _A(0), _B(0), _result(0), _op('+'), 
        _inputACharacteristic(
            0x2D00, 
            &_A,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        ), 
        _inputBCharacteristic(
            0x2D01, 
            &_B,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        ),
        _opCharacteristic(
            0x2D02, 
            &_op,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        ),
        _resultCharacteristic(
            0x2D03, 
            &_result,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE
        )
    {
        usb.printf("[alu] starting alu service:\r\n");
        _inputACharacteristic.setReadAuthorizationCallback(this, &AluService::readAuthorizationCallback);
        _inputBCharacteristic.setReadAuthorizationCallback(this, &AluService::readAuthorizationCallback);
        _resultCharacteristic.setReadAuthorizationCallback(this, &AluService::readAuthorizationCallback);

        _inputACharacteristic.setWriteAuthorizationCallback(this, &AluService::writeAuthorizationCallback_A);
        _inputBCharacteristic.setWriteAuthorizationCallback(this, &AluService::writeAuthorizationCallback_B);
        _opCharacteristic.setWriteAuthorizationCallback(this, &AluService::writeAuthorizationCallback_op);

        GattCharacteristic *charTable[] = {
            &_inputACharacteristic, 
            &_inputBCharacteristic, 
            &_resultCharacteristic,
            &_opCharacteristic
        };
        GattService service(
            0xA000, 
            charTable, 
            sizeof(charTable) / sizeof(GattCharacteristic *)
        );

        ble.addService(service);
        // ble.onDataWritten(this, &AluService::onDataWritten);
        usb.printf("[alu] started alu service:\r\n");
    }

protected:
    // void onDataWritten(const GattWriteCallbackParams *params)
    // {
    // }

    void readAuthorizationCallback(GattReadAuthCallbackParams *params)
    {
        // memcpy(params->data, &(this->_result), sizeof(float32_t));
        usb.printf("read called.\r\n");
        params->data = (uint8_t*) &(this->_result);
        params->len = sizeof(float32_t);
        params->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }

    float parseParamsFloat(GattWriteAuthCallbackParams *params){
        return 0;
    }
    char parseParamsChar(GattWriteAuthCallbackParams *params){
        return '0';
    }

    void update(){
        switch(this->_op){
            case '+':
                this->_result = this->_A + this->_B;
                break;
            case '-':
                this->_result = this->_A - this->_B;
                break;
            case '*':
                this->_result = this->_A * this->_B;
                break;
            case '/':
                if(this->_B != 0){
                    this->_result = this->_A / this->_B;
                } else {
                    this->_result = 0xffffffff;
                }
                break;
            case '^':
                this->_result = pow(this->_A, this->_B);
                break;
            case 'r':
                this->_result = pow(this->_A, 1 / this->_B);
                break;
            default:
                break;
        }
        usb.printf("[alu] result updated    : %5.100f\r\n", this->_result);
    }

    void writeAuthorizationCallback_A(GattWriteAuthCallbackParams *params) {
        memcpy(&(this->_A), params->data, sizeof(float32_t));
        usb.printf("[alu] recieved data A   : %5.100f\r\n", this->_A);
        update();

    }
    void writeAuthorizationCallback_B(GattWriteAuthCallbackParams *params) {
        memcpy(&(this->_B), params->data, sizeof(float32_t));
        usb.printf("[alu] recieved data B   : %5.100f\r\n", this->_B);
        update();
    }
    void writeAuthorizationCallback_op(GattWriteAuthCallbackParams *params) {
        memcpy(&(this->_op), params->data, sizeof(char));
        usb.printf("[alu] recieved operator : %c\r\n", this->_op);
        update();
    }

// #define PARSE_PARAMS_TO_STRING(params, data_str)               \
//     char data_str[params->len+1];                              \
//     for(uint16_t i = 0; i < params->len; i++){                 \
//         data_str[i] = (params->data)[i];                       \
//     }                                                          \
//     data_str[params->len] = 0;                                 \

    // void writeAuthorizationCallback_Test(GattWriteAuthCallbackParams *params)
    // {
    //     usb.printf("[alu] recieved data:\r\n");
        
    //     float test;
    //     // memcpy(&test, params->data+3, sizeof(float));
    //     // memcpy(&test, params->data, sizeof(float));
    //     // memcpy(&test, params->data, sizeof(float));
    //     // memcpy(&test, params->data, sizeof(float));
    //     int testint;
    //     memcpy(&testint, params->data, sizeof(int));
    //     float dataf1 = *(params->data);
    //     float32_t dataf2 = *(params->data);
    //     int datai1 = *(params->data);
    //     int32_t datai2 = *(params->data);

    //     // // std::string data;
    //     usb.printf("[alu] as hexa   : %x;\r\n" , *(params->data));
    //     usb.printf("[alu] as hexa   : %x;\r\n" , test);
    //     usb.printf("[alu] as int   1: %d;\r\n" , datai1);
    //     usb.printf("[alu] as int   2: %d;\r\n" , datai2);
    //     usb.printf("[alu] as float 1: %10.100f;\r\n" , dataf1);
    //     usb.printf("[alu] as float 2: %10.100f;\r\n" , dataf2);
    //     usb.printf("[alu] as test i : %d;\r\n" , testint);
    //     usb.printf("[alu] as test f : %10.100f;\r\n" , test);
    //     usb.printf("[alu] as test fe: %e;\r\n" , test);
    //     usb.printf("[alu] as char   : %c;\r\n" , *(params->data));
    // }

    enum { CURRENT_TIME_DATA_SIZE = 10 };

    BLE &_ble;
    float32_t _A, _B, _result;
    char _op;
    ReadWriteGattCharacteristic<float32_t> _inputACharacteristic, _inputBCharacteristic;
    ReadOnlyGattCharacteristic<float32_t> _resultCharacteristic;
    WriteOnlyGattCharacteristic<char> _opCharacteristic;
    
};

#endif /* CURRENTTIMESERVICE_H */
