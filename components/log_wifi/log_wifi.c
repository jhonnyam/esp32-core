#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"

#include "wifi.h"
#include "transmission.h"
#include "log_wifi.h"
#include "channel.h"


static void log_wifi_runner(void *pvParameters);


static uint8_t log_wifi_queue_buffer[LOG_WIFI_QUEUE_BUFFER_SIZE];
_log_wifi_queue log_wifi_queue = {
	.length = LOG_WIFI_QUEUE_SIZE,
	.item_size = LOG_WIFI_QUEUE_ITEM_SIZE,
	.buffer = log_wifi_queue_buffer
};

static INPUT_QUEUE(ch_ping, &log_wifi_queue.handle);

void
log_wifi_init
(_log_wifi_task *log_task)
{
    ESP_LOGE("LOG_WIFI", "INIT");
    wifi_init();
	wifi_wait_connected();
    transmission_init();
    ESP_LOGE("LOG_WIFI", "INIT Done");
    log_wifi_queue.handle = xQueueCreateStatic(
        log_wifi_queue.length,
        log_wifi_queue.item_size,
        log_wifi_queue.buffer,
        &log_wifi_queue.queue
    );

    channel_register(&ch_ping);
    ESP_LOGE("LOG_WIFI", "Strating Task log_wifi_runner");

    log_task->handle = xTaskCreateStatic(
        log_wifi_runner,
        log_task->name,
        LOG_WIFI_TASK_STACK_SIZE,
        (void*)&log_wifi_queue,
        log_task->priority,
        log_task->stack,
        &log_task->tcb
    );
}

void
log_wifi_runner
(void *pvParameters)
{
    _log_wifi_queue *log_queue = (_log_wifi_queue *)pvParameters;
    static _log_wifi_data data = {0};
    static char line_buffer[LOG_WIFI_LINE_MAX_LENGTH];
    
    ESP_LOGD("LOG", "Wifi log task started");

    for(;;)
    {

        ESP_LOGE("log_wifi", "Waiting for data");
        xQueueReceive(&log_queue->queue, &data, portMAX_DELAY);
        ESP_LOGE("log_wifi", "Received on channel %s\n", data.key_name);
        ESP_LOGD("log_wifi", "Format: %s", data.format);
        ESP_LOGD("log_wifi", "tags: %s", data.tags);
        ESP_LOGD("log_wifi", "key_name: %s", data.key_name);
        ESP_LOGD("log_wifi", "value: %f", data.value);
        int len = snprintf(
            line_buffer, 
            LOG_WIFI_LINE_MAX_LENGTH, 
            data.format, 
            data.tags, 
            data.key_name, 
            data.value
        );
        if(len > 0) {
            transmission_send(line_buffer, len);
        }
    }
}