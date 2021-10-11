#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include <functional>
#include "esphome/core/optional.h"

namespace esphome {
namespace miot {

// https://iot.mi.com/new/doc/embedded-development/ble/object-definition
// ID definition.
// Defined by the Mi family, the distribution is as follows:
//   * 0x0000 - 0x1001 for Bluetooth Universal Event
//   * 0x1002 - 0x1FFF is a Bluetooth universal property
//   * 0x2000 - 0x2FFF customizes attributes for the manufacturer, and the corresponding manufacturer applies to Xiaomi.
//   * 0x3000 - 0x3FFF Customize events for manufacturers, and the corresponding manufacturers apply to Xiaomi.
// In order to ensure that the data is timely and effective, and does not cause greater load pressure to the Mijia
// server, the Mijia BLE gateway will filter different types of data according to the ID. When the data is separated
// from the previously reported data for a certain time and exceeds a certain amount of changes, the BLE gateway will
// forward the data to the Mijia server. Therefore, the time interval and change amount are specifically defined, as
// shown in the following table.
// In order to ensure that the events and attributes represented by Object can be received
// by the Mijia BLE gateway, MiBeacon containing Object needs to be resent multiple times. Specific reference to the
// Mijia BLE MiBeacon protocol.
enum MIID : uint16_t {
  // Uninitialized.
  MIID_UNINITIALIZED = 0xFFFF,

  /// General events.

  // Connection event.
  MIID_CONNECTION_EVENT = 0x0001,

  // Simple pairing event.
  //
  // uint16_t object_id - Object ID to be paired, such as key event (0x1001)
  MIID_SIMPLE_PAIRING_EVENT = 0x0002,

  // Motion/Close to event.
  //
  //! uint8_t has_motion
  //!
  //! Has motion:
  //!   0x00 - no motion
  //!   0x01 - has motion
  MIID_MOTION_EVENT = 0x0003,

  // Away from/Stay away from event.
  MIID_AWAY_FROM_EVENT = 0x0004,

  // Lock (obstolete) event.
  MIID_LOCK_OBSOLETE_EVENT = 0x0005,

  // Fingerprint event.
  //
  // uint32_t key_id;
  // uint8_t matching_results;
  //
  // Key IDs are divided into the following categories:
  //   0x00000000: Administrator of Lock
  //   0xFFFFFFFF: Unknown Operator
  // The matching results are divided into the following categories:
  //   0x00: Successful match
  //   0x01: Match failed
  //   0x02: Timeout was not put in
  //   0x03: Low quality (too shallow, blurry)
  //   0x04: Insufficient area
  //   0x05: Too dry skin
  //   0x06: The skin is too wet
  MIID_FINGERPRINT_EVENT = 0x0006,

  // Door event.
  //
  // uint8_t status;
  //
  // Status is divided into the following categories:
  //   0x00: Open the door
  //   0x01: Close the door
  //   0x02: Timeout is not turned off
  //   0x03: Knock on the door
  //   0x04: Pry the door
  //   0x05: The door is stuck
  MIID_DOOR_EVENT = 0x0007,

  // Armed event.
  //
  // uint8_t whether_to_enable;
  // time_t timestamp; // optional
  //
  // Whether to enable:
  //   0x00: Off
  //   0x01: On
  // Timestamp: UTC time (optional).
  MIID_ARMED_EVENT = 0x0008,

  // Gesture controller event.
  //
  // uint16_t gesture;
  //
  // Gestures are divided into the following categories:
  //   0x0001: Shake it
  //   0x0002: Flip 90 degrees
  //   0x0003: Flip 180 degrees
  //   0x0004: Planar rotation
  //   0x0005: Percussion
  //   0x0006: Push gently
  MIID_GESTURE_CONTROLLER_EVENT = 0x0009,

  // Body temperature event.
  //
  // int16_t body_temperature;
  //
  // Body temperature	: unit 0.01 degrees, example: A7 0E = 0x0EA7 = 3751 represents 37.51 degrees
  MIID_BODY_TEMPERATURE_EVENT = 0x000A,

  // Lock event.
  //
  // uint8_t operation;
  // uint32_t key_orexecption_id;
  // time_t operation_timestamp;
  //
  // The lower 4 bits of the action field represent action, which is divided into the following categories:
  //   0000b: unlock the door
  //   0001b: Lock (If it is impossible to distinguish between locks inside or outside the door, report using this type)
  //   0010b: Turn on the anti-lock
  //   0011b: Unlock
  //   0100b: unlock the door
  //   0101b: Lock the door
  //   0110b: Turn on the child lock
  //   0111b: Turn off the child lock
  //   1000b: Lock outside the door
  //   1111b: Anomaly
  // The height of the operation field is 4 bits to represent the method, which is divided into the following
  // categories:
  //   0000b: Bluetooth mode
  //   0001b: Password method
  //   0010b: Biological characteristics (fingerprints, faces, human veins, palm prints, etc.)
  //   0011b: Key method
  //   0100b: Wheel mode
  //   0101b: NFC mode
  //   0110b: One-time password
  //   0111b: Two-factor verification
  //   1001b: Homekit
  //   1000b: Mode of coercion
  //   1010b: manual
  //   1011b: Automatic
  //   1111b: Anomaly
  // Key IDs are divided into the following categories:
  //   0x00000000: Administrator of Lock
  //   0xFFFFFFFF: Unknown Operator
  //   0xDEADBEEF: Invalid Operator
  //   0x00000000 - 0x7FFFFFFFFF: Bluetooth (up to 2147483647)
  //   0x80010000 - 0x8001FFFF: Biometrics-Fingers (up to 65,536)
  //   0x80020000 - 0x8002FFFF: Passwords (up to 65,536)
  //   0x80030000 - 0x8003FFFF: keys (up to 65,536)
  //   0x80040000 - 0x8004FFFF: NFC (up to 65,536)
  //   0x80050000 - 0x8005FFFF: Two-factor verification (up to 65,536)
  //   0x80060000 - 0x8006FFFF: Biometrics - Faces (up to 65,536)
  //   0x80070000 - 0x8007FFFF: Biofeatures - Finger veins (up to 65,536)
  //   0x80080000 - 0x8008FFFF: Biotacter - Palmprints (up to 65,536)
  // Represents an exception with the ID starting with 0xC0DE. Externally triggered exceptions include:
  //   0xC0DE0000: Frequent unlocking of wrong passwords
  //   0xC0DE0001: Frequent unlocking of wrong fingerprints
  //   0xC0DE0002: Operation timeout (password input timeout)
  //   0xC0DE0003: Pry the lock
  //   0xC0DE0004: Reset the key and press
  //   0xC0DE0005: Frequent unlocking of wrong keys
  //   0xC0DE0006: Keyhole foreign body
  //   0xC0DE0007: Key not removed
  //   0xC0DE0008: False NFC Frequently Unlocked
  //   0xC0DE0009: Timeout is not locked as required
  //   0xC0DE000A: Multiple ways to unlock frequently fail
  //   0xC0DE000B: Face Frequently Unlocking Fails
  //   0xC0DE000C: Frequent opening of veins fails
  //   0xC0DE000D: hijack alarm
  //   0xC0DE000E: Cloth anti-back door inner unlocking
  //   0xC0DE000F: Frequent unlocking failure of palm prints
  //   0xC0DE0010: The safe is moved
  // Internally triggered exceptions include:
  //   0xC0DE1000: Power is less than 10%
  //   0xC0DE1001: Power is less than 5%
  //   0xC0DE1002: Fingerprint sensor anomaly
  //   0xC0DE1003: Low battery power of accessories
  //   0xC0DE1004: Mechanical failure
  //   0xC0DE1005: Lock sensor failure
  MIID_LOCK_EVENT = 0x000B,

  // Flooding event.
  //
  // uint8_t flooding;
  //
  // Flooding incident:
  //   0x00: flood alarm release
  //   0x01: water flood alarm
  MIID_FLOODING_EVENT = 0x000C,

  // Smoke event.
  //
  // uint8_t smoke;
  //
  // Smoke event:
  //   0x00: normal monitoring
  //   0x01: fire alarm
  //   0x02: equipment failure
  //   0x03: equipment self-examination
  //   0x04: analog alarm
  MIID_SMOKE_EVENT = 0x000D,

  // Gas event.
  //
  // uint8_t gas;
  //
  // Gas event:
  //   0x00: normal monitoring
  //   0x01: gas leakage alarm
  //   0x02: equipment failure
  //   0x03: sensor life expiry
  //   0x04: sensor preheating
  //   0x05: equipment self-test
  //   0x06: analog alarm
  MIID_GAS_EVENT = 0x000E,

  // Someone is moving (with light) event.
  //
  // uint8_t light_intensity[3]; // read via get_uint24
  //
  // The unit of light intensity is Lux, the value range is 0-120000.
  //
  // Note: This event is only for human sensors with lighting sensors at the same
  //       time, and individual human sensors can use MIID_MOTION_EVENT
  MIID_MOTION_WITH_LIGHT_EVENT = 0x000F,

  // Toothbrush event.
  //
  // uint8_t type;
  // uint8_t score; // optional
  //
  // Type:
  //   0: the beginning of brushing teeth
  //   1: the end of brushing teeth
  // Score (optional):
  //   This parameter can be added to the end of brushing: this brushing score, 0~100
  MIID_TOOTHBRUSH_EVENT = 0x0010,

  // Cat's eye event.
  //
  // uint8_t type;
  //
  // Event type	1
  //   0: someone stays
  //   1: someone passes by
  //   2: someone rings the bell
  //   3: someone leaves a message
  //   4: destroys equipment
  //   5: coercive alarm
  //   6: abnormal unlock
  MIID_CATS_EYE_EVENT = 0x0011,

  // Weighing event.
  //
  // uint16_t value;
  // uint8_t type;
  //
  // Weighing value: Units are grams
  // Weighing type:
  //  0: Current weight
  //  1: Reduce weight
  //  2: Increase weight
  MIID_WEIGHING_EVENT = 0x0012,

  // Button event.
  //
  // uint16_t index;
  // uint8_t type;
  //
  // Index: Button number, value range 0~9
  // Type:
  //   0x00: click
  //   0x01: double-click
  //   0x02: long press
  //   0x03: triple click / knob rotation
  //   0x04: click the key to release
  MIID_BUTTON_EVENT = 0x1001,

  /// General attributes.

  // Sleep.
  //
  // uint8_t state;
  //
  // State:
  //   0x00: no sleep
  //   0x01: falling asleep
  MIID_SLEEP = 0x1002,

  // RSSI.
  //
  // uint8_t rssi;
  //
  // rssi: Signal strength value.
  MIID_RSSI = 0x1003,

  // Temperature.
  //
  // int16_t temperature;
  //
  // Signed variable, the unit is 0.1 degrees, example: 1A 01 = 0x011A = 282 means 28.2 degrees.
  MIID_TEMPERATURE = 0x1004,

  //! Water boil.
  //!
  //! uint8_t power;
  //! uint8_t temperature;
  MIID_WATER_BOIL = 0x1005,

  // Humidity.
  //
  // uint16_t humidity;
  //
  // Humidity percentage, taking the value 0-1000, for example, 346 means humidity 34.6%.
  MIID_HUMIDITY = 0x1006,

  // Light illuminance.
  //
  // uint8_t lux[3]; // read via get_uint24
  //
  // Lux: Range: 0-120000, lx
  MIID_ILLUMINANCE = 0x1007,

  // Soil moisture.
  //
  // uint8_t humidity;
  //
  // Humidity percentage, range: 0-100, %
  MIID_SOIL_MOISTURE = 0x1008,

  // Conductivity (Soil EC).
  //
  // uint16_t value;
  //
  // Unit µS/cm, range: 0-5000
  MIID_CONDUCTIVITY = 0x1009,

  // Battery.
  //
  // uint8_t level;
  //
  // Battery percentage: Range: 0-100, %
  MIID_BATTERY = 0x100A,

  //! Temperature and humidity.
  //!
  //! int16_t temperture;
  //! uint16_t humidity;
  //!
  //! See MIID_TEMPERATURE and MIID_HUMIDITY for details.
  MIID_TEMPERATURE_HUMIDITY = 0x100D,

  // Lock.
  //
  // uint8_t state;
  //
  // State:
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
  MIID_LOCK = 0x100E,

  // Door.
  //
  // uint8_t state;
  //
  // State:
  //   0x00: door open
  //   0x01: door closed
  //   0xFF: abnormal
  MIID_DOOR = 0x100F,

  // Formaldehyde.
  //
  // uint16_t value;
  //
  // The accuracy is 0.01 mg/m3, example: 10 00 = 0x0010 = 16 means 0.16 mg/m3
  MIID_FORMALDEHYDE = 0x1010,

  // Binding state.
  //
  // uint16_t state;
  //
  // State:
  //   0x00: unbound
  //   0x01: bound
  MIID_BIND = 0x1011,

  // Switch.
  //
  // uint16_t switch;
  //
  // Switch:
  //   0x00: OFF
  //   0x01: ON
  MIID_SWITCH = 0x1012,

  // Remaining amount of consumables.
  //
  // uint16_t value;
  //
  // Remaining percentage: range 0~100, %
  MIID_CONSUMABLE = 0x1013,

  // Flooding.
  //
  // uint16_t state;
  //
  // State:
  //   0x00: no flooding
  //   0x01: Flooding
  MIID_FLOODING = 0x1014,

  // Smoke.
  //
  // uint16_t state;
  //
  // State:
  //   0x00: Normal monitoring,
  //   0x01: fire alarm,
  //   0x02: equipment failure
  MIID_SMOKE = 0x015,

  // Gas.
  //
  // uint16_t state;
  //
  // State:
  //   0x00: no leakage
  //   0x01: with leakage
  MIID_GAS = 0x1016,

  // No one moves.
  //
  // uint32_t duration;
  //
  // The duration of the idle time, in seconds.
  //
  // Note: If someone moves, reportd the unmoved property with 0 time.
  MIID_IDLE_TIME = 0x1017,

  // Light intensity.
  //
  // uint8_t light_intensity;
  //
  // Light intensity.
  //   0x00: weak light
  //   0x01: strong light
  //
  // Note: If the sub-device can report accurate illuminance, please use the illuminance attribute (0x1007) to report.
  MIID_LIGHT_INTENSITY = 0x1018,

  // Door sensor.
  //
  // uint8_t state;
  //
  // Door sensor status.
  //   0x00: open the door
  //   0x01: close the door
  //   0x02: not closed over time (timeout and not close)
  //   0x03: device reset
  MIID_DOOR_SENSOR = 0x1019,

  // Weight attributes.
  //
  // uint16_t value;
  //
  // Current weight value (2 bytes): Unit is grams.
  MIID_WEIGHT = 0x101A,

  // Timeout unmoved (No one moves over time).
  //
  // uint8_t timeout;
  //
  // Timeout:
  //   0x00: means someone is moving
  //   0x01: means no one is moving in X seconds
  // Note: The user configures X seconds for unmanned movement on the plug-in
  //       side, and the firmware side stores this set value, and reports
  //       this object when the time expires.
  MIID_TIMEOUT = 0x101B,

  // Smart pillow.
  //
  // uint8_t status;
  //
  // The current state of the smart pillow:
  //   0x00: "Get out of bed" (not on the pillow);
  //   0x01: "On the bed" (lie on the pillow);
  //   0x02-0xFF: reserved
  MIID_SMART_PILLOW = 0x101C,

  // Formaldehyde (new).
  //
  // uint16_t value;
  //
  // The accuracy is 0.001 mg/m3, example: 10 00 = 0x0010 = 16 means 0.016 mg/m3
  MIID_FORMALDEHYDE_NEW = 0x101D,

  /// Vendor custom attributes.

  // Body temperature (measured every second).
  //
  // uint16_t skin_temperature;
  // uint16_t pcb_temperature;
  // uint8_t battery;
  //
  // Skin temperature: Accurate to 0.01 degrees.
  // PCB temperature: Accurate to 0.01 degrees.
  // battery: Percentage of battery.
  MIID_BODY_TEMPERATURE = 0x2000,

  // Mi Band (Huami) bracelet.
  //
  // uin16_t steps;
  // uin8_t sleep;
  // uint8_t rssi;
  //
  // Steps: Current number of steps
  // Sleep:
  //   0x01: Fall asleep
  //   0x02: wake up
  // RSSI: Absolute value of current signal intensity.
  MIID_MI_BAND_HUAMI = 0x2001,

  // Vacuum cleaner (Rui Mi).
  //
  // uint8_t mode;
  // uint8_t gear;
  //
  // Mode:
  //  0x00: Charging
  //  0x01: Standby
  //  0x02: Standard
  //  0x03: Strong
  //  0xFF: Abnormal
  // gear: Current standard gear
  MIID_RUIMI_VACUUM_CLEANER = 0x2002,

  // Black plus bracelet (like one).
  //
  // uint16_t steps;
  // uint8_t heart_rate;
  // uint8_t status;
  //
  // Steps: Number of steps of the day
  // Heart rate: Last heart rate
  // Status: Current activity status
  MIID_BLACK_PLUS_BRACELET = 0x2003,

  /// Vendor-defined event.

  // Flower and grass detector event.
  //
  // uint8_t state;
  //
  // State:
  //   0x00: normal
  //   0x01: unplug
  MIID_FLOWER_AND_GRASS_DETECTOR_EVENT = 0x3000,

  // Qingping Sensor location event.
  //
  // uint8_t location;
  //
  // Location:
  //  0x00: separate from the base
  //  0x01: connect
  MIID_QINGPING_SENSOR_LOCATION_EVENT = 0x3001,

  // Qingping Pomodoro event.
  //
  // uint8_t type;
  //
  // Event type:
  //   0: Start of Pomodoro
  //   1: End of Pomodoro
  //   2: Start of rest
  //   3: End of rest
  MIID_QINGPING_POMODORO_EVENT = 0x3002,

  // Xiaobei toothbrush event (Qinghe Xiaobei) (Beckham toothbrush event).
  //
  // uint8_t type;
  // time_t timestamp;
  // uint8_t score; // optional
  //
  // Types of:
  //  0: Start of brushing
  //  1: End of brushing
  // Timestamp: UTC time
  // Score (optional): This parameter can be added to the end of brushing event: the score of this brushing, 0~100
  MIID_XIAOBEI_TOOTHBRUSH_EVENT = 0x3003
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
  // Control bits, see FrameControl
  FrameControl frame_control;
  // Product ID, each type of product is unique, pid needs to be applied for on the Xiaomi IoT developer platform.
  uint16_t product_id;
  // Ordinal number, used to de-duplication, different events or attributes report require different Frame Counters.
  uint8_t frame_counter;
} PACKED;

struct RawBLEObject {
  MIID id;
  uint8_t data_len;
  // The maximum effective data length for each Object is 10 bytes.
  // 13 - is an align to 16 bytes of whole struct size.
  uint8_t data[13];
} PACKED;

}  // namespace miot
}  // namespace esphome
#endif
