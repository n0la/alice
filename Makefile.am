bin_PROGRAMS =		alice

alice_SOURCES =		src/alice.c \
			src/cmd.h \
			src/cmd.c \
			src/nickreclaimer.c \
			src/login.c \
			src/dice.c

ACLOCAL_AMFLAGS =	-Im4

alice_CFLAGS =		${IRC_CFLAGS} ${DICE_CFLAGS} ${EVENT_CFLAGS}
alice_LDFLAGS =		${IRC_LIBS} ${DICE_LIBS} ${EVENT_LIBS}
