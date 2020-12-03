#include "channel.h"

#include <string.h>

void 
channel_init
(
        Channel *ch, 
        const char *identifier, 
        void *ctx, 
        Channel_callback callback,
        Channel_callback_nb callback_nb
)
{
    ch->identifier = identifier;
    INIT_LIST_HEAD(&ch->unique);
    INIT_LIST_HEAD(&ch->same);
    ch->ctx = ctx;
    ch->callback = callback;
    ch->callback_nb = callback_nb;
}

void 
channel_register
(Channel *root, Channel *ch)
{
    Channel *curr;
    list_for_each_entry(curr, &root->unique, unique)
    {
        if (strcmp(ch->identifier, curr->identifier) == 0)
        {
            list_add(&ch->same, &curr->same);
            return;
        }
    }

    list_add(&ch->unique, &root->unique);
}

void 
channel_unregister
(Channel *ch)
{
    int is_unique = !list_empty(&ch->unique);
    int is_same = !list_empty(&ch->same);

    //channel is not registered
    if (!is_unique && !is_same)
    {
        return;
    }
    //channel is not in the unique chain so just delete it
    if (!is_unique && is_same)
    {
        list_del(&ch->same);
        INIT_LIST_HEAD(&ch->same);
        return;
    }

    //channel is in the unique list, but single element so also just delete it
    if (is_unique && !is_same)
    {
        list_del(&ch->unique);
        INIT_LIST_HEAD(&ch->unique);
        return;
    }

    //channel is in unique list and there are others in same list, so replace channel
    if (is_unique && is_same)
    {
        Channel *next_same = list_entry(ch->same.next, struct channel, same);
        Channel *next_uniq = list_entry(ch->unique.next, struct channel, unique);
        list_del(&ch->same);
        INIT_LIST_HEAD(&ch->same);
        list_del(&ch->unique);
        INIT_LIST_HEAD(&ch->unique);
        list_add(&next_same->unique, &next_uniq->unique);
        return;
    }
}

int 
channel_broadcast
(Channel *ch, void *data, const int timeout, int (*get_time)(void))
{
    int start = get_time();
    int remaining = timeout;
    Channel *pos = NULL;
    list_for_each_entry_continue(pos, (&ch->same), same)
    {
        if (remaining <= 0)
        {
            pos = pos->same.prev;
            break;
        }
        if (!pos->callback)
        {
            continue;
        }
        remaining = timeout - (get_time() - start);
        pos->callback(pos->ctx, data, remaining);
    }
    if (pos == ch)
    {
        return remaining;
    }

    list_for_each_entry_continue(pos, (&ch->same), same)
    {
        if (!pos->callback_nb)
        {
            continue;
        }
        pos->callback_nb(pos->ctx, data);
    }

    remaining = timeout - (get_time() - start);
    return remaining;
}

int
channel_broadcast_nb
(Channel *ch, void *data)
{
    Channel *pos = NULL;
    list_for_each_entry_continue(pos, (&ch->same), same)
    {
        if (!pos->callback_nb)
        {
            continue;
        }
        pos->callback_nb(pos->ctx, data);
    }
}
