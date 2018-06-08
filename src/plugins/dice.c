#include <alice.h>
#include <plugin.h>

#include <dice.h>

static bool dice_handles(char const *cmd)
{
    return (strcmp(cmd, "roll") == 0 ||
            strcmp(cmd, "dice") == 0);
}

static int dice_perform(void *arg, irc_client_t client, cmd_t const *cmd)
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

    reply = irc_message_privmsg(nick, cmd->sender, "%s: %ld", argstr, result);
    irc_queue(irc_client_irc(client), reply);

end:

    dice_expression_free(expr);
    free(argstr);

    return 0;
}

alice_plugin_t dice_plugin = {
    "dice",
    NULL,
    NULL,
    dice_handles,
    dice_perform,
    NULL
};
