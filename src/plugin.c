#include "plugin.h"
#include "log.h"
#include <irc/pa.h>

static pa_t plugin_registry = NULL;
static pa_t plugin_loaded = NULL;

extern alice_plugin_t dice_plugin;
extern alice_plugin_t nickserv_plugin;

int alice_plugin_register_default(void)
{
    alice_plugin_register(&dice_plugin);
    alice_plugin_register(&nickserv_plugin);
}

int alice_plugin_register(alice_plugin_t *p)
{
    if (plugin_registry == NULL) {
        plugin_registry = pa_new();
        if (plugin_registry == NULL) {
            return 1;
        }
    }

    pa_add(plugin_registry, p);
}

static void plugin_unload(void *p)
{
    alice_loaded_t *l = (alice_loaded_t*)p;

    if (p == NULL) {
        return;
    }

    if (l->arg != NULL && l->plugin->free != NULL) {
        l->plugin->free(l->arg);
    }

    free(p);
}

int alice_plugin_unloadall(void)
{
    if (plugin_loaded == NULL) {
        return 0;
    }

    pa_free(plugin_loaded);
    plugin_loaded = NULL;
}

int alice_plugin_loadall(void)
{
    size_t i = 0;

    if (plugin_loaded == NULL) {
        plugin_loaded = pa_new_full(plugin_unload);
        if (plugin_loaded == NULL) {
            return 3;
        }
    }

    for (i = 0; i < plugin_registry->vlen; i++) {
        void *arg = NULL;
        alice_plugin_t *p = (alice_plugin_t*)plugin_registry->v[i];
        alice_loaded_t *l = NULL;

        if (p->new != NULL) {
            arg = p->new();
            if (arg == NULL) {
                ALICE_ERROR("failed to load plugin %s, skipping", p->name);
                continue;
            }
        }

        l = calloc(1, sizeof(alice_loaded_t));
        if (l == NULL) {
            ALICE_ERROR("oom while loading plugin %s", p->name);
            break;
        }

        l->arg = arg;
        l->plugin = p;

        pa_add(plugin_loaded, l);
    }
}

int alice_handle_command(irc_client_t client, cmd_t const *cmd)
{
    size_t i = 0;

    if (plugin_loaded == NULL) {
        return 3;
    }

    for (i = 0; i < plugin_loaded->vlen; i++) {
        alice_loaded_t *p = (alice_loaded_t*)plugin_loaded->v[i];

        if (p->plugin->handles != NULL) {
            if (!p->plugin->handles(cmd->command)) {
                continue;
            }
        }

        if (p->plugin->command != NULL) {
            p->plugin->command(p->arg, client, cmd);
        }
    }

    return 0;
}

int alice_handle_message(irc_client_t client, irc_message_t m)
{
    size_t i = 0;

    if (plugin_loaded == NULL) {
        return 3;
    }


    for (i = 0; i < plugin_loaded->vlen; i++) {
        alice_loaded_t *p = (alice_loaded_t*)plugin_loaded->v[i];

        if (p->plugin->message != NULL) {
            p->plugin->message(p->arg, client, m);
        }
    }

    return 0;
}
