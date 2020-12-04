#ifndef __CHANNEL_CONSUMER__
#define __CHANNEL_CONSUMER__

#include <freertos/freeRTOS.h>
#include <freertos/queue.h>

#include "channel.h"
#include "module.h"

struct channel_consumer
{
    Lifesensor_module module;
    Channel channel;
    QueueHandle_t handle;
    StaticQueue_t queue;
    size_t depth;
    size_t elem_size;
    void *buffer;
    void (*error_handler)(void);
};
typedef channel_consumer Channel_consumer;

void channel_consumer_init(Lifesensor_module *module);
void channel_consumer_dump(Lifesensor_module *module, Lifesensor_logger *logger, Lifesensor_logger_scope *scope);
int channel_consumer_callback(QueueHandle_t queue, const void *data, const int timeout);
int channel_consumer_callback_nb(QueueHandle_t queue, const void *data);


#define INIT_CHANNEL_CONSUMER(NAME, TYPE, DEPTH, ERROR_HANDLER)
{ \
    .module  = INIT_LIFESENSOR_MODULE( \
        Channel_consumer, \
        NAME, \
        &channel_consumer_init, \
        &channel_consumer_dump\
    ),
    .channel = INIT_CHANNEL(\
        NAME,\
        NULL,\
        &channel_consumer_callback,\
        &channel_consumer_calback_nb \
    ), \
    .handle  = NULL, \
    .queue   = { 0 }, \
    .depth   = (DEPTH), \
    .elem_size = sizeof(TYPE), \
    .buffer  = ((TYPE)[(DEPTH)]){ 0 }, \
    .error_handler = ERROR_HANDLER, \
}



#endif