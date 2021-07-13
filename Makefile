beepcomp: main.c parse.c frequency.c timing.c commands.c effects.c
	gcc -o beepcomp main.c parse.c effects.c frequency.c timing.c commands.c -lm -std=c99
install: beepcomp
	[[ -e /usr/bin/beepcomp ]] && rm /usr/bin/beepcomp
	mv beepcomp /usr/bin
gdb_test: main.c parse.c frequency.c timing.c commands.c
	gcc -g -o beepcomp main.c parse.c effects.c frequency.c timing.c commands.c -lm -std=c99
