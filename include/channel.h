#ifndef __CHANNEL__
#define __CHANNEL__

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

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
 * CHANNEL_INIT - Macro to provide initialization for channel object
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

/**
 * channel_init - helper function to initialize channel object
 * @ch: pointer to channel object to initialize
 * @identifier: identifier string of the channel
 * @ctx: pointer to context of the callback
 * @flags: additional flags for the callback to modify behaviour
 * @callback: function to call on new data  
 */ 
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

/**
 * channel_setContext - helper function to change the context the callback acts on
 * @ch: pointer to channel object to modify
 * @ctx: pointer to new context
 */
static
void
inline __attribute__((always_inline))
channel_setContext 
(Channel * const ch, void * const ctx) {
    ch->ctx = ctx;
}

/**
 * channel_setCallback - helper function to change the callback
 * @ch: pointer to channel object to modify
 * @callback: function to register as callback
 */
static
void 
inline __attribute__((always_inline))
channel_setCallback
(Channel * const ch, const Channel_callback const callback) {
    ch->callback = callback;
}

/**
 * channel_init_broadcast - helper function to initialize position element in broadcast
 * @ch: pointer to channel object
 * @ctx: pointer to position element
 */
static
void
inline __attribute__((always_inline))
channel_init_broadcast
(const Channel * const ch, const Channel ** const pos)
{
    *pos = ch;
}

/**
 * channel_register - register a channel
 * @ch: channel to register
 * 
 * This function adds channel to the message passing structure. 
 * It doesn't make a difference whether this is a message
 * provider or consumer. The difference is only made by the channels
 * callback implementation.
 */
void
channel_register
(Channel * const ch);

/**
 * unregister_channel - removes a channel input or output from the message structure
 * @ch: channel element to remove
 */
void
channel_deregister
(Channel * const ch);

/**
 * channel_broadcast - send a message to all who registered on this channels identifier
 * @ch: the channel structure that holds informations for this channel
 * @tmp: iterator pointer, needs to be initialized by channel_init_broadcast or reused to continue broadcast
 * @data: the pointer to the message
 * @timeout: how many time-slices each callback may block before aborting
 * @return: returns pdPass (=1) if broadcast to was successful, if non-one value is returned, iteration was stopped, because a callback failed 
 * 
 * Iterates over all elements in the same list, that registered on this channels identifier
 * and calls the callback function on it. May return with error_code before all elements were notified, allowing for custom error handling
 * Example for skipping failing callbacks:
 *  Channel **pos;
 *  channel_init_broadcast(ch,pos);
 *  while(!channel_broadcast(ch,pos,data,timeout));
 */
const BaseType_t
channel_broadcast
(Channel * const ch, Channel ** tmp, const void * const data, const TickType_t timeout);

/**
 * channel_send - sends a message to a single channel object
 * @ch: the channel structure that holds informations for this channel
 * @data: the pointer to the message
 * @timeout: how many time-slices the callback may block before aborting
 * @return: returns pdPass (=1) if send was successful, if non-one value is returned, callback failed
 * 
 * Calls the callback function on the channel.
 */
const BaseType_t
channel_send
(Channel * const ch, const void * const data, const TickType_t timeout, const BaseType_t flags);

#endif
