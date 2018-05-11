#ifndef ALICE_LOG_H
#define ALICE_LOG_H

#include "alice.h"
#include <syslog.h>

#define ALICE_WARN(fmt, args...)  (syslog(LOG_WARNING, fmt, args))
#define ALICE_DEBUG(fmt, args...) (syslog(LOG_DEBUG, fmt, args))
#define ALICE_INFO(fmt, args...)  (syslog(LOG_INFO, fmt, args))
#define ALICE_ERROR(fmt, args...) (syslog(LOG_ERR, fmt, args))

#endif
