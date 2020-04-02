#include "unity.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "channel.h"

#define MP_TEST_STACK_SIZE 0x1000
#define CHANNEL_QUEUE_TEST_QUEUE_SIZE 2

// #ifdef CONFIG_TEST_CHANNEL_QUEUE_CONSUMER

#define CHANNEL_QUEUE_TEST_IDENTIFIER_C1 "test_c1"
static Channel_consumer cc;
static StaticQueue cc_queue;
static uint8_t cc_queue_buffer[CHANNEL_QUEUE_TEST_QUEUE_SIZE*sizeof(float)];

static float data;
static uint8_t timeout;
static void channel_consumer_task(void *pvParameters)
{
    data = 0;
    for (;;) {
        xQueueReceive((QueueHandle_t)cc.ctx, &data, 100/portTICK_PERIOD_MS);
        timeout = 1;
    }
}

TEST_CASE("channel_queue", "[channel]")
{

    static QueueHandle_t cc_queue_handle = xQueueCreateStatic(
        CHANNEL_QUEUE_TEST_QUEUE_SIZE,
        sizeof(float),
        cc_queue_buffer,
        &cc_queue
    );

    channel_init_consumer(&cc, CHANNEL_QUEUE_TEST_IDENTIFIER_C1, cc_queue);

    static StackType_t cc_task_stack[MP_TEST_STACK_SIZE];
    static StaticTask_t cc_task;
    
    static Channel_producer cp;
    
    channel_init_producer(&cp, CHANNEL_QUEUE_TEST_IDENTIFIER_C1);

    timeout = 0;
    cc_task = xTaskCreateStatic(
        channel_consumer_task,
        "channel_consumer_task",
        MP_TEST_STACK_SIZE,
        NULL,
        1,
        cc_task_stack,
        &cc_task
    );
    configASSERT(cc_task);

    Channel_broadcast br;
    float test_data = 3.14;
    channel_broadcast_init(&br, &cp, &test_data, 0);
    channel_broadcast(&br);
    
    while(!timeout){}
    TEST_ASSERT_EQUAL_FLOAT(data, test_data);
}
// #endif