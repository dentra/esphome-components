# Setup

Allow execute your code at end of setup process.

Parameters:

- depends: Optional string or list of strings. Component names required in config to generate lambda.
  You could also set alias for component to use in your code.
- lambda: c++ code to execute

## Examples

```yaml
setup: |-
  // put your c++ code here
```

```yaml
setup:
  lambda: |-
    // put your c++ code here
```

Example using depends:

```yaml
setup:
  depends: mqtt
  lambda: |-
    // g_mqtt is defined with id in config
    ESP_LOGI("setup", "MQTT prefix is %s", id(g_mqtt).get_topic_prefix());
```

Example using depends with alias:

```yaml
setup:
  depends: mqtt as my_mqtt
  lambda: |-
    // my_mqtt may not be defined with id in config
    ESP_LOGI("setup", "MQTT prefix is %s", my_mqtt->get_topic_prefix());
```

Complex example:

```yaml
setup:
  - depends:
      - mqtt as my_mqtt
      - ota
    lambda: |-
      // do something when only mqtt and ota exists in then config
  - depends: [api, ota]
    lambda: |-
      // do something when only api and ota exists in the config
```
