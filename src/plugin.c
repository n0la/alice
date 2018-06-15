#include "plugin.h"
#include "log.h"
#include "yamlconfig.h"

static pa_t plugin_registry = NULL;
static pa_t plugin_loaded = NULL;
static yaml_config_t plugins = NULL;

extern alice_plugin_t dice_plugin;
extern alice_plugin_t nickserv_plugin;

void alice_plugin_deinit(void)
{
    alice_plugin_unloadall();
    yaml_config_free(plugins);
    plugins = NULL;
}

bool alice_plugin_init(void)
{
    if (plugins != NULL) {
        return true;
    }

    plugins = yaml_config_new();
    if (plugins == NULL) {
        return false;
    }

    if (!yaml_config_load_file(plugins, ALICE_PLUGINS_CONFIG)) {
        ALICE_ERROR("failed to load plugin configuration file: %s",
                    yaml_config_strerror(plugins)
            );
        goto fail;
    }

    if (!yaml_config_is_mapping(plugins)) {
        ALICE_ERROR("root node of plugins configuration is not a mapping");
        goto fail;
    }

    return true;

fail:

    yaml_config_free(plugins);
    plugins = NULL;
    return false;
}

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

static bool alice_plugin_loadone(alice_plugin_t *p, yaml_config_t config)
{
    void *arg = NULL;
    alice_loaded_t *l = NULL;

    if (plugin_loaded == NULL) {
        plugin_loaded = pa_new_full(plugin_unload);
        if (plugin_loaded == NULL) {
            return false;
        }
    }

    if (p->new != NULL) {
        arg = p->new(config);
        if (arg == NULL) {
            ALICE_ERROR("failed to load plugin %s, skipping", p->name);
            return false;
        }
    }

    l = calloc(1, sizeof(alice_loaded_t));
    if (l == NULL) {
        ALICE_ERROR("oom while loading plugin %s", p->name);
        if (p->free) {
            p->free(arg);
        }
        return false;
    }

    l->arg = arg;
    l->plugin = p;

    pa_add(plugin_loaded, l);
    ALICE_DEBUG("plugin %s successfully loaded", p->name);

    return true;
}

bool alice_plugin_load(void)
{
    char *node = NULL;
    yaml_config_t child = NULL;
    yaml_config_iterator_t it = NULL;
    size_t i = 0;

    if (plugins == NULL) {
        ALICE_WARN("no plugins to load");
        return false;
    }

    while (yaml_config_mapping_next(plugins, &it, &node, &child)) {
        /* find node in the list of our plugins
         */
        for (i = 0; i < plugin_registry->vlen; i++) {
            alice_plugin_t *p = (alice_plugin_t*)plugin_registry->v[i];
            if (strcmp(p->name, node) == 0) {
                alice_plugin_loadone(p, child);
            }
        }

        yaml_config_free(child);
    }
}

int alice_plugin_loadall(void)
{
    size_t i = 0;

    for (i = 0; i < plugin_registry->vlen; i++) {
        alice_plugin_t *p = (alice_plugin_t*)plugin_registry->v[i];
        alice_plugin_loadone(p, NULL);
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
