
PREFIX ?= /usr/local

SRC = main.c
OBJS = ${SRC:%.c=%.c.o}

CFLAGS = -Ofast
LIBS = -lX11 -lXfixes

XNIGHTLIGHT = xnightlight

CC ?= clang

%.c.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

${XNIGHTLIGHT}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIBS} -o $@

install: ${XNIGHTLIGHT}
	cp ${XNIGHTLIGHT} ${PREFIX}/bin/${XNIGHTLIGHT}

uninstall:
	rm -f ${PREFIX}/bin/${XNIGHTLIGHT}
