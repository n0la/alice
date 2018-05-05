#include "cmd.h"

cmd_t cmds[] = {
    alice_nickreclaimer,
    alice_login,
    NULL,
};

cmd_queue_t * cmd_queue_new(void)
{
    cmd_queue_t *q = calloc(1, sizeof(cmd_queue_t));

    if (q == NULL) {
        return NULL;
    }

    q->queue = irc_queue_new();
    if (q->queue == NULL) {
        return NULL;
    }

    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);

    return q;
}

void cmd_queue_item_free(cmd_queue_item_t *i)
{
    irc_message_unref(i->message);
    free(i);
}

void cmd_queue_free(cmd_queue_t *q)
{
    if (q == NULL) {
        return;
    }

    pthread_mutex_lock(&q->mutex);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);

    if (q->queue != NULL) {
        irc_queue_clear(q->queue, (free_t)cmd_queue_item_free);
        irc_queue_free(q->queue);
        q->queue = NULL;
    }

    free(q);
}

bool cmd_queue(cmd_queue_t *q, irc_client_t c, irc_message_t m)
{
    cmd_queue_item_t *it = NULL;

    if (q == NULL || c == NULL || m == NULL) {
        return false;
    }

    it = calloc(1, sizeof(cmd_queue_item_t));
    if (it == NULL) {
        return false;
    }

    /* ref the message before handing it to the queue
     */
    irc_message_ref(m);

    it->client = c;
    it->message = m;

    pthread_mutex_lock(&q->mutex);
    irc_queue_push(q->queue, it);
    pthread_cond_broadcast(&q->cond);
    pthread_mutex_unlock(&q->mutex);

    return 0;
}

bool cmd_handle(cmd_queue_t *q, void *arg)
{
    cmd_queue_item_t *it = NULL;
    cmd_t *c = NULL;
    char *s = NULL;
    size_t slen = 0;

    if (q == NULL) {
        return false;
    }

    it = irc_queue_pop(q->queue);
    if (it == NULL) {
        return false;
    }

    irc_message_string(it->message, &s, &slen);
    printf("[%p] %s", pthread_self(), s);
    free(s);

    for (c = cmds; *c != NULL; c++) {
        (*c)(it->client, it->message, arg);
    }

    cmd_queue_item_free(it);
    return true;
}
