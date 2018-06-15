#ifndef ALICE_H
#define ALICE_H

#define _GNU_SOURCE

#define ALICE_IRC_CONFIG "/etc/alice/irc.conf"
#define ALICE_CONFIG     "/etc/alice/alice.yml"
#define ALICE_PLUGINS_CONFIG "/etc/alice/plugins.yml"

#include <irc/irc.h>
#include <irc/client.h>
#include <irc/config.h>
#include <irc/queue.h>
#include <irc/pa.h>

#include <event2/event.h>

#include <stdbool.h>
#include <stdint.h>

#endif
