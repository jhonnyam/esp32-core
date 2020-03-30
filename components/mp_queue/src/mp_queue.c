#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "channel.h"
#include "mp_queue.h"

#define MP_TEST_STACK_SIZE 0x1000

static void producer(void *pvParameters);
static void consumer(void *pvParameters);

static Channel_in in1, in2;
static Channel_in * in[] = {&in1, &in2};
static Channel_out out1, out2;

static uint8_t c1_queue_buffer[8*sizeof(float)];
static uint8_t c2_queue_buffer[8*sizeof(float)];

static StaticQueue_t queue1;
static StaticQueue_t queue2;

static StaticTask_t p1, p2, c1;
static StackType_t p1_task_stack[MP_TEST_STACK_SIZE];
static StackType_t p2_task_stack[MP_TEST_STACK_SIZE];
static StackType_t c1_task_stack[MP_TEST_STACK_SIZE];

void
test_mp_queue_init() {
	QueueHandle_t q1 = xQueueCreateStatic(
		8,
		sizeof(float),
		c1_queue_buffer,
		&queue1
	);

	QueueHandle_t q2 = xQueueCreateStatic(
		8,
		sizeof(float),
		c2_queue_buffer,
		&queue2
	);


	ESP_LOGD("mp_queue:init", "Init input channels");
	channel_init(&in1, "ch1", &q1, queueSEND_TO_BACK, &xQueueGenericSend);
	channel_init(&in1, "ch2", &q2, queueSEND_TO_BACK, &xQueueGenericSend);
	ESP_LOGD("mp_queue:init", "Init output channels");
	channel_init(&out1, "ch1", NULL, 0, NULL);
	channel_init(&out2, "ch2", NULL, 0, NULL);

	ESP_LOGD("mp_queue:init", "Satrting Tasks");
	xTaskCreateStatic(
		producer,
		"prod_1",
		MP_TEST_STACK_SIZE,
		(void *)&out1,
		2,
		p1_task_stack,
		&p1
	);
	ESP_LOGD("mp_queue:init", "Satrted p1");

	xTaskCreateStatic(
		producer,
		"prod_2",
		MP_TEST_STACK_SIZE,
		(void *)&out2,
		3,
		p2_task_stack,
		&p2
	);
	ESP_LOGD("mp_queue:init", "Satrted p2");

	xTaskCreateStatic(
		consumer,
		"cons_1",
		MP_TEST_STACK_SIZE,
		(void *)in,
		4,
		c1_task_stack,
		&c1
	);
	ESP_LOGD("mp_queue:init", "Satrted c1");
}


static void producer(void *pvParameters) {
	Channel_out *ch = (Channel_out *)pvParameters;
	Channel_broadcast b;
	float data = 0.0;
	ESP_LOGI("mp_queue::producer", "Satrted");
	for (int i = 0;;i++) {
		data = i / 3;
		ESP_LOGI("producer", "sending %f to channel %s", data, ch->identifier);
		channel_broadcast_init(&b, ch, &data, 0);
		channel_broadcast(&b);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

static void consumer(void *pvParameters) {
	float d1 = 0.0, d2 = 0.0, sum = 0.0;
	ESP_LOGI("mp_queue::consumer", "Satrted");
	for (;;) {
		if (xQueueReceive((QueueHandle_t)in1.ctx, &d1, 0)) {
			ESP_LOGI("consumer", "received %f on channel %s", d1, in1.identifier);
			sum += d1;
		}
		if (xQueueReceive((QueueHandle_t)in2.ctx, &d2, 0)) {
			ESP_LOGI("consumer", "received %f on channel %s", d2, in2.identifier);
			sum += d2;
		}
		ESP_LOGI("consumer", "Sum: %f", sum);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}
