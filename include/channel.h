#ifndef __CHANNEL__
#define __CHANNEL__

#include <string.h>

#include "list.h"

typedef enum {
    //TODO: include freertos queue codes
    PUSH_OK, PUSH_ERROR, PUSH_BLOCK
} callback_status;

struct channel {
    const char *name;
    struct list_head same;
    struct list_head unique;
    void *ctx;
    callback_status (*callback) (void *ctx, void *data);
};

#ifndef STR
#define STR(X) _STR(X)
#define _STR(X) #X
#endif

/**
 * CHANNEL_INIT - Macro to provide initialization code for channel object
 */
#define CHANNEL_INIT(name, identifier, ctx, callback) { "" identifier "", LIST_HEAD_INIT(name.same), LIST_HEAD_INIT(name.unique), ctx, callback }

/**
 * CHANNEL - Macro to create and initialize channel object
 */
#define CHANNEL(name, identifier, ctx, callback) struct channel name = CHANNEL_INIT(name, "" identifier "", ctx, callback)

/*
 * OUTPUT - Macro to create and initialize channel object used by producer
 */
#define OUTPUT(name) CHANNEL(name, STR(name), NULL, NULL)

/*
 * INPUT - Macro to create and initialize channel object used by consumer
 */
#define INPUT(name, ctx, callback) CHANNEL(name, STR(name), ctx, callback)

/*
 * INPUT_QUEUE - Macro to create and initialize channel object used by consumer for queues
 */
#define INPUT_QUEUE(name, queue) CHANNEL(name, STR(name), queue, &channel_callback_queueSendToBack)

//TODO: implement callbacks
callback_status channel_callback_queueSendToBack(void *ctx, void *data);
callback_status channel_callback_queueSendToFront(void *ctx, void *data);
callback_status channel_callback_queueOverwrite(void *ctx, void *data);

void channel_register(struct channel *ch);
void channel_deregister(struct channel *ch);

void channel_send(struct channel *ch, void *data);

#endif
