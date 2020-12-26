#ifndef __CHANNEL_PRODUCER__
#define __CHANNEL_PRODUCER__

#include "channel.h"
#include "module.h"
#include "logger.h"

struct channel_producer {
    Lifesensor_module module;
    Channel channel;
};
typedef struct channel_producer Channel_producer;

void channel_producer_init(Lifesensor_module *module);
void channel_producer_dump(Lifesensor_module *module, Lifesensor_logger *logger, Lifesensor_logger_scope *scope);

#define INIT_CHANNEL_PRODUCER(NAME) \
{   \
    .module = INIT_LIFESENSOR_MODULE(   \
        Channel_producer,   \
        NAME,   \
        &channel_producer_init, \
        &channel_producer_dum   \
    ),  \
    .channel = INIT_CHANNEL(NAME, NULL, NULL, NULL) \
}

#endif
