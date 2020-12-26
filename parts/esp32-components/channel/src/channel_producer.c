#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "channel.h"
#include "channel_producer.h"
#include "module.h"

extern Channel root;

void 
channel_producer_init
(Lifesensor_module *module) 
{
    Channel_producer *inst = (Channel_producer *)module;
    channel_init(&inst->channel);
    channel_register(&root, &inst->channel);
}

void channel_producer_dump(Lifesensor_module *module, Lifesensor_logger *logger, Lifesensor_logger_scope *scope) {
    Channel_producer *inst = (Channel_producer *)module;

    Lifesensor_logger_scope subscope;
    Lifesensor_logger_scope subsubscope;
    logger->enter_map(scope, &subscope, "channel");

    logger->log_str(scope, (char *)inst->channel.identifier, "identifier");

    logger->enter_map(&subscope, &subsubscope, "same");
    logger->log_ptr(scope, inst->channel.same.prev, "prev");
    logger->log_ptr(scope, inst->channel.same.next, "next");
    logger->exit(&subscope, &subsubscope);

    logger->enter_map(&subscope, &subsubscope, "unique");
    logger->log_ptr(scope, inst->channel.unique.prev, "prev");
    logger->log_ptr(scope, inst->channel.unique.next, "next");
    logger->exit(&subscope, &subsubscope);

    logger->exit(scope, &subscope);
}

int 
channel_producer_broadcast
(Channel_producer *prod, void *data, int timeout)
{
    return channel_broadcast(&prod->channel, data, timeout, (int (*)(void))&xTaskGetTickCount);
}

void
channel_producer_broadcast_nb
(Channel_producer *prod, void *data)
{
    channel_broadcast_nb(&prod->channel, data);
}

