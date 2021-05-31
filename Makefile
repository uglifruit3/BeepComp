beepcomp: main.c parse.c frequency.c timing.c commands.c
	gcc -o beepcomp main.c parse.c frequency.c timing.c commands.c -lm -std=c99
install: beepcomp
	[[ -e /usr/bin/beepcomp ]] && rm /usr/bin/beepcomp
	mv beepcomp /usr/bin
debug: debug.c parse.c frequency.c timing.c commands.c
	gcc -g -o debug.c parse.c frequency.c timing.c commands.c -lm -std=c99
gdb_test: main.c parse.c frequency.c timing.c commands.c
	gcc -g -o beepcomp main.c parse.c frequency.c timing.c commands.c -lm -std=c99
