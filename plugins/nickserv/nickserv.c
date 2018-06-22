#include <alice.h>
#include <plugin.h>

typedef struct {
    char *password;
    char *nickserv;
} nickserv_t;

static nickserv_t *nickserv_new(yaml_config_t c)
{
    nickserv_t *n = NULL;
    char const *pass;
    char const *nick;

    if ((n = calloc(1, sizeof(nickserv_t))) == NULL) {
        ALICE_ERR("nickserv: OOM");
        return NULL;
    }

    if (!yaml_config_string(c, "password", &pass, NULL)) {
        ALICE_ERR("nickserv: no password specified in configuration");
        return NULL;
    }

    yaml_config_string(c, "nickserv", &nick, "nickserv");

    n->password = strdup(pass);
    n->nickserv = strdup(nick);

    return n;
}

static void nickserv_free(nickserv_t *n)
{
    if (n == NULL) {
        return;
    }

    free(n->password);
    free(n->nickserv);
    free(n);
}

static int nickserv_login(nickserv_t *n,
                          irc_client_t client, irc_message_t m)
{
    irc_message_t r = NULL;
    irc_t i = irc_client_irc(client);
    char *nick = NULL;

    irc_getopt(i, ircopt_nick, &nick);

    /* look for error MODE on our nick
     */
    if (!(irc_message_is(m, IRC_COMMAND_MODE) &&
          irc_message_arg_is(m, 0, nick))) {
        return 0;
    }

    if (n->nickserv == NULL || n->password == NULL) {
        return 0;
    }

    r = irc_message_privmsg(nick, n->nickserv, "IDENTIFY %s", n->password);
    irc_queue(i, r);

    return 0;
}

static int nickserv_reclaimer(nickserv_t *n,
                              irc_client_t client, irc_message_t m)
{
    irc_message_t r = NULL;
    irc_t i = irc_client_irc(client);
    char const *nickserv = n->nickserv;
    char const *password = n->password;
    char *nick = NULL, *altnick = NULL;

    /* look for error 433
     */
    if (!irc_message_is(m, IRC_ERR_NICKNAMEINUSE)) {
        return 0;
    }

    if (nickserv == NULL || password == NULL) {
        return 0;
    }

    irc_getopt(i, ircopt_nick, &nick);

    /* change nick
     */
    asprintf(&altnick, "%s_alt", nick);
    r = irc_message_make(nick, IRC_COMMAND_NICK, altnick, NULL);
    irc_queue(i, r);

    /* send identify first
     */
    r = irc_message_privmsg(altnick, nickserv, "IDENTIFY %s", password);
    irc_queue(i, r);

    /* now reclaim
     */
    r = irc_message_privmsg(altnick, nickserv, "REGAIN %s %s", nick, password);
    irc_queue(i, r);

    /* and change nick (to be sure)
     */
    r = irc_message_make(altnick, IRC_COMMAND_NICK, nick, NULL);
    irc_queue(i, r);

    free(altnick);
    altnick = NULL;

    return 0;
}

static int nickserv_handle(void *arg, irc_client_t client, irc_message_t m)
{
    nickserv_login((nickserv_t*)arg, client, m);
    nickserv_reclaimer((nickserv_t*)arg, client, m);
}

alice_plugin_t nickserv_plugin = {
    "nickserv",
    (alice_plugin_new)nickserv_new,
    (alice_plugin_free)nickserv_free,
    NULL,
    NULL,
    nickserv_handle
};
