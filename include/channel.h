#ifndef __CHANNEL__
#define __CHANNEL__

#include <string.h>

// #include <freertos/FreeRTOS.h>
// #include <freertos/queue.h>
typedef int BaseType_t;
typedef int TickType_t;
#define pdPass 1;
void xQueueGenericSend(void *ctx, const void * const data, TickType_t timeout, const BaseType_t flags);

#include "list.h"

typedef BaseType_t (*Channel_callback) (void *ctx, const void * const data, TickType_t timeout, const BaseType_t flags);

typedef struct channel {
    const char *identifier;
    struct list_head same;
    struct list_head unique;
    void *ctx;
    BaseType_t flags;
    Channel_callback callback;
} Channel;

#ifndef STR
#define STR(X) _STR(X)
#define _STR(X) #X
#endif

/**
 * CHANNEL_INIT - Macro to provide initialization code for channel object
 */
#define CHANNEL_INIT(name, identifier, ctx, flags, callback) { "" identifier "", LIST_HEAD_INIT(name.same), LIST_HEAD_INIT(name.unique), ctx, flags, callback }

/**
 * CHANNEL - Macro to create and initialize channel object
 */
#define CHANNEL(name, identifier, ctx, flags, callback) Channel name = CHANNEL_INIT(name, "" identifier "", ctx, flags, callback)

/*
 * OUTPUT - Macro to create and initialize channel object used by producer
 */
#define OUTPUT(name) CHANNEL(name, STR(name), NULL, 0, NULL)

/*
 * INPUT - Macro to create and initialize channel object used by consumer
 */
#define INPUT(name, ctx, flags, callback) CHANNEL(name, STR(name), ctx, flags, callback)

/*
 * INPUT_QUEUE - Macro to create and initialize channel object used by consumer for queues
 */
#define INPUT_QUEUE(name, queue) CHANNEL(name, STR(name), queue, queueSEND_TO_BACK, &xQueueGenericSend)

static
void
inline __attribute__((always_inline))
channel_init 
(Channel * const ch, const char * const identifier, void * const ctx, const BaseType_t flags, Channel_callback callback) {
    ch->identifier = identifier;
    INIT_LIST_HEAD(&ch->unique);
    INIT_LIST_HEAD(&ch->same);
    ch->ctx = ctx;
    ch->flags = flags;
    ch->callback = callback;
}

static
void
inline __attribute__((always_inline))
channel_setContext 
(Channel * const ch, void * const ctx) {
    ch->ctx = ctx;
}

static
void 
inline __attribute__((always_inline))
channel_setCallback
(Channel * const ch, const Channel_callback const callback) {
    ch->callback = callback;
}

static
void
inline __attribute__((always_inline))
channel_init_broadcast
(const Channel * const ch, const Channel ** const tmp)
{
    *tmp = ch;
}

void channel_register(Channel * const ch);
void channel_deregister(Channel * const ch);

const BaseType_t channel_broadcast(Channel * const ch, Channel ** tmp, const void * const data, const TickType_t timeout);

#endif
