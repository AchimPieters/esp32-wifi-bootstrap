#include "esp_err.h"
#include "esp_idf_version.h"
#include "esp_wifi.h"

// Fallback-declaratie voor IDF 5.0+ als header hem niet definieert
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
esp_err_t esp_wifi_set_auto_connect(bool enable);
#endif

esp_err_t safe_set_auto_connect(bool enable) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    return esp_wifi_set_auto_connect(enable);
#else
    return ESP_OK;
#endif
}