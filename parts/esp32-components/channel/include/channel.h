// Channels are objects which enable iteration over other Channel objects with
// the same name/identifier.
// This implementation resembles a map, mapping identifiers to lists. 
// Channels are implemented as an item of two separate lists.
// The "unique" list holds items with different identifiers,
// the "same" list holds items that share the same identifier. 
// Each list item itself is a valid list containing only itself;
// multiple items can be combined to a larger list.
// 
// This implementation constructs a single valid "unique" list,
// starting with a globally defined root node. All "same" lists of all items
// are considered valid.
//
// Adding to this structure implies iterating the "unique" list, until either
// the same identifier is found (and the new item is added to the "same" list
// of the found item) or the end of the "unique" list reached (and it's
// appended as new "unique" list item).
//
// Each added Channel object is now part of a "same" list which can be directly
// iterated to find other items.
//
// exemplary structure:
// 
//              root node---.  O        O
//                          v  |        |
// unique identifier list-> O--O--O--O--O  
//                             |     |
//                             O     O
//                             ^
//      same identifier list---'                    
//
// Each Channel object contains a context pointer, two callback functions (blocking/nonblocking) and
// an additional flags field. A broadcast function is provided to iterate
// through all "same" list items of a Channel object. The callbacks are issued
// with a data pointer and a timeout value (if blocking) by the caller and the callee
// specific context and flags. This interface is intended to enable dynamic
// management of FreeRTOS queues, where a queue can be registered as a context
// of a Channel and therefore fed by the Channels callback.

#ifndef __CHANNEL__
#define __CHANNEL__

#include "list.h"

typedef int (*Channel_callback) (void *ctx, const void *data, const int timeout);
typedef int (*Channel_callback_nb) (void *ctx, const void *data);

struct channel {
    const char *identifier;
    struct list_head same;
    struct list_head unique;
    void *ctx;
    Channel_callback callback;
    Channel_callback_nb callback_nb;
};

typedef struct channel Channel;


#define INIT_CHANNEL(NAME, CTX, CALLBACK, CALLBACK_NB) \
{ \
    .identifier  = (NAME),\
    .same        = { 0 },\
    .unique      = { 0 },\
    .ctx         = (CTX),\
    .callback    = (CALLBACK),\
    .callback_nb = (CALLBACK_NB),\
}

/**
 * channel_init - initialize channel object
 * @ch: pointer to channel object to initialize
 * @identifier: identifier string of the channel
 * @ctx: pointer to context of the callback
 * @callback: function to call on new data  
 * @callback_nb: like callback but non blocking thus can be called from ISR
 */ 
void
channel_init 
(Channel *ch);

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
(Channel *ch);

/**
 * channel_unregister - removes a channel input or output from the message structure
 * @ch: channel element to unregister
 */
void 
channel_unregister
(Channel *ch);

/**
 * channel_broadcast - send a message to all who registered on this channels identifier
 * @ch: the channel to act on
 * @data: the data that should be send to all channels
 * @timeout: the overall time the broadcast is allowed to block for sending to channels
 * @get_time: the function to determine the current time to calculate elapsed time of a send operation
 * @return: returns remaining time in regard to timeout
 * 
 * Iterates over all elements in the same list, that registered on this channels identifier
 * and calls the callback function on it. If timeout is exceeded continues operation on
 * remaining channels using the non-blocking callback.
 */
int
channel_broadcast
(Channel *ch, void *data, const int timeout, int (* get_time)(void));

/**
 * channel_broadcast_nb - non-blocking broadcast version that can be called from ISRs
 * @ch: the channel to act on
 * @data: the data that should be send to registered channels
 * 
 * Iterates over all elements in the same list, that registered on this channels identifier
 * and calls the non-blocking callback function on it.
 */
void
channel_broadcast_nb
(Channel *ch, void *data);

#endif
