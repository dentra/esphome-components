# Setup

Allow execute your code at end of setup process.

```yaml
setup: |-
  #ifdef USE_MQTT
    if (App.is_name_add_mac_suffix_enabled()) {
      auto topic_prefix = str_sanitize(App.get_name());
      if (id(g_mqtt).get_topic_prefix() != topic_prefix) {
        id(g_mqtt).set_topic_prefix(topic_prefix);
      }
      if (id(g_mqtt).is_log_message_enabled()) {
        auto log_topic = topic_prefix + "/debug";
        id(g_mqtt).set_log_message_template(mqtt::MQTTMessage{
          .topic = log_topic, .payload = "", .qos = 0, .retain = true
        });
      }
      auto status_topic = topic_prefix + "/status";
      id(g_mqtt).set_birth_message(mqtt::MQTTMessage{
        .topic = status_topic, .payload = "online", .qos = 0, .retain = true
      });
      id(g_mqtt).set_last_will(mqtt::MQTTMessage{
        .topic = status_topic, .payload = "offline", .qos = 0, .retain = true
      });
      id(g_mqtt).set_shutdown_message(mqtt::MQTTMessage{
        .topic = status_topic, .payload = "offline", .qos = 0, .retain = true
      });
    }
  #endif
```
