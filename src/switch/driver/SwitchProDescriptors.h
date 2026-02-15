/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef SWITCHPROCONTROLLERESP32S3_SWITCH_PRO_DESCRIPTORS_H
#define SWITCHPROCONTROLLERESP32S3_SWITCH_PRO_DESCRIPTORS_H

#pragma once

#define SWITCH_PRO_ENDPOINT_SIZE 64

#define SWITCH_PRO_VENDOR_ID     0x057E
#define SWITCH_PRO_PRODUCT_ID    0x2009

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

// HAT report (4 bits)
#define SWITCH_PRO_HAT_UP        0x00
#define SWITCH_PRO_HAT_UPRIGHT   0x01
#define SWITCH_PRO_HAT_RIGHT     0x02
#define SWITCH_PRO_HAT_DOWNRIGHT 0x03
#define SWITCH_PRO_HAT_DOWN      0x04
#define SWITCH_PRO_HAT_DOWNLEFT  0x05
#define SWITCH_PRO_HAT_LEFT      0x06
#define SWITCH_PRO_HAT_UPLEFT    0x07
#define SWITCH_PRO_HAT_NOTHING   0x08

#define SWITCH_PRO_MASK_ZR (1U << 7)
#define SWITCH_PRO_MASK_R (1U << 6)
#define SWITCH_PRO_MASK_A (1U << 3)
#define SWITCH_PRO_MASK_B (1U << 2)
#define SWITCH_PRO_MASK_X (1U << 1)
#define SWITCH_PRO_MASK_Y 1U

#define SWITCH_PRO_MASK_CAPTURE (1U << 5)
#define SWITCH_PRO_MASK_HOME (1U << 4)
#define SWITCH_PRO_MASK_L3 (1U << 3)
#define SWITCH_PRO_MASK_R3 (1U << 2)
#define SWITCH_PRO_MASK_PLUS (1U << 1)
#define SWITCH_PRO_MASK_MINUS 1U

#define SWITCH_PRO_MASK_ZL (1U << 7)
#define SWITCH_PRO_MASK_L (1U << 6)

// Switch analog sticks only report 8 bits
//#define SWITCH_PRO_JOYSTICK_MIN 0x0000
//#define SWITCH_PRO_JOYSTICK_MID 0x07FF
//#define SWITCH_PRO_JOYSTICK_MAX 0x0FFF

#define SWITCH_PRO_JOYSTICK_MIN 0x0000
#define SWITCH_PRO_JOYSTICK_MID 0x7FFF
#define SWITCH_PRO_JOYSTICK_MAX 0xFFFF

typedef enum {
    REPORT_OUTPUT_00 = 0x00,
    REPORT_FEATURE = 0x01,
    REPORT_OUTPUT_10 = 0x10,
    REPORT_OUTPUT_21 = 0x21,
    REPORT_OUTPUT_30 = 0x30,
    REPORT_CONFIGURATION = 0x80,
    REPORT_USB_INPUT_81 = 0x81,
} SwitchReportID;

typedef enum {
    IDENTIFY = 0x01,
    HANDSHAKE = 0x02,
    BAUD_RATE = 0x03,
    DISABLE_USB_TIMEOUT = 0x04,
    ENABLE_USB_TIMEOUT  = 0x05
} SwitchOutputSubtypes;

typedef enum {
    GET_CONTROLLER_STATE = 0x00,
    BLUETOOTH_PAIR_REQUEST = 0x01,
    REQUEST_DEVICE_INFO = 0x02,
    SET_MODE = 0x03,
    TRIGGER_BUTTONS = 0x04,
    SET_SHIPMENT = 0x08,
    SPI_READ = 0x10,
    SPI_WRITE = 0x11,
    SET_NFC_IR_CONFIG = 0x21,
    SET_NFC_IR_STATE = 0x22,
    SET_PLAYER_LIGHTS = 0x30,
    GET_PLAYER_LIGHTS = 0x31,
    COMMAND_UNKNOWN_33 = 0x33,
    SET_HOME_LIGHT = 0x38,
    TOGGLE_IMU = 0x40,
    IMU_SENSITIVITY = 0x41,
    READ_IMU = 0x43,
    ENABLE_VIBRATION = 0x48,
    GET_VOLTAGE = 0x50,
} SwitchCommands;

typedef struct {
    uint8_t data[3];
} SwitchAnalog;

// left and right calibration are stored differently for some reason, so two structs
typedef struct {
    uint8_t data[9];
} SwitchLeftCalibration;

typedef struct {
    uint8_t data[9];
} SwitchRightCalibration;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} SwitchColorDefinition;

typedef struct __attribute((packed, aligned(1)))
{
    uint8_t serialNumber[16];

    uint8_t unknown00[2];

    uint8_t deviceType;

    uint8_t unknown01; // usually 0xA0

    uint8_t unknown02[7];

    uint8_t colorInfo; // 0 = default colors

    uint8_t unknown03[4];

    uint8_t motionCalibration[24];

    uint8_t unknown04[5];

    SwitchLeftCalibration leftStickCalibration;

    SwitchRightCalibration rightStickCalibration;

    uint8_t unknown08;

    SwitchColorDefinition bodyColor;

    SwitchColorDefinition buttonColor;

    SwitchColorDefinition leftGripColor;

    SwitchColorDefinition rightGripColor;

    uint8_t unknown06[37];

    uint8_t motionHorizontalOffsets[6];

    uint8_t stickParams1[17];

    uint8_t stickParams2[17];

    uint8_t unknown07[0xE57];
} SwitchFactoryConfig;

typedef struct __attribute((packed, aligned(1)))
{
    uint8_t unknown00[16];

    uint8_t leftCalibrationMagic[2];
    SwitchLeftCalibration leftCalibration;

    uint8_t rightCalibrationMagic[2];
    SwitchRightCalibration rightCalibration;

    uint8_t motionCalibrationMagic[2];
    uint8_t motionCalibration[24];
} SwitchUserCalibration;

typedef struct __attribute((packed, aligned(1)))
{
    // byte 00
    uint8_t buttonY : 1;
    uint8_t buttonX : 1;
    uint8_t buttonB : 1;
    uint8_t buttonA : 1;
    uint8_t buttonRightSR : 1;
    uint8_t buttonRightSL : 1;
    uint8_t buttonR : 1;
    uint8_t buttonZR : 1;

    // byte 01
    uint8_t buttonMinus : 1;
    uint8_t buttonPlus : 1;
    uint8_t buttonThumbR : 1;
    uint8_t buttonThumbL : 1;
    uint8_t buttonHome : 1;
    uint8_t buttonCapture : 1;
    uint8_t dummy : 1;
    uint8_t chargingGrip : 1;

    // byte 02
    uint8_t dpadDown : 1;
    uint8_t dpadUp : 1;
    uint8_t dpadRight : 1;
    uint8_t dpadLeft : 1;
    uint8_t buttonLeftSL : 1;
    uint8_t buttonLeftSR : 1;
    uint8_t buttonL : 1;
    uint8_t buttonZL : 1;
} SwitchInputReport;

typedef struct __attribute((packed, aligned(1))){
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
} ImuData;

typedef struct __attribute((packed, aligned(1))) {
    uint8_t reportID;
    uint8_t timestamp;
    uint8_t connectionInfo : 4;
    uint8_t batteryLevel : 4;
    SwitchInputReport inputs;
    SwitchAnalog leftStick;
    SwitchAnalog rightStick;
    uint8_t rumbleReport;
    ImuData imuData[3];
    uint8_t padding[15];
} SwitchProReport;

typedef enum {
    SWITCH_TYPE_LEFT_JOYCON = 0x01,
    SWITCH_TYPE_RIGHT_JOYCON,
    SWITCH_TYPE_PRO_CONTROLLER,
    SWITCH_TYPE_FAMICOM_LEFT_JOYCON = 0x07,
    SWITCH_TYPE_FAMICOM_RIGHT_JOYCON = 0x08,
    SWITCH_TYPE_NES_LEFT_JOYCON = 0x09,
    SWITCH_TYPE_NES_RIGHT_JOYCON = 0x0A,
    SWITCH_TYPE_SNES = 0x0B,
    SWITCH_TYPE_N64 = 0x0C,
} SwitchControllerType;

typedef struct {
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint8_t controllerType;
    uint8_t unknown00;
    uint8_t macAddress[6];
    uint8_t unknown01;
    uint8_t storedColors;
} SwitchDeviceInfo;

typedef struct
{
	uint16_t buttons;
	uint8_t hat;
	uint8_t lx;
	uint8_t ly;
	uint8_t rx;
	uint8_t ry;
} SwitchProOutReport;


static const uint8_t switch_pro_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x15, 0x00,        // Logical Minimum (0)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Application)

    0x85, 0x30,        //   Report ID (48)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0A,        //   Usage Maximum (0x0A)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0A,        //   Report Count (10)
    0x55, 0x00,        //   Unit Exponent (0)
    0x65, 0x00,        //   Unit (None)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x0B,        //   Usage Minimum (0x0B)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x0B, 0x01, 0x00, 0x01, 0x00,  //   Usage (0x010001)
    0xA1, 0x00,        //   Collection (Physical)
    0x0B, 0x30, 0x00, 0x01, 0x00,  //     Usage (0x010030)
    0x0B, 0x31, 0x00, 0x01, 0x00,  //     Usage (0x010031)
    0x0B, 0x32, 0x00, 0x01, 0x00,  //     Usage (0x010032)
    0x0B, 0x35, 0x00, 0x01, 0x00,  //     Usage (0x010035)
    0x15, 0x00,        //     Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,  //     Logical Maximum (65534)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x04,        //     Report Count (4)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0x0B, 0x39, 0x00, 0x01, 0x00,  //   Usage (0x010039)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x0F,        //   Usage Minimum (0x0F)
    0x29, 0x12,        //   Usage Maximum (0x12)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x34,        //   Report Count (52)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)

    0x85, 0x21,        //   Report ID (33)
    0x09, 0x01,        //   Usage (0x01)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x81,        //   Report ID (-127)
    0x09, 0x02,        //   Usage (0x02)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x01,        //   Report ID (1)
    0x09, 0x03,        //   Usage (0x03)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x91, 0x83,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x10,        //   Report ID (16)
    0x09, 0x04,        //   Usage (0x04)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x91, 0x83,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x80,        //   Report ID (-128)
    0x09, 0x05,        //   Usage (0x05)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x91, 0x83,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x82,        //   Report ID (-126)
    0x09, 0x06,        //   Usage (0x06)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x91, 0x83,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0xC0,              // End Collection
};

#endif //SWITCHPROCONTROLLERESP32S3_SWITCH_PRO_DESCRIPTORS_H