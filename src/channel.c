#include "channel.h"

static CHANNEL(root, "", NULL, NULL);

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
(struct channel *ch) 
{
    struct channel *curr;
    list_for_each_entry(curr, &root.unique, unique) {
        if (strcmp(ch->name, curr->name) == 0) {
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
void 
channel_send
(struct channel *ch, void *data) 
{
    struct channel *curr;
    list_for_each_entry(curr, &ch->same, same) {
        if (curr->callback) {
            curr->callback(curr->ctx, data);
        }
    }
}

/**
 * unregister_channel - removes a channel input or output from the message structure
 * @ch: channel element to remove
 */
void 
channel_unregister
(struct channel *ch) 
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
