#ifndef semantics
#define semantics
/*******************************************************************
 * Header file for all things semantics related
 * Designed to provide a comprehensive list of allowed keywords,
 * 	command options, characters, etc
 *******************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define COMMENT_CHAR '%'

/* denotes the command keyword, which tells the compiler to expect
 * a command from the given list of commands */
#define COMMAND_KEYWORD		"set"

/* General denotation of the error-checking state;
 * State is used for arg checker, Parse_Status is more general and
 * is dealt with in main */
enum State 					{NORMAL, TIME_ERROR, NOTE_ERROR, NAME_ERROR, ARG_ERROR}; 
enum Parse_Status 	{COMMAND, NOTE, FAILED, NONE};

/* list of commands the compiler can expect to see, along with 
 * their anticipated arguments. Even indices are command names,
 * odd indices are formats */
static const char *commands[] = {
/*	 command name - argument format */
	   "tempo",       "%f",				
	   "key",         "%s %c",
/*   end of aray sentinel */
		 "\0"
};

static const char accidental_modifiers[] = {
	'#', 'b', 'n'
};
static const char timing_modifiers[] = {
	'o', // whole note
	',', // half note
	'^', // eight note (concatenated for further subdivisions)
	'.'  // dot (concatenated for further subdivisions)
};

#endif
