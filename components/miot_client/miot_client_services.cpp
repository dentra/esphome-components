#include "miot_client_services.h"

namespace esphome {
namespace miot_client {

namespace internal {
/* c++14 version with checks
constexpr int chr2bin(int ch) {
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  if (ch >= 'A' && ch <= 'F')
    return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}
*/
constexpr int chr2bin(int ch) {
  return ch >= '0' && ch <= '9'   ? ch - '0'
         : ch >= 'A' && ch <= 'F' ? ch - 'A' + 10
         : ch >= 'a' && ch <= 'f' ? ch - 'a' + 10
                                  : 0;
}
}  // namespace internal

/**
 * Helper to instaniate esp_bt_uuid_t with 128bit UUIDs.
 *
 * Usage:
 *   constexpr auto uuid = bt_uuid_t(0x00000100, 0x0065, 0x6C62, 0x2E74, 0x6F696D2E696D);
 *
 * Warning: use only with constexpr, do not use at runtime, it slow!
 */
esp_bt_uuid_t bt_uuid_t(const uint32_t data1, const uint16_t data2, const uint16_t data3, const uint16_t data4,
                        const uint64_t node) {
  return esp_bt_uuid_t{.len = ESP_UUID_LEN_128,
                       .uuid = {.uuid128 = {
                                    uint8_t((node & 0x0000000000FF) >> 0),
                                    uint8_t((node & 0x00000000FF00) >> 8),
                                    uint8_t((node & 0x000000FF0000) >> 16),
                                    uint8_t((node & 0x0000FF000000) >> 24),
                                    uint8_t((node & 0x00FF00000000) >> 32),
                                    uint8_t((node & 0xFF0000000000) >> 40),
                                    uint8_t((data4 & 0x00FF) >> 0),
                                    uint8_t((data4 & 0xFF00) >> 8),
                                    uint8_t((data3 & 0x00FF) >> 0),
                                    uint8_t((data3 & 0xFF00) >> 8),
                                    uint8_t((data2 & 0x00FF) >> 0),
                                    uint8_t((data2 & 0xFF00) >> 8),
                                    uint8_t((data1 & 0x000000FF) >> 0),
                                    uint8_t((data1 & 0x0000FF00) >> 8),
                                    uint8_t((data1 & 0x00FF0000) >> 16),
                                    uint8_t((data1 & 0xFF000000) >> 24),
                                }}};
}

/**
 * Helper to instaniate esp_bt_uuid_t with 128bit UUIDs.
 *
 * Usage:
 *   constexpr auto uuid = bt_uuid_t("00000100-0065-6C62-2E74-6F696D2E696D")
 *
 * Warning: use only with constexpr, do not use at runtime, it slow!
 */
esp_bt_uuid_t bt_uuid_t(const char *const uuid) {
  return esp_bt_uuid_t{.len = ESP_UUID_LEN_128,
                       .uuid = {.uuid128 = {
                                    uint8_t((internal::chr2bin(uuid[34]) << 4) | internal::chr2bin(uuid[35])),
                                    uint8_t((internal::chr2bin(uuid[32]) << 4) | internal::chr2bin(uuid[33])),
                                    uint8_t((internal::chr2bin(uuid[30]) << 4) | internal::chr2bin(uuid[31])),
                                    uint8_t((internal::chr2bin(uuid[28]) << 4) | internal::chr2bin(uuid[29])),
                                    uint8_t((internal::chr2bin(uuid[26]) << 4) | internal::chr2bin(uuid[27])),
                                    uint8_t((internal::chr2bin(uuid[24]) << 4) | internal::chr2bin(uuid[25])),
                                    uint8_t((internal::chr2bin(uuid[21]) << 4) | internal::chr2bin(uuid[22])),
                                    uint8_t((internal::chr2bin(uuid[19]) << 4) | internal::chr2bin(uuid[20])),
                                    uint8_t((internal::chr2bin(uuid[16]) << 4) | internal::chr2bin(uuid[17])),
                                    uint8_t((internal::chr2bin(uuid[14]) << 4) | internal::chr2bin(uuid[15])),
                                    uint8_t((internal::chr2bin(uuid[11]) << 4) | internal::chr2bin(uuid[12])),
                                    uint8_t((internal::chr2bin(uuid[9]) << 4) | internal::chr2bin(uuid[10])),
                                    uint8_t((internal::chr2bin(uuid[6]) << 4) | internal::chr2bin(uuid[7])),
                                    uint8_t((internal::chr2bin(uuid[4]) << 4) | internal::chr2bin(uuid[5])),
                                    uint8_t((internal::chr2bin(uuid[2]) << 4) | internal::chr2bin(uuid[3])),
                                    uint8_t((internal::chr2bin(uuid[0]) << 4) | internal::chr2bin(uuid[1])),
                                }}};
}

/**
 * Nordic UART Service (NUS)
 *
 * The Bluetooth LE GATT Nordic UART Service is a custom service that receives and writes data and serves as a
 * bridge to the UART interface.
 *
 * Service UUID
 * The 128-bit vendor-specific service UUID is 6E400001-B5A3-F393-E0A9-E50E24DCCA9E (16-bit offset: 0x0001).
 * */
esp_bt_uuid_t BLE_UUID_NUS_SERVICE = bt_uuid_t("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

/**
 * RX Characteristic (6E400002-B5A3-F393-E0A9-E50E24DCCA9E)
 *  - Write or Write Without Response
 * Write data to the RX Characteristic to send it on to the UART interface.
 */
esp_bt_uuid_t BLE_UUID_NUS_RX = bt_uuid_t("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");

/**
 * TX Characteristic (6E400003-B5A3-F393-E0A9-E50E24DCCA9E)
 *  - Notify
 * Enable notifications for the TX Characteristic to receive data from the application. The application transmits all
 * data that is received over UART as notifications.
 */
esp_bt_uuid_t BLE_UUID_NUS_TX = bt_uuid_t("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

/**
 * Mi Sevice
 * See https://github.com/MiEcosystem/mijia_ble_secure/blob/silabs/gatt.xml
 */
uint16_t BLE_UUID_MI_SERVICE = 0xFE95;

/**
 * Mi Service Legacy Auth Characteristic
 * Notify, Write No Response
 * Length=0..12
 */
uint16_t BLE_UUID_MI_LEGACY_AUTH = 0x0001;

/**
 * Mi Service PID Characteristic
 * Read
 * Length=0..2
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 */
uint16_t BLE_UUID_MI_PID = 0x0002;

/**
 * Mi Service Version Characteristic
 * Read
 * Length=20
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
uint16_t BLE_UUID_MI_VERSION = 0x0004;

/**
 * Mi Service WiFiCfg (Wifi AP SSID/Wifi Status) Characteristic
 * Notify
 * Length=20
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 */
uint16_t BLE_UUID_MI_WIFI_CFG = 0x0005;

/**
 * Mi Service Wifi AP Password Characteristic
 * Source: https://github.com/freenetwork/ble-in-xiaomi/blob/master/doc/auth_protocol.md
 */
uint16_t BLE_UUID_MI_WIFI_PWD = 0x0006;

/**
 * Mi Service Auth Control Point Characteristic
 * Notify
 * Length=4
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
uint16_t BLE_UUID_MI_CONTROL_POINT = 0x0010;

/**
 * Mi Service Wifi UID Password Characteristic
 * Source: https://github.com/freenetwork/ble-in-xiaomi/blob/master/doc/auth_protocol.md
 */
uint16_t BLE_UUID_MI_WIFI_UID = 0x0011;

/**
 * Mi Service Device ID (serial number) Characteristic
 * Read
 * Length=20
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 */
uint16_t BLE_UUID_MI_DEVICE_ID = 0x0013;

/**
 * Mi Service Beaconkey Characteristic
 * Read
 * Length=12
 */
uint16_t BLE_UUID_MI_BEACONKEY = 0x0014;

/**
 * Secure Auth Characteristic
 * Notify, Write No Response
 * Length=0..20
 * Source: https://github.com/scooterhacking/mijia_ble_libs/blob/master/mijia_profiles/mi_service_server.c#L64
 */
uint16_t BLE_UUID_MI_SECURE_AUTH = 0x0016;

/**
 * OTA Control Point Characteristic
 * Notify, Write
 * Length=0..20
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
uint16_t BLE_UUID_MI_OTA_CONTROL_POINT = 0x0017;

/**
 * OTA Data Characteristic
 * Notify, Write No Response
 * Length=0..20
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
uint16_t BLE_UUID_MI_OTA_DATA = 0x0018;

/**
 * Standard Auth Characteristic
 * Notify, Write No Response
 * Length=0..20,244?
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
uint16_t BLE_UUID_MI_STANDARD_AUTH = 0x0019;

/**
 * SPEC RX Characteristic
 * Notify, Write No Response
 * Length=244?
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
uint16_t BLE_UUID_MI_SPEC_RX = 0x001A;

/**
 * SPEC TX Characteristic
 * Notify, Write No Response
 * Length=0..244?
 * Source: https://github.com/MiEcosystem/mijia_ble_standard/blob/silabs_BG22/gatt.xml
 */
uint16_t BLE_UUID_MI_SPEC_TX = 0x001B;

/**
 * Bswifi Service
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 **/
uint16_t BLE_UUID_BSWIFI_SERVICE = 0xFF13;

/**
 * Bswifi Service Send Characteristic
 * Notify, notify_requirement="mandatory"
 * Length: 0..20
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 **/
uint16_t BLE_UUID_BSWIFI_SEND = 0xFF11;

/**
 * Bswifi Service Recv Characteristic
 * Write No Response, write_no_response_requirement="mandatory"
 * Length=0..3
 * Source:
 * https://github.com/doufan/bswifi/blob/master/i-bswifi-silicon/mijia_ble_2/demos/Silabs/mible_std_authen/gatt.xml
 **/
uint16_t BLE_UUID_BSWIFI_RECV = 0xFF12;

/**
 * Mijia lock service
 */
esp_bt_uuid_t BLE_UUID_MIJIA_LOCK_SERVICE = bt_uuid_t("00001000-0065-6c62-2e74-6f696d2e696d");

/**
 * Lock Operation
 * Write
 * Length=7
 */
esp_bt_uuid_t BLE_UUID_MIJIA_LOCK_OPS = bt_uuid_t("00001001-0065-6c62-2e74-6f696d2e696d");

/**
 * Lock State
 * Notify, Read
 * Length=7
 */
esp_bt_uuid_t BLE_UUID_MIJIA_LOCK_STAT = bt_uuid_t("00001002-0065-6c62-2e74-6f696d2e696d");

/**
 * Lock logs
 * Notify, Read
 * Length=0..20
 */
esp_bt_uuid_t BLE_UUID_MIJIA_LOCK_LOGS = bt_uuid_t("00001003-0065-6c62-2e74-6f696d2e696d");

/**
 * Mi Stdio Service (00000100-0065-6C62-2E74-6F696D2E696D)
 */
esp_bt_uuid_t BLE_UUID_MI_STDIO_SERVICE = bt_uuid_t("00000100-0065-6C62-2E74-6F696D2E696D");

/**
 * MI STDIO RX (00000101-0065-6C62-2E74-6F696D2E696D)
 *  - Write without response
 */
esp_bt_uuid_t BLE_UUID_MI_STDIO_RX = bt_uuid_t("00000101-0065-6C62-2E74-6F696D2E696D");

/**
 * MI STDIO TX (00000102-0065-6C62-2E74-6F696D2E696D)
 *  - Notify
 */
esp_bt_uuid_t BLE_UUID_MI_STDIO_TX = bt_uuid_t("00000102-0065-6C62-2E74-6F696D2E696D");

}  // namespace miot_client
}  // namespace esphome
