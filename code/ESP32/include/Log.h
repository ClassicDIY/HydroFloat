#pragma once

#include "esp_log.h"
#include "Defines.h"

void inline printHexString(char* ptr, int len)
{
#if APP_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
	esp_log_level_set(TAG, ESP_LOG_DEBUG);
	esp_log_buffer_hex_internal(TAG, ptr, len, ESP_LOG_DEBUG);
#endif
}

#if APP_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
#define logv(format, ...) log_printf(ARDUHAL_LOG_FORMAT(V, format), ##__VA_ARGS__)
#else
#define logv(format, ...)
#endif

#if APP_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
#define logd(format, ...) log_printf(ARDUHAL_LOG_FORMAT(D, format), ##__VA_ARGS__)
#else
#define logd(format, ...)
#endif

#if APP_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
#define logi(format, ...) log_printf(ARDUHAL_LOG_FORMAT(I, format), ##__VA_ARGS__)
#else
#define logi(format, ...)
#endif

#if APP_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN
#define logw(format, ...) log_printf(ARDUHAL_LOG_FORMAT(W, format), ##__VA_ARGS__)
#else
#define logw(format, ...)
#endif

#if APP_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
#define loge(format, ...) log_printf(ARDUHAL_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define loge(format, ...)
#endif

