#ifndef ALICE_PLUGIN_H
#define ALICE_PLUGIN_H

#include "alice.h"
#include "cmd.h"
#include "yamlconfig.h"
#include <stdbool.h>

typedef void * (*alice_plugin_new)(yaml_config_t c);
typedef void (*alice_plugin_free)(void*);
typedef bool (*alice_plugin_handles)(char const *cmd);
typedef int (*alice_plugin_command)(void*, irc_client_t, cmd_t const *);
typedef int (*alice_plugin_message)(void *, irc_client_t, irc_message_t);

typedef struct {
    char const *name;
    alice_plugin_new new;
    alice_plugin_free free;
    alice_plugin_handles handles;
    alice_plugin_command command;
    alice_plugin_message message;
} alice_plugin_t;

typedef struct {
    void *arg;
    alice_plugin_t *plugin;
} alice_loaded_t;

bool alice_plugin_init(void);
void alice_plugin_deinit(void);
bool alice_plugin_load(void);

int alice_plugin_register(alice_plugin_t *p);
int alice_plugin_register_default(void);
int alice_plugin_unloadall(void);
int alice_plugin_loadall(void);

int alice_handle_command(irc_client_t client, cmd_t const *cmd);
int alice_handle_message(irc_client_t client, irc_message_t m);

#endif
