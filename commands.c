#include <string.h>

#include "commands.h"
#include "timing.h"
#include "frequency.h"
#include "parse.h"

void command_tempo(double new_tempo, double *tempo) {
	*tempo = new_tempo;
}

int command_key(char *new_key0, char *new_key1, char *key, Key_Map **keymap) {
	free(*keymap); 
	char new_key[5];
	memset(new_key, '\0', 5);
	char space[2] = " \0";
	strcat(new_key, new_key0); strcat(new_key, space); strcat(new_key, new_key1);
	strcpy(key, new_key);
	void *stat = gen_key_sig(key);
	if     ( *((int *) stat) == 1 ) { 
		free(stat);
		return FAILED; }
	else if( *((int *) stat) == 0 ) {
		free(stat);
		*keymap = malloc(1*sizeof(Key_Map));
		keymap[0]->name = NULL;
	} else {
		*keymap = ((Key_Map *) stat);
	}
	return NORMAL;
}

void command_arprate(int new_arprate, int *arprate) {
	*arprate = new_arprate;
}

void command_staccato(double new_staccato, double *staccato) {
	*staccato = new_staccato;
}
