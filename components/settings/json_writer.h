#pragma once

#include "helpers.h"

namespace esphome {

namespace settings {

template<class T> class JsonWriter {
 public:
  JsonWriter(T *stream) : s_(stream) {}

  void start_object() const { this->print_raw("{"); }
  void end_object() const { this->print_raw("}"); }
  void start_object(const char *key) const {
    this->start_kv(key);
    this->start_object();
  }

  void start_array() const { this->print_raw("["); }
  void end_array() const { this->print_raw("]"); }
  void start_array(const char *key) {
    this->start_kv(key);
    this->start_array();
  }

  void add_comma(bool cond = true) const {
    if (cond) {
      this->print_raw(",");
    }
  }

  void start_kv(const char *key) const {
    this->print_raw("\"");
    this->print_raw(key);
    this->print_raw("\":");
  }

  // string val is not escaped, you must escape it first
  void add_kv(const char *key, const char *val, bool add_comma = true) const {
    if (val == nullptr || *val == 0) {
      return;
    }
    this->start_kv(key);
    this->print_raw("\"");
    this->print_raw(val);
    this->print_raw("\"");
    this->add_comma(add_comma);
  }

  // string val is not escaped, you must escape it first
  void add_kv(const char *key, const std::string &val, bool add_comma = true) const {
    if (val.empty()) {
      return;
    }
    this->start_kv(key);
    this->print_raw("\"");
    this->print_raw(val);
    this->print_raw("\"");
    this->add_comma(add_comma);
  }

  // string value is not quoted, you must quote it first if you need it
  void add_kv_nq(const char *key, const char *val, bool add_comma = true) const {
    if (val == nullptr || *val == 0) {
      return;
    }
    this->start_kv(key);
    this->print_raw(val);
    this->add_comma(add_comma);
  }

  // string value is not quoted, you must quote it first if you need it
  void add_kv_nq(const char *key, const std::string &val, bool add_comma = true) const {
    if (val.empty()) {
      return;
    }
    this->start_kv(key);
    this->print_raw(val);
    this->add_comma(add_comma);
  }

  template<typename V,
           enable_if_t<(!std::is_same<V, std::string>::value && !std::is_same<V, const char *>::value), int> = 0>
  void add_kv(const char *key, V val, bool add_comma = true) const {
    this->add_kv_nq(key, to_string(val), add_comma);
  }

  void print_raw(const char *val) const { this->s_->print(val); }
  void print_raw(const std::string &val) const {
#ifdef USE_ARDUINO
    this->s_->print(val.c_str());
#else
    this->s_->print(val);
#endif
  }

  operator T *() { return this->s_; }

 protected:
  T *s_{};
};

}  // namespace settings
}  // namespace esphome
