#include "alice.h"
#include "cmd.h"

int alice_login(irc_client_t client, irc_message_t m,
                cmd_t const *cmd, void *arg)
{
    irc_message_t r = NULL;
    irc_config_network_t c = irc_client_config(client);
    irc_t i = irc_client_irc(client);
    char const *nickserv = irc_config_network_nickserv(c);
    char const *password = irc_config_network_nickserv_password(c);
    char *nick = NULL;

    irc_getopt(i, ircopt_nick, &nick);

    /* look for error MODE on our nick
     */
    if (!(irc_message_is(m, IRC_COMMAND_MODE) &&
          irc_message_arg_is(m, 0, nick))) {
        return 0;
    }

    if (nickserv == NULL || password == NULL) {
        return 0;
    }

    r = irc_message_privmsg(nick, nickserv, "IDENTIFY %s", password);
    irc_queue(i, r);

    return 0;
}
