#include "channel.h"

static CHANNEL(root, "", NULL, 0, NULL);

/**
 * channel_register - register a channel
 * @channel: channel to register
 * 
 * This function adds channel to the message passing structure. 
 * It doesn't make a difference whether this is a message
 * provider or consumer. The difference is only made by the channels
 * callback implementation.
 */
void 
channel_register
(Channel *ch) 
{
    Channel *curr;
    list_for_each_entry(curr, &root.unique, unique) {
        if (strcmp(ch->identifier, curr->identifier) == 0) {
            list_add(&ch->same, &curr->same);
            return;
        }
    }

    list_add(&ch->unique, &root.unique);
}

/**
 * channel_send - send a message to all who registered on this channels identifier
 * @ch: the channel structure that holds informations for this channel
 * @data: the messages content
 * 
 * Iterates over all elements in the same list, that registered on this channels identifier
 * and calls the push callback function on it. 
 */
const BaseType_t
channel_broadcast
(Channel * const ch, Channel ** tmp, const void * const data, const TickType_t timeout)
{
    Channel *curr = *tmp;
    list_for_each_entry_continue(curr, &ch->same, same) {
        if (curr->callback) {
            BaseType_t status = curr->callback(curr->ctx, data, timeout, curr->flags);
            if (!status) {
                *tmp = curr;
                return status;
            };
        }
    }
    return pdPass;
}

const BaseType_t
channel_send
(Channel * const ch, const void * const data, const TickType_t timeout, const BaseType_t flags)
{
    if (!ch->callback) {
        return pdPass;
    }
    return ch->callback(ch->ctx, data, timeout, flags);
}

/**
 * unregister_channel - removes a channel input or output from the message structure
 * @ch: channel element to remove
 */
void 
channel_unregister
(Channel *ch) 
{
    int unique = ! list_empty(&ch->unique);
    int same   = ! list_empty(&ch->same);

    //channel is not registered
    if (!unique && !same) {
        return;
    }
    //channel is not in the unique chain so just delete it
    if (!unique && same) {
        list_del(&ch->same);
        return;
    }
    //channel is in the unique list, but single element so also just delete it
    if (unique && !same) {
        list_del(&ch->unique);
        return;
    }
    //channel is in unique list and there are others in same list, so replace channel
    if (unique && same) {
        list_add(&ch->unique, &list_entry(ch->same.next, struct channel, same)->unique);
        list_del(&ch->unique);
        list_del(&ch->same);
        return;
    }
}
