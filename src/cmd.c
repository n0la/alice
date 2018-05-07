#include "cmd.h"

#include <irc/pa.h>
#include <ctype.h>

typedef struct {
    char const *command;
    cmd_handler_t handler;
} cmd_entry_t;

static cmd_entry_t cmds[] = {
    /* NULL means that they take all messages
     */
    { NULL, alice_nickreclaimer },
    { NULL, alice_login },
    /* roll dices
     */
    { "roll", alice_dice },
    { "dice", alice_dice },
    /* end marker
     */
    { NULL, NULL },
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
    cmd_entry_t *c = NULL;
    cmd_t *cmd = NULL;

    if (q == NULL) {
        return false;
    }

    it = irc_queue_pop(q->queue);
    if (it == NULL) {
        return false;
    }

    if (irc_message_is(it->message, IRC_COMMAND_PRIVMSG) &&
        it->message->argslen >= 2) {
        cmd = cmd_parse(it->message->args[1]);
    }

    for (c = cmds; c->handler != NULL; c++) {
        if (cmd == NULL && c->command == NULL) {
            c->handler(it->client, it->message, cmd, arg);
        }

        if (cmd != NULL && cmd->command != NULL && c->command != NULL &&
            strcmp(cmd->command, c->command) == 0) {
            c->handler(it->client, it->message, cmd, arg);
        }
    }

    cmd_queue_item_free(it);
    cmd_free(cmd);

    return true;
}

void cmd_free(cmd_t *c)
{
    size_t i = 0;

    if (c == NULL) {
        return;
    }

    free(c->command);
    c->command = NULL;

    if (c->argv != NULL) {
        for (i = 0; i < c->argc; i++) {
            free(c->argv[i]);
        }
        free(c->argv);
        c->argv = NULL;
    }
}

cmd_t *cmd_parse(char const *message)
{
    size_t len = 0, i = 0;
    cmd_t *cmd = NULL;
    char *cur = NULL;
    size_t curlen = 0;
    FILE *str = NULL;
    pa_t args = NULL;
    bool inquote = false;
    char c = 0, prev = 0;

    enum {
        do_ignore,
        do_add,
        do_finish,
    } what;

    if (message == NULL || strlen(message) < 1 || message[0] != '#') {
        return NULL;
    }

    len = strlen(message);
    cmd = calloc(1, sizeof(cmd_t));
    if (cmd == NULL) {
        return NULL;
    }

    args = pa_new();
    if (args == NULL) {
        free(cmd);
        return NULL;
    }

    /* skip #
     */
    for (i = 1; i < len; i++) {
        prev = c;
        c = message[i];
        what = do_ignore;

        if (str == NULL) {
            str = open_memstream(&cur, &curlen);
        }

        if (c == '"') {
            if (prev == '\\') {
                what = do_add;
            } else {
                inquote = !inquote;
                if (!inquote) {
                    what = do_finish;
                }
            }
        } else if (isspace(c)) {
            what = (inquote ? do_add : do_finish);
            if (what == do_finish && isspace(prev)) {
                what = do_ignore;
            }
        } else {
            what = do_add;
        }

        switch (what) {
        case do_add: fputc(c, str); break;
        case do_finish:
        {
            fclose(str);
            if (cmd->command == NULL) {
                cmd->command = cur;
            } else {
                if (strlen(cur) > 0) {
                    pa_add(args, cur);
                }
            }
            cur = NULL;

            str = open_memstream(&cur, &curlen);
        } break;
        case do_ignore: /* intentional */
        default: break;
        }
    }

    fclose(str);
    if (cmd->command == NULL) {
        cmd->command = cur;
    } else {
        if (strlen(cur) > 0) {
            pa_add(args, cur);
        }
    }
    cur = NULL;

    /* steal pointer
     */
    cmd->argv = (char **)args->v;
    cmd->argc = args->vlen;
    args->v = NULL;
    args->vlen = 0;

    pa_free(args);
    args = NULL;

    return cmd;
}

char *cmd_concat(cmd_t const *cmd)
{
    FILE *s = NULL;
    char *buf = NULL;
    size_t buflen = 0;
    int i = 0;

    if (cmd == NULL) {
        return NULL;
    }

    s = open_memstream(&buf, &buflen);
    if (s == NULL) {
        return NULL;
    }

    for (i = 0; i < cmd->argc; i++) {
        fprintf(s, "%s", cmd->argv[i]);
    }

    fclose(s);
    return buf;
}
