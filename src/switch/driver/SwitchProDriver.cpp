#include "SwitchProDriver.h"
#include <Arduino.h>
#include "driverhelper.h"
#include "debug/log.h"

SwitchProDriver::SwitchProDriver() : gen(rd()), dist(0, 0xFF) {
    // 设置实例指针
    usb_hid = Adafruit_USBD_HID(switch_pro_report_descriptor,
              sizeof(switch_pro_report_descriptor),
              HID_ITF_PROTOCOL_NONE, 2, true);
}

void SwitchProDriver::initialize() {
    TinyUSBDevice.setID(0x057E, 0x2009); // Nintendo Switch Pro Controller
    TinyUSBDevice.setDeviceVersion(0x0200);
    TinyUSBDevice.setManufacturerDescriptor("Nintendo Co., Ltd");
    TinyUSBDevice.setProductDescriptor("Switch Pro Controller");
    TinyUSBDevice.setSerialDescriptor("000000000001");

    usb_hid.setPollInterval(4);
    usb_hid.setReportDescriptor(switch_pro_report_descriptor, sizeof(switch_pro_report_descriptor));
    logPrintf("switch_pro_configuration_descriptor_size=%d\n", sizeof(switch_pro_report_descriptor));
    usb_hid.setStringDescriptor("Nintendo Switch Pro Controller");
    usb_hid.setReportCallback(nullptr, set_report_callback);
    usb_hid.begin();


    if (TinyUSBDevice.mounted()){
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
    while (!TinyUSBDevice.mounted()) {
        // logPrintf("USB mounted: %d\n", TinyUSBDevice.mounted());
        delay(100);
    }
    logPrintf("USB Mounted Successfully!\n");

    playerID = 0;
    last_report_counter = 0;
    handshakeCounter = 0;
    isReady = false;

    deviceInfo = {
        .majorVersion = 0x04,
        .minorVersion = 0x91,
        .controllerType = SwitchControllerType::SWITCH_TYPE_PRO_CONTROLLER,
        .unknown00 = 0x02,
        // MAC address in reverse
        .macAddress = {0x7c, 0xbb, 0x8a, dist(gen), dist(gen), dist(gen)},
        .unknown01 = 0x01,
        .storedColors = 0x02,
    };

    last_report_timer = millis();
    resetSwitchReport();
    is_init = true;
}

void SwitchProDriver::resetSwitchReport() {
    switchReport = {
        .reportID = 0x30,
        .timestamp = 0,
        .connectionInfo = 0xE,
        .batteryLevel = 0x8,

        .inputs {
            // byte 00
            .buttonY = 0,
            .buttonX = 0,
            .buttonB = 0,
            .buttonA = 0,
            .buttonRightSR = 0,
            .buttonRightSL = 0,
            .buttonR = 0,
            .buttonZR = 0,

            // byte 01
            .buttonMinus = 0,
            .buttonPlus = 0,
            .buttonThumbR = 0,
            .buttonThumbL = 0,
            .buttonHome = 0,
            .buttonCapture = 0,
            .dummy = 0,
            .chargingGrip = 1,

            // byte 02
            .dpadDown = 0,
            .dpadUp = 0,
            .dpadRight = 0,
            .dpadLeft = 0,
            .buttonLeftSL = 0,
            .buttonLeftSR = 0,
            .buttonL = 0,
            .buttonZL = 0,
        },
        .leftStick = {0x00, 0x08, 0x80}, // 2048, 2048
        .rightStick =  {0x00, 0x08, 0x80}, // 2048, 2048
        .rumbleReport = 0x09,
        .imuData = {{0, 0, -4096, 0, 0, 0}, {0, 0, -4096, 0, 0, 0}, {0, 0, -4096, 0, 0, 0}},
        .padding = {0x00}
    };
}

bool SwitchProDriver::process(const bool force) {
    const uint32_t now = millis();
    const uint32_t next_report_time = last_report_timer + SWITCH_PRO_KEEPALIVE_TIMER;
    // Wake up TinyUSB device
    if (tud_suspended()) {
        tud_remote_wakeup();
    }

    if (!isInitialized) {
        // send identification
        {
            std::lock_guard lock(reportMtx);
            sendIdentify();
        }
        if (tud_hid_ready() && sendReport(0, report, false) == true) {
            isInitialized = true;
            return true;
        }
    }
    // serialInput.

    if (now < next_report_time && !force) {
        return false;
    }

    if (isReportQueued) {
        // logSamplingPrintf("SwitchProDriver::sendReport:[");
        // for (int i = 0; i < 64; i++) {
        //     logSamplingPrintf("%02x,", report[i]);
        // }
        // logSamplingPrintf("]\n");
        if (tud_hid_ready() && sendReport(queuedReportID, report, true) == true) {
            isReportQueued = false;
            return true;
        }
    }

    if (isReady) {
        switchReport.timestamp = last_report_counter;
        const void * inputReport = &switchReport;
        uint16_t report_size = sizeof(switchReport);
        if (memcmp(last_report, inputReport, report_size) != 0) {
            // HID ready + report sent, copy previous report
            if (tud_hid_ready() && sendReport(0, inputReport, true) == true) {
                memcpy(last_report, inputReport, report_size);
                return true;
            }
        }
    }
    return false;
}


void SwitchProDriver::sendIdentify() {
    memset(report, 0x00, 64);
    report[0] = SwitchReportID::REPORT_USB_INPUT_81;
    report[1] = SwitchOutputSubtypes::IDENTIFY;
    report[2] = 0x00;
    report[3] = deviceInfo.controllerType;
    // MAC address
    for (uint8_t i = 0; i < 6; i++) {
        report[4+i] = deviceInfo.macAddress[5-i];
    }
}

void SwitchProDriver::sendSubCommand(uint8_t subCommand) {

}

bool SwitchProDriver::sendReport(uint8_t reportID, void const* reportData, bool addCount) {
    std::lock_guard lock(reportMtx);

    // logSamplingPrintf("SwitchProDriver::sendReport:[");
    // for (int i = 0; i < 64; i++) {
    //     logSamplingPrintf("%02x,", ((uint8_t*)reportData)[i]);
    // }
    // logSamplingPrintf("]\n");
    const bool result = tud_hid_report(reportID, reportData, SWITCH_PRO_ENDPOINT_SIZE);
    if (addCount) {
        if (last_report_counter < 255) {
            last_report_counter++;
        } else {
            last_report_counter = 0;
        }
    }
    last_report_timer = millis();
    return result;
}

void SwitchProDriver::handleConfigReport(uint8_t switchReportID, uint8_t switchReportSubID, const uint8_t *reportData, uint16_t reportLength) {
    bool canSend = false;

    switch (switchReportSubID) {
        case SwitchOutputSubtypes::IDENTIFY:
            // logSamplingPrintf("SwitchProDriver::set_report: IDENTIFY\n");
            sendIdentify();
            canSend = true;
            break;
        case SwitchOutputSubtypes::HANDSHAKE:
            // logSamplingPrintf("SwitchProDriver::set_report: HANDSHAKE\n");
            report[0] = SwitchReportID::REPORT_USB_INPUT_81;
            report[1] = SwitchOutputSubtypes::HANDSHAKE;
            canSend = true;
            break;
        case SwitchOutputSubtypes::BAUD_RATE:
            // logSamplingPrintf("SwitchProDriver::set_report: BAUD_RATE\n");
            report[0] = SwitchReportID::REPORT_USB_INPUT_81;
            report[1] = SwitchOutputSubtypes::BAUD_RATE;
            canSend = true;
            break;
        case SwitchOutputSubtypes::DISABLE_USB_TIMEOUT:
            // logSamplingPrintf("SwitchProDriver::set_report: DISABLE_USB_TIMEOUT\n");
            report[0] = SwitchReportID::REPORT_OUTPUT_30;
            report[1] = switchReportSubID;
            //if (handshakeCounter < 4) {
            //    handshakeCounter++;
            //} else {
                isReady = true;
            //}
            canSend = true;
            break;
        case SwitchOutputSubtypes::ENABLE_USB_TIMEOUT:
            // logSamplingPrintf("SwitchProDriver::set_report: ENABLE_USB_TIMEOUT\n");
            report[0] = SwitchReportID::REPORT_OUTPUT_30;
            report[1] = switchReportSubID;
            canSend = true;
            break;
        default:
            // logSamplingPrintf("SwitchProDriver::set_report: Unknown Sub ID %02x\n", switchReportSubID);
            report[0] = SwitchReportID::REPORT_OUTPUT_30;
            report[1] = switchReportSubID;
            canSend = true;
            break;
    }
    if (canSend) isReportQueued = true;
}

void SwitchProDriver::handleFeatureReport(uint8_t switchReportID, uint8_t switchReportSubID, const uint8_t *reportData, uint16_t reportLength) {
    uint8_t commandID = reportData[10];
    uint32_t spiReadAddress = 0;
    uint8_t spiReadSize = 0;
    bool canSend = false;

    report[0] = SwitchReportID::REPORT_OUTPUT_21;
    report[1] = last_report_counter;
    memcpy(report+2,&switchReport.inputs,sizeof(SwitchInputReport));
    report[12] = switchReport.rumbleReport;

    // logSamplingPrintf("logSamplingPrintf=%02x,switchReportID=%02x,switchReportSubID=%02x,commandID=%02x\n",inputMode, switchReportID,switchReportSubID,commandID);

    switch (commandID) {
        case SwitchCommands::GET_CONTROLLER_STATE:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 GET_CONTROLLER_STATE\n");
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
        case SwitchCommands::BLUETOOTH_PAIR_REQUEST:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 BLUETOOTH_PAIR_REQUEST\n");
            report[13] = 0x81;
            report[14] = commandID;
            report[15] = 0x03;
            // MAC address
            for (uint8_t i = 0; i < 6; i++) {
                report[16+i] = deviceInfo.macAddress[5-i];
            }
            canSend = true;
            break;
        case SwitchCommands::REQUEST_DEVICE_INFO:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 REQUEST_DEVICE_INFO\n");
            report[13] = 0x82;
            report[14] = 0x02;
            memcpy(&report[15], &deviceInfo, sizeof(deviceInfo));
            canSend = true;
            break;
        case SwitchCommands::SET_MODE:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 SET_MODE\n");
            inputMode = reportData[11];
            report[13] = 0x80;
            report[14] = 0x03;
            report[15] = inputMode;
            isReady = true;
            canSend = true;
            // logSamplingPrintf("Input Mode set to ");
            switch (inputMode) {
                case 0x00:
                    // logSamplingPrintf("NFC/IR Polling Data");
                    break;
                case 0x01:
                    // logSamplingPrintf("NFC/IR Polling Config");
                    break;
                case 0x02:
                    // logSamplingPrintf("NFC/IR Polling Data+Config");
                    break;
                case 0x03:
                    // logSamplingPrintf("IR Scan");
                    break;
                case 0x23:
                    // logSamplingPrintf("MCU Update");
                    break;
                case 0x30:
                    // logSamplingPrintf("Full Input");
                    break;
                case 0x31:
                    // logSamplingPrintf("NFC/IR");
                    break;
                case 0x3F:
                    // logSamplingPrintf("Simple HID");
                    break;
                case 0x33:
                case 0x35:
                default:
                    // logSamplingPrintf("Unknown");
                    break;
            }
            // logSamplingPrintf("\n");
            break;
        case SwitchCommands::TRIGGER_BUTTONS:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 TRIGGER_BUTTONS\n");
            report[13] = 0x83;
            report[14] = 0x04;
            canSend = true;
            break;
        case SwitchCommands::SET_SHIPMENT:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 SET_SHIPMENT\n");
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            // for (uint8_t i = 2; i < bufsize; i++) {
            //    printf("%02x ", reportData[i]);
            // }
            // printf("\n");
            break;
        case SwitchCommands::SPI_READ:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 SPI_READ\n");
            spiReadAddress = (reportData[14] << 24) | (reportData[13] << 16) | (reportData[12] << 8) | (reportData[11]);
            spiReadSize = reportData[15];
            // logSamplingPrintf("Read From: 0x%08x Size %d\n", spiReadAddress, spiReadSize);
            report[13] = 0x90;
            report[14] = reportData[10];
            report[15] = reportData[11];
            report[16] = reportData[12];
            report[17] = reportData[13];
            report[18] = reportData[14];
            report[19] = reportData[15];
            readSPIFlash(&report[20], spiReadAddress, spiReadSize);
            canSend = true;
            // logSamplingPrintf("----------------------------------------------\n");
            break;
        case SwitchCommands::SET_NFC_IR_CONFIG:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 SET_NFC_IR_CONFIG\n");
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::SET_NFC_IR_STATE:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 SET_NFC_IR_STATE\n");
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::SET_PLAYER_LIGHTS:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 SET_PLAYER_LIGHTS\n");
            playerID = reportData[11];
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            // logSamplingPrintf("Player set to %d\n", playerID);
            // logSamplingPrintf("----------------------------------------------\n");
            break;
        case SwitchCommands::GET_PLAYER_LIGHTS:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 GET_PLAYER_LIGHTS\n");
            playerID = reportData[11];
            report[13] = 0xB0;
            report[14] = commandID;
            report[15] = playerID;
            canSend = true;
            // logSamplingPrintf("Player is %d\n", playerID);
            // logSamplingPrintf("----------------------------------------------\n");
            break;
        case SwitchCommands::COMMAND_UNKNOWN_33:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 COMMAND_UNKNOWN_33\n");
            // Command typically thrown by Chromium to detect if a Switch controller exists. Can ignore.
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
        case SwitchCommands::SET_HOME_LIGHT:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 SET_HOME_LIGHT\n");
            // NYI
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            break;
        case SwitchCommands::TOGGLE_IMU:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 TOGGLE_IMU\n");
            isIMUEnabled = reportData[11];
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            // logSamplingPrintf("IMU set to %d\n", isIMUEnabled);
            // logSamplingPrintf("----------------------------------------------\n");
            break;
        case SwitchCommands::IMU_SENSITIVITY:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 IMU_SENSITIVITY\n");
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::ENABLE_VIBRATION:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 ENABLE_VIBRATION\n");
            isVibrationEnabled = reportData[11];
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            // logSamplingPrintf("Vibration set to %d\n", isVibrationEnabled);
            // logSamplingPrintf("----------------------------------------------\n");
            break;
        case SwitchCommands::READ_IMU:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 READ_IMU\n");
            report[13] = 0xC0;
            report[14] = commandID;
            report[15] = reportData[11];
            report[16] = reportData[12];
            canSend = true;
            // logSamplingPrintf("IMU Addr: %02x, Size: %02x\n", reportData[11], reportData[12]);
            // logSamplingPrintf("----------------------------------------------\n");
            break;
        case SwitchCommands::GET_VOLTAGE:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 GET_VOLTAGE\n");
            report[13] = 0xD0;
            report[14] = 0x50;
            report[15] = 0x83;
            report[16] = 0x06;
            canSend = true;
            break;
        default:
            // logSamplingPrintf("SwitchProDriver::set_report: Rpt 0x01 Unknown 0x%02x\n", commandID);
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
    }

    if (canSend) isReportQueued = true;
}

void SwitchProDriver::set_report(uint8_t report_id, hid_report_type_t report_type, const uint8_t *buffer, uint16_t bufsize) {
    if (report_type != HID_REPORT_TYPE_OUTPUT) return;

    std::lock_guard lock(reportMtx);

    memset(report, 0x00, sizeof(report));

    uint8_t switchReportID = buffer[0];
    uint8_t switchReportSubID = buffer[1];
    // printf("SwitchProDriver::set_report Rpt: %02x, Type: %d, Len: %d :: SID: %02x, SSID: %02x\n", report_id, report_type, bufsize, switchReportID, switchReportSubID);
    if (switchReportID == SwitchReportID::REPORT_OUTPUT_00) {
    } else if (switchReportID == SwitchReportID::REPORT_FEATURE) {
        queuedReportID = report_id;
        handleFeatureReport(switchReportID, switchReportSubID, buffer, bufsize);
    } else if (switchReportID == SwitchReportID::REPORT_CONFIGURATION) {
        queuedReportID = report_id;
        handleConfigReport(switchReportID, switchReportSubID, buffer, bufsize);
    } else {
        // logSamplingPrintf("SwitchProDriver::set_report Rpt: %02x, Type: %d, Len: %d :: SID: %02x, SSID: %02x\n", report_id, report_type, bufsize, switchReportID, switchReportSubID);
    }
}

void SwitchProDriver::set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    getInstance().set_report(report_id, report_type, buffer, bufsize);
}

void SwitchProDriver::readSPIFlash(uint8_t* dest, uint32_t address, uint8_t size) {
    uint32_t addressBank = address & 0xFFFFFF00;
    uint32_t addressOffset = address & 0x000000FF;
    // logSamplingPrintf("Address: %08x, Bank: %04x, Offset: %04x, Size: %d\n", address, addressBank, addressOffset, size);

    if (const auto it = spiFlashData.find(addressBank); it != spiFlashData.end()) {
        // address found
        const uint8_t* data = it->second;
        memcpy(dest, data+addressOffset, size);
        //for (uint8_t i = 0; i < size; i++) printf("%02x ", dest[i]);
        // logSamplingPrintf("\n---\n");
    } else {
        // could not find defined address
        // logSamplingPrintf("Not Found\n");
        memset(dest, 0xFF, size);
    }
}

bool SwitchProDriver::updateInputReport(SwitchProSerialInput* serialInput) {
    bool update = false;
    if (memcmp(&switchReport.inputs, &serialInput->inputs, sizeof(SwitchInputReport)) != 0) {
        memcpy(&switchReport.inputs, &serialInput->inputs, sizeof(SwitchInputReport));
        update = true;
    }
    if (memcmp(&switchReport.leftStick, &serialInput->leftStick, sizeof(SwitchAnalog)) != 0) {
        memcpy(&switchReport.leftStick, &serialInput->leftStick, sizeof(SwitchAnalog));
        update = true;
    }
    if (memcmp(&switchReport.rightStick, &serialInput->rightStick, sizeof(SwitchAnalog)) != 0) {
        memcpy(&switchReport.rightStick, &serialInput->rightStick, sizeof(SwitchAnalog));
        update = true;
    }
    if (memcmp(&switchReport.imuData, &serialInput->imuData, sizeof(ImuData) * 3) != 0) {
        memcpy(&switchReport.imuData, &serialInput->imuData, sizeof(ImuData) * 3);
        update = true;
    }
    return update;
}