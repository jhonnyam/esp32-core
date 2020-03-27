#include <transmission.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <unistd.h>
#include <esp_log.h>

#include "channel.h"
#include "log_wifi.h"

#include "test.h"

StackType_t stack[0x1000];
StaticTask_t tcb;

static OUTPUT(ch_ping);

static void ping_over_wifi (void *params);

void
wifi_test_init
(void)
{
    channel_register(&ch_ping);
    ESP_LOGE("TEST", "Start ping task\n");
    xTaskCreateStatic(
            ping_over_wifi, 
            "Ping_Task", 
            0x1000, 
            NULL, 
            1, 
            stack, 
            &tcb
        );
}

static const char *logf = "lifesensor2,src=\"esp\",channel=%u test=%u,foo=\"bar\"\n";
static const char *tags = "";
static Channel *tmp;

static
void 
ping_over_wifi
(void *params)
{
    ESP_LOGE("TEST", "Started ping task\n");

    for (unsigned int i = 0;;i++) {
        
        ESP_LOGE("test", "send broadcast\n");

        _log_wifi_data data = {
            .format = logf,
            .key_name = ch_ping.identifier,
            .value = (i/10)%17,
            .tags = tags
        };
        
        ESP_LOGD("TEST", "Send ping\n");

        channel_init_broadcast(&ch_ping, &tmp);
        channel_broadcast(&ch_ping, &tmp, &data, portMAX_DELAY);
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}