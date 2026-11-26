#pragma once

#ifdef USE_MQTT

#include "esphome/core/defines.h"
#include "esphome/core/version.h"

#include "esphome/core/application.h"
#include "esphome/components/mqtt/mqtt_client.h"

#include "esphome/components/dtu/dtu.h"

namespace esphome {
namespace settings {

namespace detail {

// ESPHome 2026.10 marks MQTTClientComponent as final, so the accessor cannot be derived
// from it anymore. The settings UI still needs the protected message/credential fields, so
// take pointers to them using the friend-injection trick. It works for private/protected
// members regardless of the class being final and does not depend on the ESPHome version.
template<typename Tag, typename Tag::type M> struct Rob {
  friend typename Tag::type get(Tag) { return M; }
};

#define SETTINGS_MQTT_MESSAGE_FIELD(tag, field)                                          \
  struct tag {                                                                           \
    using type = ::esphome::mqtt::MQTTMessage (::esphome::mqtt::MQTTClientComponent::*); \
    friend type get(tag);                                                                \
  };                                                                                     \
  template struct Rob<tag, &::esphome::mqtt::MQTTClientComponent::field>

SETTINGS_MQTT_MESSAGE_FIELD(BirthMessage, birth_message_);
SETTINGS_MQTT_MESSAGE_FIELD(LastWill, last_will_);
SETTINGS_MQTT_MESSAGE_FIELD(ShutdownMessage, shutdown_message_);
SETTINGS_MQTT_MESSAGE_FIELD(LogMessage, log_message_);

#undef SETTINGS_MQTT_MESSAGE_FIELD

struct CredentialsField {
  using type = ::esphome::mqtt::MQTTCredentials (::esphome::mqtt::MQTTClientComponent::*);
  friend type get(CredentialsField);
};
template struct Rob<CredentialsField, &::esphome::mqtt::MQTTClientComponent::credentials_>;

}  // namespace detail

class MQTTClientComponentAccessor {
 public:
  explicit MQTTClientComponentAccessor(mqtt::MQTTClientComponent *ref) : ref_(ref) {}

  const std::string topic_prefix() const {
    auto s = this->ref_->get_topic_prefix();
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
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 2, 0) && ESPHOME_VERSION_CODE < VERSION_CODE(2026, 11, 0)
    this->ref_->set_topic_prefix(s, "");
#else
    this->ref_->set_topic_prefix(s);
#endif
  }

  const std::string log_message_topic() const {
    return this->unprefix_((this->ref_->*get(detail::LogMessage{})).topic);
  }
  void log_message_topic(const std::string &topic) {
    this->ref_->set_log_message_template(this->mk_topic_(topic, this->ref_->*get(detail::LogMessage{})));
  }

  const std::string log_message_topic_full() const { return (this->ref_->*get(detail::LogMessage{})).topic; }

  const std::string last_will_topic() const {
    return this->unprefix_((this->ref_->*get(detail::LastWill{})).topic);
  }
  void last_will_topic(const std::string &topic) {
    this->ref_->set_last_will(this->mk_topic_(topic, this->ref_->*get(detail::LastWill{})));
  }

  const std::string birth_message_topic() const {
    return this->unprefix_((this->ref_->*get(detail::BirthMessage{})).topic);
  }
  void birth_message_topic(const std::string &topic) {
    this->ref_->set_birth_message(this->mk_topic_(topic, this->ref_->*get(detail::BirthMessage{})));
  }

  const std::string shutdown_message_topic() const {
    return this->unprefix_((this->ref_->*get(detail::ShutdownMessage{})).topic);
  }
  void shutdown_message_topic(const std::string &topic) {
    this->ref_->set_shutdown_message(this->mk_topic_(topic, this->ref_->*get(detail::ShutdownMessage{})));
  }

  const std::string &get_broker_address() const { return (this->ref_->*get(detail::CredentialsField{})).address; }

 protected:
  inline static bool trim_pred_(const char c) { return std::isspace(c) || c == '/'; }

  const std::string unprefix_(const std::string &topic) const {
    auto s = topic;
    if (!s.empty()) {
      auto prefix = this->ref_->get_topic_prefix();
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
    // trim only spaces to check slash later
    dtu::str_ltrim_ref(s);
    if (!s.empty()) {
      // check that user wants his own prefix
      if (s.front() == '/') {
        dtu::str_ltrim_ref(s, trim_pred_);
      } else {
        const auto topic_prefix = this->ref_->get_topic_prefix();
        if (!topic_prefix.empty()) {
          if (topic_prefix.back() != '/') {
            s.insert(s.cbegin(), '/');
          }
          s.insert(0, topic_prefix);
        }
      }
    }

    return mqtt::MQTTMessage{
        .topic = s,
        .payload = src.payload,
        .qos = src.qos,
        .retain = src.retain,
    };
  }

  mqtt::MQTTClientComponent *ref_;
};

inline MQTTClientComponentAccessor mqtt_access(mqtt::MQTTClientComponent *ref) {
  return MQTTClientComponentAccessor(ref);
}

}  // namespace settings
}  // namespace esphome

#endif
