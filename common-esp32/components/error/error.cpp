#include "error.h"

#include <system_error>

using namespace std;

void check(esp_err_t err) {
  if (err == ESP_OK) return;
  throw system_error((int)err, system_category());
}

void check(esp_err_t err, string what) {
  if (err == ESP_OK) return;
  throw system_error((int)err, system_category(), what);
}

void error(const string what) { throw runtime_error(what); }
