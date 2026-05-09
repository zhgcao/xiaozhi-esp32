#include "temperature_service.h"
#include "boards/common/board.h"
#include <esp_log.h>
#include <cJSON.h>

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

void TemperatureService::FetchTemperature() {
    auto& board = Board::GetInstance();
    auto http = board.GetNetwork()->CreateHttp(3);
    
    if (!http->Open("GET", TEMPERATURE_URL)) {
        ESP_LOGE(TAG, "Failed to open URL: %s", TEMPERATURE_URL);
        valid_ = false;
        return;
    }
    
    int status_code = http->GetStatusCode();
    if (status_code != 200) {
        ESP_LOGW(TAG, "HTTP request failed with status: %d", status_code);
        http->Close();
        valid_ = false;
        return;
    }
    
    std::string response = http->ReadAll();
    http->Close();
    
    if (response.empty()) {
        ESP_LOGE(TAG, "Empty response from server");
        valid_ = false;
        return;
    }
    
    // Parse JSON response
    cJSON* root = cJSON_Parse(response.c_str());
    if (root == nullptr) {
        ESP_LOGE(TAG, "Failed to parse JSON response");
        valid_ = false;
        return;
    }
    
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
}
