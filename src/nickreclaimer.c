#include "alice.h"
#include "cmd.h"

int alice_nickreclaimer(irc_client_t client, irc_message_t m,
                        cmd_t const *cmd, void *arg)
{
    irc_message_t r = NULL;
    irc_config_network_t c = irc_client_config(client);
    irc_t i = irc_client_irc(client);
    char const *nickserv = irc_config_network_nickserv(c);
    char const *password = irc_config_network_nickserv_password(c);
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
