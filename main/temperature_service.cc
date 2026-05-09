#include "temperature_service.h"
#include <esp_log.h>
#include <http.h>
#include <cJSON.h>
#include <cstring>

#define TAG "TempService"
#define TEMPERATURE_URL "http://192.168.5.71/temperature"
#define UPDATE_INTERVAL_US (60 * 1000000)  // 60 seconds

TemperatureService::TemperatureService() {
}

TemperatureService::~TemperatureService() {
    Stop();
}

void TemperatureService::Start() {
    if (running_) {
        return;
    }

    ESP_LOGI(TAG, "Starting temperature service");
    
    esp_timer_create_args_t timer_args = {
        .callback = TimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "temperature_timer",
        .skip_unhandled_events = false,
    };
    
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_, UPDATE_INTERVAL_US));
    
    running_ = true;
    
    // Fetch immediately
    FetchTemperature();
}

void TemperatureService::Stop() {
    if (!running_) {
        return;
    }

    ESP_LOGI(TAG, "Stopping temperature service");
    
    if (timer_ != nullptr) {
        esp_timer_stop(timer_);
        esp_timer_delete(timer_);
        timer_ = nullptr;
    }
    
    running_ = false;
}

void TemperatureService::TimerCallback(void* arg) {
    TemperatureService* service = static_cast<TemperatureService*>(arg);
    service->FetchTemperature();
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static char response_buffer[256];
    static int response_len = 0;
    
    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (evt->data_len < sizeof(response_buffer) - response_len) {
                memcpy(response_buffer + response_len, evt->data, evt->data_len);
                response_len += evt->data_len;
                response_buffer[response_len] = '\0';
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            if (evt->user_data != nullptr) {
                *(char**)evt->user_data = response_buffer;
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            response_len = 0;
            break;
        default:
            break;
    }
    return ESP_OK;
}

void TemperatureService::FetchTemperature() {
    char* response_data = nullptr;
    
    esp_http_client_config_t config = {};
    config.url = TEMPERATURE_URL;
    config.event_handler = http_event_handler;
    config.user_data = &response_data;
    config.timeout_ms = 5000;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return;
    }
    
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200 && response_data != nullptr) {
            // Parse JSON response
            cJSON* root = cJSON_Parse(response_data);
            if (root != nullptr) {
                cJSON* temp = cJSON_GetObjectItem(root, "temperature");
                cJSON* unit = cJSON_GetObjectItem(root, "unit");
                
                if (cJSON_IsNumber(temp)) {
                    temperature_ = (float)temp->valuedouble;
                    valid_ = true;
                    
                    if (cJSON_IsString(unit)) {
                        unit_ = unit->valuestring;
                    }
                    
                    ESP_LOGI(TAG, "Temperature updated: %.1f %s", temperature_, unit_.c_str());
                } else {
                    ESP_LOGW(TAG, "Invalid temperature data in JSON");
                    valid_ = false;
                }
                
                cJSON_Delete(root);
            } else {
                ESP_LOGE(TAG, "Failed to parse JSON response");
                valid_ = false;
            }
        } else {
            ESP_LOGW(TAG, "HTTP request failed with status: %d", status_code);
            valid_ = false;
        }
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        valid_ = false;
    }
    
    esp_http_client_cleanup(client);
}
