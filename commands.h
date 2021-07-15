#ifndef commands_include
#define commands_include

/* header for specifying command names and argument formats 
 * and functions carried out by commands */

#include "frequency.h"

/* list of commands the compiler can expect to see, along with 
 * their anticipated arguments. Even indices are command names,
 * odd indices are formats */
static const char *commands[] = {
/*	 command name - argument format */
	   "tempo",       "%f",				
	   "key",         "%s %c",
		 "arprate",     "%i",
/*   end of aray sentinel */
		 "\0"
};

void command_tempo(double new_tempo, double *tempo);

/* Returns: NORMAL or FAILED from enum Parse_Status */
int command_key(char *new_key0, char *new_key1, char *key, Key_Map **keymap);

void command_arprate(int new_arprate, int *arprate);

#endif
