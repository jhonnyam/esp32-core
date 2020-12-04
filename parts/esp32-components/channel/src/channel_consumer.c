#include "channel_consumer.h"
#include "freertos/freeRTOS.h"
#include "freertos/queue.h"

void channel_consumer_init(Lifesensor_module *module) {
    Channel_consumer *inst = (void*) module;
    channel_init(&inst->channel);
    inst->handle = xQueueCreateStatic(
        inst->depth,
        inst->elem_size,
        inst->buffer,
        &inst->queue
    );
}

void channel_consumer_dump(Lifesensor_module *module, Lifesensor_logger *logger, Lifesensor_logger_scope *scope) {
    Channel_consumer *inst = (void *)module;

    Lifesensor_logger_scope subscope;
    Lifesensor_logger_scope subsubscope;
    logger->enter_map(scope, &subscope, "channel");

    logger->log_str(inst->channel.identifier, "identifier");

    logger->enter_map(&subscope, &subsubscope, "same");
    logger->log_ptr(inst->channel.same.prev, "prev");
    logger->log_ptr(inst->channel.same.next, "next");
    logger->exit(&subscope, &subsubscope);

    logger->enter_map(&subscope, &subsubscope, "unique");
    logger->log_ptr(inst->channel.unique.prev, "prev");
    logger->log_ptr(inst->channel.unique.next, "next");
    logger->exit(&subscope, &subsubscope);

    logger->log_ptr(inst->channel.ctx, "ctx");
    logger->log_ptr(inst->channel.callback, "callback");
    logger->log_ptr(inst->channel.callback_nb, "callback_nb");

    logger->log_uint(inst->elem_size, "elem_size");
    logger->log_uint(inst->depth, "depth");
    logger->log_ptr(inst->buffer, "buffer");
    logger->log_ptr(inst->error_handler, "error_handler");
    
    logger->exit(scope, &subscope);
}

int channel_consumer_callback(QueueHandle_t queue, const void *data, const int timeout) {
    int ret = xQueueSendToBack(queue, data, timeout);
    return ret == pdTrue;
}

int channel_consumer_callback_nb(QueueHandle_t queue, const void *data) {
    int wakeHigherPriorityTask = pdFALSE;
    int ret = xQueueSendToBackFromISR(queue, data, NULL);
}