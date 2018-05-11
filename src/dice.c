#include "alice.h"
#include "cmd.h"

#include <dice.h>

int alice_dice(irc_client_t client, irc_message_t m,
               cmd_t const *cmd, void *arg)
{
    char *argstr = cmd_concat(cmd);
    dice_expression_t expr = NULL;
    irc_message_t reply = NULL;
    int64_t result = 0;
    int error = 0;
    char *nick = NULL;

    irc_getopt(irc_client_irc(client), ircopt_nick, &nick);

    if (argstr == NULL) {
        goto end;
    }

    expr = dice_expression_new();
    if (expr == NULL) {
        goto end;
    }

    if (!dice_expression_parse(expr, argstr, &error)) {
        goto end;
    }

    if (!dice_expression_roll(expr, &result)) {
        goto end;
    }

    reply = irc_message_privmsg(nick, m->args[0], "%s: %ld", argstr, result);
    irc_queue(irc_client_irc(client), reply);

end:

    dice_expression_free(expr);
    free(argstr);

    return 0;
}
