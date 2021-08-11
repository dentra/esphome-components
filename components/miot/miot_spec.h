#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include <functional>
#include "esphome/core/optional.h"

namespace esphome {
namespace miot {

// https://iot.mi.com/new/doc/embedded-development/ble/object-definition
enum Attribute : uint16_t {
  // Uninitialized.
  ATTR_UNINITIALIZED = 0x0000,

  /// Passing case.

  // Connection.
  ATTR_PC_CONNECTION = 0x0001,
  // Simple pairing.
  ATTR_PC_SIMPLE_PAIRING = 0x0002,
  // Close to.
  ATTR_PC_CLOSE_TO = 0x0003,
  // Away from.
  ATTR_PC_AWAY_FROM = 0x0004,
  // Lock (obstolete).
  ATTR_PC_LOCK_OBSOLETE = 0x0005,
  // Fingerprint.
  ATTR_PC_FINGERPRINT = 0x0006,
  // Door.
  ATTR_PC_DOOR = 0x0007,
  // Armed.
  ATTR_PC_ARMED = 0x0008,
  // Gesture controller.
  ATTR_PC_GESTURE_CONTROLLER = 0x0009,
  // Body temperature.
  ATTR_PC_BODY_TEMPERATURE = 0x000A,
  // Lock.
  ATTR_PC_LOCK = 0x000B,
  // Flooding.
  ATTR_PC_FLOODING = 0x000C,
  // Smoke.
  ATTR_PC_SMOKE = 0x000D,
  // Gas.
  ATTR_PC_GAS = 0x000E,
  // Someone is moving (with light).
  // Light intensity. (3 bytes).
  // The unit of light intensity is Lux, the value range is 0-120000
  ATTR_PC_SOMEONE_IS_MOVING_WITH_LIGHT = 0x000F,
  // Toothbrush event.
  ATTR_PC_TOOTHBRUSH_EVENT = 0x0010,
  // Cat's eye event.
  ATTR_PC_CATS_EYE_EVENT = 0x0011,
  // Weighing event.
  ATTR_PC_WEIGHING_EVENT = 0x0012,
  // Button.
  ATTR_PC_BUTTON = 0x1001,

  /// General attributes.

  // Sleep.
  ATTR_SLEEP = 0x1002,
  // RSSI.
  // motion detection, 1 byte, 8-bit unsigned integer
  ATTR_MOTION = 0x1003,
  // Temperature.
  // 2 bytes, 16-bit signed integer (LE), 0.1 °C
  // Signed variable, the unit is 0.1 degrees, example: 1A 01 = 0x011A means 28.2 degrees.
  ATTR_TEMPERATURE = 0x1004,
  // Unknown attribute 0x1005.
  ATTR_UNKNOWN_1005 = 0x1005,
  // Humidity. 2 bytes, 16-bit signed integer (LE), 0.1 %
  // Humidity percentage, ranging from 0-1000, for example, 346 means humidity 34.6%.
  ATTR_HUMIDITY = 0x1006,
  // Light illuminance.
  // Lux (3 bytes): Range: 0-120000, lx
  ATTR_LIGHT_ILLUMINANCE = 0x1007,
  // soil moisture, 1 byte, 8-bit unsigned integer, 1 %
  // percentage, range: 0-100, %
  ATTR_SOIL_MOISTURE = 0x1008,
  // Conductivity, 2 bytes
  // Unit µS/cm, range: 0-5000
  ATTR_CONDUCTIVITY = 0x1009,
  // Battery.
  // Battery percentage (1 byte): Range: 0-100, %
  ATTR_BATTERY = 0x100A,
  // Unknown attribute 0x100B.
  ATTR_UNKNOWN_100B = 0x100B,
  // Unknown attribute 0x100B.
  ATTR_UNKNOWN_100C = 0x100C,
  // temperature + humidity,
  // 4 bytes, 16-bit signed integer (LE) each, 0.1 °C, 0.1 %
  ATTR_TEMPERATURE_HUMIDITY = 0x100D,
  // Lock.
  // State (1 byte):
  //   bit 0: square tongue state (1: eject; 0: retract)
  //   bit 1: dull state (1: eject; 0: retract)
  //   bit 2: oblique tongue state (1: eject; 0: retract)
  //   bit 3: Child lock status (1: open; 0: close)
  // All normal combination states:
  //   0x00: unlock state (all bolts retracted)
  //   0x04: The lock tongue pops out (the oblique tongue pops out)
  //   0x05: Lock + lock tongue eject (square tongue, oblique tongue eject)
  //   0x06: Reverse lock + tongue ejection (stay tongue, oblique tongue ejection)
  //   0x07: All lock tongues pop out (square tongue, dull tongue, oblique tongue pop out)
  ATTR_LOCK = 0x100E,
  // Door.
  // Attributes:
  //   State. 1 byte.
  //     0x00: door open
  //     0x01: door closed
  //     0xFF: abnormal
  ATTR_DOOR = 0x100F,
  // Formaldehyde.
  // Formaldehyde value (2 bytes): The accuracy is 0.01mg/m3, example: 10 00 = 0x0010 means 0.16mg/m3
  ATTR_FORMALDEHYDE = 0x1010,
  // Binding state.
  // State (1 byte):
  //   0x00: unbound
  //   0x01: bound
  ATTR_BIND = 0x1011,
  // Switch.
  // State (1 byte):
  //   0x00: OFF
  //   0x01: ON
  ATTR_SWITCH = 0x1012,
  // Remaining amount of consumables.
  // Remaining percentage (1 byte): range 0~100, %
  ATTR_CONSUMABLE = 0x1013,
  // Flooding.
  // Flooding state:
  //   0x00: no flooding
  //   0x01: Flooding
  ATTR_FLOODING = 0x1014,
  // Smoke.
  // Smoke state (1 byte):
  //   0x00: Normal monitoring,
  //   0x01: fire alarm,
  //   0x02: equipment failure
  ATTR_SMOKE = 0x015,
  // Gas.
  // Gas status	(1 byte):
  //   0x00: no leakage
  //   0x01: With leakage
  ATTR_GAS = 0x1016,
  // No one moves.
  // idle time since last motion, 4 bytes.
  // The duration of the unmanned state, in seconds.
  ATTR_IDLE_TIME = 0x1017,
  // Light intensity (1 byte).
  // Attributes:
  //   Light intensity. 1 byte.
  //     0x00: weak light
  //     0x01: strong light
  // Note: If the sub-device can report accurate illuminance, please use the illuminance attribute (0x1007) to report.
  ATTR_LIGHT_INTENSITY = 0x1018,
  // Door sensor.
  //
  // Properties:
  //   Door sensor status. 1 byte.
  //     0x00: open the door
  //     0x01: close the door
  //     0x02: not closed over time
  //     0x03: device reset
  ATTR_DOOR_SENSOR = 0x1019,
  // Weight attributes.
  // Current weight value (2 bytes): Unit is grams.
  ATTR_WEIGHT = 0x101A,
  // No one moves over time.
  // State (1 byte):
  //   0x00: means someone is moving
  //   0x01: means no one is moving in X seconds
  // Note: The user configures X seconds for unmanned movement on the plug-in
  //       side, and the firmware side stores this set value, and reports
  //       this object when the time expires.
  ATTR_NO_MOVES = 0x101B,
  // Smart pillow.
  // The current state of the smart pillow:
  //   0x00: "Get out of bed" (not on the pillow);
  //   0x01: "On the bed" (lie on the pillow);
  //   0x02-0xFF: reserved
  ATTR_SMART_PILLOW = 0x101C,
  // Formaldehyde (new).
  // The accuracy is 0.001mg/m3, example: 10 00 = 0x0010 means 0.016mg/m3
  ATTR_FORMALDEHYDE_NEW = 0x101D,

  /// Vendor custom attributes.

  // Body temperature (measured every second).
  ATTR_VCA_BODY_TEMPERATURE = 0x2000,
  // Mi Band (Huami).
  ATTR_VCA_MI_BAND_HUAMI = 0x2001,
  // Vacuum cleaner (Rui Mi).
  ATTR_VCA_VACUUM_CLEANER_RUI_MI = 0x2002,
  // Black plus bracelet (like one).
  ATTR_VCA_BLACK_PLUS_BRACELET = 0x2003,

  /// Vendor-defined event.

  // Monitor (flowers and grasses).
  ATTR_VDE_MONITOR = 0x3000,
  // Sensor location (Qingping).
  ATTR_VDE_SENSOR_LOCATION_QINGPING = 0x3001,
  // Pomodoro event (Qingping).
  ATTR_VDE_POMODORO_EVENT_QINGPING = 0x3002,
  // Xiaobei toothbrush incident (Qinghe Xiaobei).
  ATTR_VDE_XIAOBEI_TOOTHBRUSH_INCIDENT_QINGHE_XIAOBEI = 0x3003
};

// Frame Control.
struct FrameControl {
  // Bit 0. Reserved.
  bool reserved0 : 1;
  // Bit 1. Reserved.
  bool reserved1 : 1;
  // Bit 2. Reserved.
  bool reserved2 : 1;
  // Bit 3. isEncrypted.
  // 0: The package is not encrypted;
  // 1: The package is encrypted
  bool is_encrypted : 1;
  // Bit 4. MAC Include.
  // 0: does not include the MAC address;
  // 1: includes a fixed MAC address (the MAC address is included for iOS to recognize this device and connect)
  bool mac_include : 1;
  // Bit 5. Capability Include
  // 0: does not include Capability;
  // 1: includes Capability. Before the device is bound, this bit is forced to 1
  bool capability_include : 1;
  // Bit 6. Object Include.
  // 0: does not include Object;
  // 1: includes Object
  bool object_include : 1;
  // Bit 7. Mesh.
  // 0: does not include Mesh;
  // 1: includes Mesh. For standard BLE access products and high security level access, this item is
  //    mandatory to 0. This item is mandatory for Mesh access to 1. For more information about Mesh
  //    access, please refer to Mesh related documents
  bool mesh : 1;
  // Bit 8. registered.
  // 0: The device is not bound;
  // 1: The device is registered and bound. This item is used to indicate whether the device has been reset
  bool registered : 1;
  // Bit 9. solicited.
  // 0: No operation;
  // 1: Request APP to register and bind. It is only valid when the user confirms the pairing by selecting the
  //    device on the developer platform, otherwise it is set to 0. The original name of this item was bindingCfm,
  //    and it was renamed to solicited "actively request, solicit" APP for registration and binding
  bool solicited : 1;
  // Bit 10-11. Auth Mode.
  // 0: old version certification;
  // 1: safety certification;
  // 2: standard certification;
  // 3: reserved
  uint8_t auth_mode : 2;
  // Bit 12-15. Version number (currently v5).
  uint8_t version : 4;
} PACKED;

// Capability field definition.
struct Capability {
  // Connectable is temporarily not used.
  bool connectable : 1;
  // Centralable temporarily not used.
  bool centralable : 1;
  // Encryptable is temporarily not used.
  bool encryptable : 1;
  // BondAbility.
  //   0: no binding,
  //   1: front binding,
  //   2: rear binding,
  //   3: Combo
  bool bond_ability : 2;
  // I/O.
  //   1: including I/O Capability field
  bool io : 1;
  // Reserved.
  uint8_t reserved : 2;
} PACKED;

struct IOCapability {
  // Bit 0. The device can enter 6 digits.
  bool can_enter_digits : 1;
  // Bit 1. The device can enter 6 letters.
  bool can_enter_letters : 1;
  // Bit 2. The device can read NFC tags.
  bool can_read_nfc_tags : 1;
  // Bit 3. The device can recognize QR Code.
  bool can_recognize_qr_code : 1;
  // Bit 4. The device can output 6 digits.
  bool can_output_digits : 1;
  // Bit 5. The device can output 6 letters.
  bool can_output_letters : 1;
  // Bit 6. The device can generate NFC tag.
  bool can_generate_nfc_tag : 1;
  // Bit 7. The device can generate QR Code.
  bool can_generate_qr_code : 1;
  // Reserved.
  uint8_t reserved;
} PACKED;

// https://iot.mi.com/new/doc/embedded-development/ble/ble-mibeacon
//
// Structure of MiBeacon.
//
// MiBeaconHeader
//
// Device Mac address.
// esp_bd_addr_t mac_address;
//
// Equipment capacity.
// Capability capability;
//
// I/O capability. This field is currently only used for high-security BLE access, and only MiBeacon v5
// is supported. It is only used before binding; when the binding is completed and an event is reported
// (such as door opening, door closing), this field is no longer needed.
// IOCapability io_capability;
//
// Trigger events or broadcast properties, see Mijia BLE Object protocol for details
// BLEObject object;
//
// If encrypted, it is a required field, combined with Frame Counter to become a 4-byte Counter for anti-replay.
// mebeacon_rnd_t random_number;
//
// If encrypted, it is a required field, MIC four bytes.
// mebeacon_mic_t message_integrity_check;
struct RawMiBeaconHeader {
  // Control bits, see the specific definition in the table below
  FrameControl frame_control;
  // Product ID, each type of product is unique, pid needs to be applied for on the Xiaomi IoT developer platform
  uint16_t product_id;
  // Serial number, used for de-duplication, different event or attribute reporting requires different Frame Counter
  uint8_t frame_counter;
} PACKED;

struct RawBLEObject {
  Attribute id;
  uint8_t data_len;
  uint8_t data[13];
} PACKED;

}  // namespace miot
}  // namespace esphome
#endif
