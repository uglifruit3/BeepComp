CC = gcc
CFLAGS = -g -lm -std=c99 -Wpedantic
DEPS = commands.h frequency.h macros.h parse.h timing.h
OBJS = main.o parse.o macros.o frequency.o timing.o commands.o
BIN = beepcomp
INSTALLPATH = /usr/bin/

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

beepcomp: $(OBJS)
	$(CC) -o $(BIN) $^ $(CFLAGS)

install: $(BIN)
	if [ -f $(INSTALLPATH)$(BIN) ]; then rm $(INSTALLPATH)$(BIN); fi
	mv $(BIN) $(INSTALLPATH)

clean:
	rm -f *.o
