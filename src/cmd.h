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

typedef struct {
    char *command;
    char **argv;
    int argc;
} cmd_t;

typedef int (*cmd_handler_t)(irc_client_t client, irc_message_t m,
                             cmd_t const *cmd, void *arg);

/* reclaims nick name if 443 error happens.
 */
int alice_nickreclaimer(irc_client_t client, irc_message_t m,
                        cmd_t const *cmd, void *arg);
/* logs in using nickserv
 */
int alice_login(irc_client_t client, irc_message_t m,
                cmd_t const *cmd, void *arg);
/* rolls dice
 */
int alice_dice(irc_client_t client, irc_message_t m,
               cmd_t const *cmd, void *arg);

void cmd_free(cmd_t *cmd);
cmd_t *cmd_parse(char const *message);
char *cmd_concat(cmd_t const *cmd);

cmd_queue_t *cmd_queue_new(void);
void cmd_queue_free(cmd_queue_t *q);
void cmd_queue_item_free(cmd_queue_item_t *i);
bool cmd_queue(cmd_queue_t *q, irc_client_t client, irc_message_t m);
bool cmd_handle(cmd_queue_t *q, void *arg);

#endif
