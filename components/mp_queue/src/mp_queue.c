#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "channel.h"
#include "mp_queue.h"

#define MP_TEST_STACK_SIZE 0x1000

static void producer(void *pvParameters);
static void producer_slow(void *pvParameters);
static void consumer(void *pvParameters);
static void consumer_slow(void *pvParameters);

static Channel_in in1[2];
static Channel_in in2[2];
static Channel_out out1, out2;

static uint8_t c1_q1_buffer[2*sizeof(float)];
static uint8_t c1_q2_buffer[2*sizeof(float)];
static uint8_t c2_q1_buffer[2*sizeof(float)];
static uint8_t c2_q2_buffer[2*sizeof(float)];

static StaticQueue_t c1_q1;
static StaticQueue_t c1_q2;
static StaticQueue_t c2_q1;
static StaticQueue_t c2_q2;

static StaticTask_t p1, p2, c1, c2;
static StackType_t p1_task_stack[MP_TEST_STACK_SIZE];
static StackType_t p2_task_stack[MP_TEST_STACK_SIZE];
static StackType_t c1_task_stack[MP_TEST_STACK_SIZE];
static StackType_t c2_task_stack[MP_TEST_STACK_SIZE];

void
test_mp_queue_init() {
	QueueHandle_t q11 = xQueueCreateStatic(
		2,
		sizeof(float),
		c1_q1_buffer,
		&c1_q1
	);

	QueueHandle_t q12 = xQueueCreateStatic(
		2,
		sizeof(float),
		c1_q2_buffer,
		&c1_q2
	);

	QueueHandle_t q21 = xQueueCreateStatic(
		2,
		sizeof(float),
		c2_q1_buffer,
		&c2_q1
	);

	QueueHandle_t q22 = xQueueCreateStatic(
		2,
		sizeof(float),
		c2_q2_buffer,
		&c2_q2
	);

	channel_init_input(&in1[1], "ch1", q11);
	channel_init_input(&in1[2], "ch2", q12);
	channel_init_input(&in2[1], "ch1", q21);
	channel_init_input(&in2[2], "ch2", q22);
	
	channel_init_output(&out1, "ch1");
	channel_init_output(&out2, "ch2");

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
		producer_slow,
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
		(void *)in1,
		4,
		c1_task_stack,
		&c1
	);
	ESP_LOGD("mp_queue:init", "Satrted c1");

	xTaskCreateStatic(
		consumer_slow,
		"cons_2",
		MP_TEST_STACK_SIZE,
		(void *)in2,
		5,
		c2_task_stack,
		&c2
	);
	ESP_LOGD("mp_queue:init", "Satrted c1");
}


static void producer(void *pvParameters) {
	Channel_out *ch = (Channel_out *)pvParameters;
	Channel_broadcast b;
	float data = 0.0;
	ESP_LOGI("mp_queue::producer", "Satrted");
	for (int i = 0;;i++) {
		data = (float)i;
		ESP_LOGW("producer_skip", "sending %f to channel %s", data, ch->identifier);
		channel_broadcast_init(&b, ch, &data, 1000/portTICK_PERIOD_MS);
		channel_broadcast(&b);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

static void producer_slow(void *pvParameters) {
	Channel_out *ch = (Channel_out *)pvParameters;
	Channel_broadcast b;
	float data = 0.0;
	ESP_LOGI("mp_queue::producer", "Satrted");
	for (int i = 0;;i++) {
		data = (float)i;
		ESP_LOGW("producer_block", "sending %f to channel %s", data, ch->identifier);
		channel_broadcast_init(&b, ch, &data, 1000/portTICK_PERIOD_MS);
		while (!channel_broadcast_finished(&b))
		{
			int i = channel_broadcast(&b);
			ESP_LOGI("bc", "i: %d", i);
		}
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

static void consumer(void *pvParameters) {
	Channel_in *in = (Channel_in *)pvParameters;
	float d1 = 0.0, d2 = 0.0, sum = 0.0;
	ESP_LOGI("mp_queue::consumer", "Satrted");
	for (;;) {
		if (xQueueReceive((QueueHandle_t)in[1].ctx, &d1, 0)) {
			ESP_LOGI("consumer", "received %f on channel %s", d1, in[1].identifier);
			sum += d1;
		}
		ESP_LOGI("consumer", "Sum: %f", sum);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

static void consumer_slow(void *pvParameters) {
	Channel_in *in = (Channel_in *)pvParameters;
	float d1 = 0.0, d2 = 0.0, sum = 0.0;
	ESP_LOGI("mp_queue::consumer_slow", "Satrted");
	for (;;) {
		if (xQueueReceive((QueueHandle_t)in[2].ctx, &d2, 0)) {
			ESP_LOGI("consumer_slow", "received %f on channel %s", d2, in[2].identifier);
			sum += d2;
		}
		ESP_LOGI("consumer_slow", "Sum: %f", sum);
		vTaskDelay(3000/portTICK_PERIOD_MS);
	}
}
