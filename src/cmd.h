#ifndef ALICE_CMD_H
#define ALICE_CMD_H

#include <irc/irc.h>
#include <irc/client.h>
#include <irc/config.h>
#include <irc/queue.h>

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    irc_queue_t queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} cmd_queue_t;

typedef struct {
    irc_client_t client;
    irc_message_t message;
} cmd_queue_item_t;

typedef int (*cmd_t)(irc_client_t client, irc_message_t m, void *arg);

extern cmd_t cmds[];

int alice_nickreclaimer(irc_client_t client, irc_message_t m, void *arg);
int alice_login(irc_client_t client, irc_message_t m, void *arg);

cmd_queue_t *cmd_queue_new(void);
void cmd_queue_free(cmd_queue_t *q);
void cmd_queue_item_free(cmd_queue_item_t *i);
bool cmd_queue(cmd_queue_t *q, irc_client_t client, irc_message_t m);
bool cmd_handle(cmd_queue_t *q, void *arg);

#endif
