#include <transmission.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <unistd.h>
#include <esp_log.h>

static const char *ping = "ping";

static void ping_over_wifi(void *params);
StackType_t stack[0x1000];
StaticTask_t tcb;

void
wifi_test_init
(void)
{
    // ESP_LOGD("WIFI", "Starting ping task");
    xTaskCreateStatic(ping_over_wifi, "Ping_Task", 0x1000, NULL, 1, stack, &tcb);
    // ESP_LOGD("WIFI", "Started ping task");
}

// static uint64_t ts = 1583600881002186352;1583605744622675324


static const char *logf = "lifesensor2,src=\"esp\",channel=%u test=%u,foo=\"bar\"\n";
static char line_buffer[256];

static
void 
ping_over_wifi
(void *params)
{
    for (unsigned int i = 0;;i++) {
        // ESP_LOGD("ping_over_wifi", "Trying to send ping over wifi");
        unsigned int size = sprintf(line_buffer, logf, i%3, (i%20 + (i%3) * 10));
        transmission_send(line_buffer, size);
        // ESP_LOGD("ping_over_wifi", "Sleep 1000ms");
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}
