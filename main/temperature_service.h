#ifndef _TEMPERATURE_SERVICE_H_
#define _TEMPERATURE_SERVICE_H_

#include <string>
#include <esp_timer.h>
#include <functional>

class TemperatureService {
public:
    TemperatureService();
    ~TemperatureService();

    void Start();
    void Stop();
    
    float GetTemperature() const { return temperature_; }
    bool IsValid() const { return valid_; }
    const char* GetUnit() const { return unit_.c_str(); }

private:
    static void TimerCallback(void* arg);
    void FetchTemperature();

    esp_timer_handle_t timer_ = nullptr;
    float temperature_ = 0.0f;
    std::string unit_ = "celsius";
    bool valid_ = false;
    bool running_ = false;
};

#endif // _TEMPERATURE_SERVICE_H_
