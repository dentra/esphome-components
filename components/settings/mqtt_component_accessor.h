#pragma once

#include "esphome/core/defines.h"
#ifdef USE_MQTT

#include "esphome/core/application.h"
#include "esphome/components/mqtt/mqtt_client.h"

#include "esphome/components/dtu/dtu.h"

namespace esphome {
namespace settings {
class MQTTClientComponentAccessor : public mqtt::MQTTClientComponent {
 public:
  MQTTClientComponentAccessor() = delete;

  static MQTTClientComponentAccessor *make(mqtt::MQTTClientComponent *ref) {
    return reinterpret_cast<MQTTClientComponentAccessor *>(ref);
  }

  const std::string topic_prefix() const {
    auto s = this->get_topic_prefix();
    if (!s.empty() && App.is_name_add_mac_suffix_enabled()) {
      auto suffix = dtu::get_mac_suffix();
      if (str_endswith(s, suffix)) {
        s.erase(s.size() - suffix.size());
      }
    }
    return s;
  }
  void topic_prefix(const std::string &topic_prefix) {
    auto s = dtu::str_trim(topic_prefix, trim_pred_);
    if (!s.empty() && App.is_name_add_mac_suffix_enabled()) {
      auto suffix = dtu::get_mac_suffix();
      if (!str_endswith(s, suffix)) {
        s.append(suffix);
      }
    }
    this->set_topic_prefix(s);
  }

  const std::string log_message_topic() const { return this->unprefix_(this->log_message_.topic); }
  void log_message_topic(const std::string &topic) {
    this->set_log_message_template(this->mk_topic_(topic, this->log_message_));
  }

  const std::string log_message_topic_full() const { return this->log_message_.topic; };

  const std::string last_will_topic() const { return this->unprefix_(this->last_will_.topic); }
  void last_will_topic(const std::string &topic) { this->set_last_will(this->mk_topic_(topic, this->last_will_)); }

  const std::string birth_message_topic() const { return this->unprefix_(this->birth_message_.topic); }
  void birth_message_topic(const std::string &topic) {
    this->set_birth_message(this->mk_topic_(topic, this->birth_message_));
  }

  const std::string shutdown_message_topic() const { return this->unprefix_(this->shutdown_message_.topic); }
  void shutdown_message_topic(const std::string &topic) {
    this->set_shutdown_message(this->mk_topic_(topic, this->shutdown_message_));
  }

  const std::string &get_broker_address() const { return this->credentials_.address; }

 protected:
  inline static bool trim_pred_(const char c) { return std::isspace(c) || c == '/'; }

  const std::string unprefix_(const std::string &topic) const {
    auto s = topic;
    if (!s.empty()) {
      auto prefix = this->get_topic_prefix();
      prefix.append(1, '/');
      if (str_startswith(s, prefix)) {
        s.erase(0, prefix.length());
      } else {
        s.insert(s.cbegin(), '/');
      }
    }
    return s;
  }

  // make topic with appending topic prefix.
  mqtt::MQTTMessage mk_topic_(const std::string &topic, const mqtt::MQTTMessage &src) {
    auto s = dtu::str_rtrim(topic, trim_pred_);
    dtu::str_ltrim_ref(s);  // trim only spaces to check slash later
    if (!s.empty()) {
      // check that user wants his own prefix
      if (s.front() == '/') {
        dtu::str_ltrim_ref(s, trim_pred_);
      } else {
        const auto topic_prefix = this->get_topic_prefix();
        if (topic_prefix.back() != '/') {
          s.insert(s.cbegin(), '/');
        }
        s.insert(0, topic_prefix);
      }
    }

    return mqtt::MQTTMessage{
        .topic = s,
        .payload = src.payload,
        .qos = src.qos,
        .retain = src.retain,
    };
  }
};

inline MQTTClientComponentAccessor *mqtt_access(mqtt::MQTTClientComponent *ref) {
  return MQTTClientComponentAccessor::make(ref);
}

}  // namespace settings
}  // namespace esphome

#endif
