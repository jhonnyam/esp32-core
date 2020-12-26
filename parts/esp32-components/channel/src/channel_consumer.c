#include "channel_consumer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

extern Channel root;

void channel_consumer_init(Lifesensor_module *module) {
    Channel_consumer *inst = (Channel_consumer *) module;
    channel_init(&inst->channel);
    inst->handle = xQueueCreateStatic(
        inst->depth,
        inst->elem_size,
        inst->buffer,
        &inst->queue
    );
    inst->channel.ctx = &inst->queue;
    channel_register(&root, &inst->channel);
}

void channel_consumer_dump(Lifesensor_module *module, Lifesensor_logger *logger, Lifesensor_logger_scope *scope) {
    Channel_consumer *inst = (void *)module;

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

    logger->log_ptr(scope, inst->channel.ctx, "ctx");
    logger->log_ptr(scope, inst->channel.callback, "callback");
    logger->log_ptr(scope, inst->channel.callback_nb, "callback_nb");

    logger->log_uint(scope, inst->elem_size, "elem_size");
    logger->log_uint(scope, inst->depth, "depth");
    logger->log_ptr(scope, inst->buffer, "buffer");
    logger->log_ptr(scope, inst->error_handler, "error_handler");
    
    logger->exit(scope, &subscope);
}

int channel_consumer_callback(QueueHandle_t queue, const void *data, const int timeout) {
    return xQueueSendToBack(queue, data, timeout);
}

int channel_consumer_callback_nb(QueueHandle_t queue, const void *data) {
    return xQueueSendToBackFromISR(queue, data, NULL);
}