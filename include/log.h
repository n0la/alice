#ifndef ALICE_LOG_H
#define ALICE_LOG_H

#include "alice.h"
#include <syslog.h>

#define ALICE_WARN(fmt, ...)  (syslog(LOG_WARNING, fmt, ##__VA_ARGS__))
#define ALICE_DEBUG(fmt, ...) (syslog(LOG_DEBUG, fmt, ##__VA_ARGS__))
#define ALICE_INFO(fmt, ...)  (syslog(LOG_INFO, fmt, ##__VA_ARGS__))
#define ALICE_ERROR(fmt, ...) (syslog(LOG_ERR, fmt, ##__VA_ARGS__))

#endif
