beepcomp: main.c parse.c frequency.c timing.c commands.c macros.c
	gcc -g -o beepcomp main.c parse.c macros.c frequency.c timing.c commands.c -lm -std=c99
install: beepcomp
	if [ -f /usr/bin/beepcomp ]; then rm /usr/bin/beepcomp; fi
	mv beepcomp /usr/bin
