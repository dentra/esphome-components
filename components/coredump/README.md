# Coredump

This component configures the firmware for storage and downloading [ESP-IDF core dump](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/core_dump.html).

> WARNING: This is an advanced component, and you need to understand what you need it for.

## Configuration

```yaml
external_components:
  - source: github://dentra/esphome-components
    # we need additional partitions component here
    components: [coredump, partitions]

coredump:
```

## Usage
Navigate to http://[node_name].local/coredump/ where [node_name] is the name of your esphome configured node. Now here you have a buttons with actions:
* Download - download coredump.elf to your PC (available only when core dump exist)
* Erase - erase existing core dump information (available only when core dump exist)
* Test Crash - generate assertion to test core dump (available only when no cored ump exists)

## Decoding

To decode core dump you need a firmware ELF binary. Here is a sample decoding command:

```bash
espcoredump.py --chip esp32c3 info_corefile \
  --core ~/Downloads/coredump.elf \
  config/.esphome/build/coredump-test/.pioenvs/coredump-test/firmware.elf
```
