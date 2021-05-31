#include <string.h>

#include "commands.h"
#include "timing.h"
#include "frequency.h"
#include "semantics.h"

void command_tempo(double new_tempo, double *tempo) {
	*tempo = new_tempo;
}

void command_time(char *new_time, char *time) {
	strncpy(time, new_time, 128);
}

int command_key(char *new_key0, char *new_key1, char *key, Key_Map **keymap) {
	char new_key[5];
	memset(new_key, '\0', 5);
	char space[2] = " \0";
	strcat(new_key, new_key0); strcat(new_key, space); strcat(new_key, new_key1);
	strcpy(key, new_key);
	void *stat = gen_key_sig(key);
	if     ( *((int *) stat) == 1 ) { free(stat); return FAILED; }
	else if( *((int *) stat) == 0 ) {
		free(stat);
		*keymap = malloc(1*sizeof(Key_Map));
		keymap[0]->name = NULL;
	} else {
		*keymap = ((Key_Map *) stat);
	}
	return NORMAL;
}
