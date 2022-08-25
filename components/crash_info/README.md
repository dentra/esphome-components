# Crash Info

An ESP8266 remote crash detector.
With enabled indicator you can monitor crashes right from your Home Assistant.

## Advanced usage
Connect to you ESP8266 and find crash backtrace information in the log.
Copy stack frames and decode with `xtensa-lx106-elf-addr2line -aipfC -e $elf ...` command.

> Crash information stored in RTC memory and do not survive power loss.

```yaml
# Example configuration entry.
crash_info:
  id: crash_info_obj
  # Optional, uint32. The number of stack frames to be saved.
  max_stack_frames_size: 10
  # Optional, uint32. Minimum address of stack frame to be saved. Default: 0x40000000
  min_stack_frames_addr: 0x40000000
  # Optional, uint32. Maximum address of stack frame to be saved. Default: 0x50000000
  max_stack_frames_addr: 0x50000000
  # Optional, binary_sensor. Crash indicator.
  indicator:
    name: $name Crash state

# Add button to reset state of crash.
button:
  - platform: template
    name: $name Reset crash state
    on_press:
      lambda: id(crash_info_obj).reset();

# Add sntp or homeassistant time platform to enable saving crash time.
time:
  - platform: sntp
    timezone: UTC-3
    id: time_sntp_obj

```
