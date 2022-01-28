#pragma once

#include <esp_bt_defs.h>

namespace esphome {
namespace miot_client {

/**
 * Nordic UART Service (NUS)
 *
 * The Bluetooth LE GATT Nordic UART Service is a custom service that receives and writes data and serves as a
 * bridge to the UART interface.
 *
 * Service UUID
 * The 128-bit vendor-specific service UUID is 6E400001-B5A3-F393-E0A9-E50E24DCCA9E (16-bit offset: 0x0001).
 * */
extern esp_bt_uuid_t BLE_UUID_NUS_SERVICE;

/**
 * RX Characteristic (6E400002-B5A3-F393-E0A9-E50E24DCCA9E)
 *  - Write or Write Without Response
 * Write data to the RX Characteristic to send it on to the UART interface.
 */
extern esp_bt_uuid_t BLE_UUID_NUS_RX;

/**
 * TX Characteristic (6E400003-B5A3-F393-E0A9-E50E24DCCA9E)
 *  - Notify
 * Enable notifications for the TX Characteristic to receive data from the application. The application transmits all
 * data that is received over UART as notifications.
 */
extern esp_bt_uuid_t BLE_UUID_NUS_TX;

/**
 * Mi Sevice
 * See https://github.com/MiEcosystem/mijia_ble_secure/blob/silabs/gatt.xml
 */
extern uint16_t BLE_UUID_MI_SERVICE;

/**
 * Mi Service Legacy Auth Characteristic
 * Notify, Write No Response
 * Length=0..12
 */
extern uint16_t BLE_UUID_MI_LEGACY_AUTH;

/**
 * Mi Service PID Characteristic
 * Read
 * Length=0..2
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 */
extern uint16_t BLE_UUID_MI_PID;

/**
 * Mi Service Version Characteristic
 * Read
 * Length=20
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
extern uint16_t BLE_UUID_MI_VERSION;

/**
 * Mi Service WiFiCfg (Wifi AP SSID/Wifi Status) Characteristic
 * Notify
 * Length=20
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 */
extern uint16_t BLE_UUID_MI_WIFI_CFG;

/**
 * Mi Service Wifi AP Password Characteristic
 * Source: https://github.com/freenetwork/ble-in-xiaomi/blob/master/doc/auth_protocol.md
 */
extern uint16_t BLE_UUID_MI_WIFI_PWD;

/**
 * Mi Service Auth Control Point Characteristic
 * Notify
 * Length=4
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
extern uint16_t BLE_UUID_MI_CONTROL_POINT;

/**
 * Mi Service Wifi UID Password Characteristic
 * Source: https://github.com/freenetwork/ble-in-xiaomi/blob/master/doc/auth_protocol.md
 */
extern uint16_t BLE_UUID_MI_WIFI_UID;

/**
 * Mi Service Device ID (serial number) Characteristic
 * Read
 * Length=20
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 */
extern uint16_t BLE_UUID_MI_DEVICE_ID;

/**
 * Mi Service Beaconkey Characteristic
 * Read
 * Length=12
 */
extern uint16_t BLE_UUID_MI_BEACONKEY;

/**
 * Secure Auth Characteristic
 * Notify, Write No Response
 * Length=0..20
 * Source: https://github.com/scooterhacking/mijia_ble_libs/blob/master/mijia_profiles/mi_service_server.c#L64
 */
extern uint16_t BLE_UUID_MI_SECURE_AUTH;

/**
 * OTA Control Point Characteristic
 * Notify, Write
 * Length=0..20
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
extern uint16_t BLE_UUID_MI_OTA_CONTROL_POINT;

/**
 * OTA Data Characteristic
 * Notify, Write No Response
 * Length=0..20
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
extern uint16_t BLE_UUID_MI_OTA_DATA;

/**
 * Standard Auth Characteristic
 * Notify, Write No Response
 * Length=0..20,244?
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
extern uint16_t BLE_UUID_MI_STANDARD_AUTH;

/**
 * SPEC RX Characteristic
 * Notify, Write No Response
 * Length=244?
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
extern uint16_t BLE_UUID_MI_SPEC_RX;

/**
 * SPEC TX Characteristic
 * Notify, Write No Response
 * Length=0..244?
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
extern uint16_t BLE_UUID_MI_SPEC_TX;

/**
 * Bswifi Service
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 **/
extern uint16_t BLE_UUID_BSWIFI_SERVICE;

/**
 * Bswifi Service Send Characteristic
 * Notify, notify_requirement="mandatory"
 * Length: 0..20
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 **/
extern uint16_t BLE_UUID_BSWIFI_SEND;

/**
 * Bswifi Service Recv Characteristic
 * Write No Response, write_no_response_requirement="mandatory"
 * Length=0..3
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 **/
extern uint16_t BLE_UUID_BSWIFI_RECV;

/**
 * Mijia lock service
 */
extern esp_bt_uuid_t BLE_UUID_MIJIA_LOCK_SERVICE;

/**
 * Lock Operation
 * Write
 * Length=7
 */
extern esp_bt_uuid_t BLE_UUID_MIJIA_LOCK_OPS;

/**
 * Lock State
 * Notify, Read
 * Length=7
 */
extern esp_bt_uuid_t BLE_UUID_MIJIA_LOCK_STAT;

/**
 * Lock logs
 * Notify, Read
 * Length=0..20
 */
extern esp_bt_uuid_t BLE_UUID_MIJIA_LOCK_LOGS;

/**
 * Mi Stdio Service (00000100-0065-6C62-2E74-6F696D2E696D)
 */
extern esp_bt_uuid_t BLE_UUID_MI_STDIO_SERVICE;

/**
 * MI STDIO RX (00000101-0065-6C62-2E74-6F696D2E696D)
 *  - Write without response
 */
extern esp_bt_uuid_t BLE_UUID_MI_STDIO_RX;

/**
 * MI STDIO TX (00000102-0065-6C62-2E74-6F696D2E696D)
 *  - Notify
 */
extern esp_bt_uuid_t BLE_UUID_MI_STDIO_TX;

}  // namespace miot_client
}  // namespace esphome
