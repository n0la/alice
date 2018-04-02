#include <stdlib.h>
#include <stdio.h>

#include <event2/event.h>

#include <irc/irc.h>
#include <irc/client.h>
#include <irc/config.h>

#include "alice.h"

static irc_config_t irc_config = NULL;
static pa_t clients = NULL;
static bool done = false;
struct event_base *base = NULL;

void cleanup(void)
{
    if (irc_config != NULL) {
        irc_config_free(irc_config);
    }

    if (clients != NULL) {
        pa_free(clients);
    }

    event_base_free(base);
}

static void irc_handler(irc_t irc, irc_message_t m, void *data)
{
    irc_client_t c = (irc_client_t)data;
    char *str = NULL;
    size_t strlen = 0;

    if (irc_message_is(m, IRC_COMMAND_PRIVMSG)) {
        /* TODO
         */
    }

    if (irc_message_string(m, &str, &strlen) == irc_error_success) {
        printf("%s", str);
        free(str);
    }
}

static void network_handler(evutil_socket_t s, short what, void *data)
{
    irc_client_t c = (irc_client_t)data;
    irc_t irc = irc_client_irc(c);
    int ret = 0;

    if ((what & EV_READ) == EV_READ) {
        char buf[100] = {0};

        ret = irc_client_read(c, buf, sizeof(buf));
        if (ret > 0) {
            irc_feed(irc, buf, ret);
        }
    }

    if ((what & EV_READ) == EV_WRITE) {
        char *message = NULL;
        size_t len = 0;

        if (irc_pop(irc, &message, &len) == irc_error_success) {
            ret = irc_client_write(c, message, len);
        }

        free(message);
    }

    if (ret >= 0) {
        /* attach the event again.
         */
        struct event *ev = NULL;

        ev = event_new(base, s, EV_READ | EV_WRITE, network_handler, c);
        if (ev == NULL) {
            return;
        }

        event_add(ev, NULL);
    }
}

int main(int ac, char **av)
{
    irc_error_t ret = irc_error_success;
    pa_t networks = NULL;
    int i = 0;

    atexit(cleanup);

    clients = pa_new_full((free_t)irc_client_free);
    if (clients == NULL) {
        return 3;
    }

    irc_config = irc_config_new();
    if (irc_config == NULL) {
        return 3;
    }

    ret = irc_config_load_file(irc_config, ALICE_IRC_CONFIG);
    if (ret != irc_error_success) {
        fprintf(stderr, "failed to load IRC config file: %s\n",
                irc_config_error_string(irc_config));
        return 3;
    }

    networks = irc_config_networks(irc_config);
    if (networks == NULL || networks->vlen == 0) {
        fprintf(stderr, "no networks found in IRC config file\n");
        return 3;
    }

    base = event_base_new();
    if (base == NULL) {
        return 3;
    }

    for (i = 0; i < networks->vlen; i++) {
        irc_config_network_t conf = (irc_config_network_t)networks->v[i];
        irc_client_t c = NULL;
        irc_t irc = NULL;

        c = irc_client_new_config(conf);
        if (c == NULL) {
            return 3;
        }

        irc = irc_client_irc(c);
        irc_handler_add(irc, NULL, irc_handler, c);

        pa_add(clients, c);
    }

    while (!done) {
        for (i = 0; i < clients->vlen; i++) {
            irc_client_t c = clients->v[i];

            if (!irc_client_connected(c) &&
                irc_client_connect(c) == irc_error_success) {
                struct event *ev = NULL;

                ev = event_new(base, irc_client_socket(c),
                               EV_READ | EV_WRITE,
                               network_handler,
                               c
                    );
                if (ev == NULL) {
                    return 3;
                }

                event_add(ev, NULL);
            }

            if (irc_client_connected(c)) {
                irc_t irc = irc_client_irc(c);

                irc_think(irc);
            }
        }

        /* loop once
         */
        ret = event_base_loop(base, EVLOOP_ONCE | EVLOOP_NONBLOCK);
        if (ret < 0) {
            break;
        } else if (ret > 1) {
            usleep(10 * 1000);
        }
    }

    return 0;
}
