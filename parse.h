#ifndef parse
#define parse

/* Header file for dealing with text processing, error checking,
 * and general i/o */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "frequency.h"

/* Used as the intermediate representation for notes before being 
 * written to outfile */
typedef struct node {
	int frequency;
	double duration;
	struct node *next;
} Note_Node;
/* typedef'd in effects.h; defined here to contain Note_Node */
struct fx_node {
	int name;
	Note_Node param1; 
	int param2;
	int param3;
	int param4;
};

#include "effects.h"

/* the Note_Node struct is also used for temporarily storing the
 * position and associated time mods of parentheses in buffers;
 * these macros provide semantic clarity to that end */
#define INDEX frequency
#define NO_TIME_MODS duration

/* color codes for printing errors and notes */
#define ANSI_RED      "\x1b[1;31m"
#define ANSI_CYAN     "\x1b[36m"	
#define ANSI_BOLD     "\x1b[1m"
#define ANSI_UNDR     "\x1b[4;37m"
#define ANSI_RESET    "\x1b[0m"

#define COMMENT_CHAR    '%'
#define COMMAND_KEYWORD "set"

/* general denotation for error-checking state used by parser */
enum Error_State  {NORMAL, TIME_ERROR, NOTE_ERROR, NAME_ERROR, ARG_ERROR, NO_ARGS_PASSED_ERROR}; 
/* denotes whether a command, note, or error has been found */
enum Parse_Status {COMMAND, NOTE, FAILED, NONE};
/* for handling exit status in main */
enum Exit_Status { NORMAL_EXIT, ERROR_EXIT, HELP_EXIT };

/* linked list helper functions */
void add2start(Note_Node **start, Note_Node *new);
void add2end(Note_Node **start, Note_Node **tail, Note_Node *new);
void traverse(Note_Node *start);
void del_from_end(Note_Node **start, Note_Node **tail);
void free_list(Note_Node **list, Note_Node **tail);

/* Parses the command line invocation of beepcomp, 
 * initializing in and outfiles.
 * Returns: 0 for success, 1 for failure */
unsigned int parse_cmdline(char **cmdline_args, int no_args, FILE **in, FILE **out, char **out_name);

/* Performs a similar function to sscanf(), albeit with less
 * segfault-y behavior. Compares a string and a format.
 * Returns: 0 for match, 1 for no match */
unsigned int frmtcmp(char *str, char *frmt);

/* Validates the argument of a command against its format specified
 * in commands.h 
 * Returns: NORMAL or ARG_ERROR from enum Error_State */
unsigned int argchecker(char *argument, const char *format);

/* Validates a command, in part using argchecker.
 * Returns: NAME_ERROR, ARG_ERROR, or NORMAL */
unsigned int validate_command(char *command, char *argument);

/* Prints the text wherein an error occurs
 * IN: Error line, line number, and string with the flagged error */
void print_error_line(char *line, int line_no, char *error_string);

/* Parses a line from notescript, to include checking syntax and
 * printing relevant arguments to the beep command 
 * Returns: COMMAND, FAILED, or NONE from enum Parse_Status */
unsigned int parse_for_command(char *line, int line_number);

/* Parses a given element in a buffer, identifying a correctly
 * placed effects macro and constructing a more useful 
 * representation. It also alters the element, removing the macro
 * in order to allow it to be validated (returned effect info not
 * used) or converted into an intermediate representation */
Effect_Package parse_effects_macros(char *element);

/* Parses a line buffer for illegal syntax.
 * Returns: status from enum Error_State */
unsigned int validate_buffer(int no_buffer_elements, char **buffer);

/* Parses a line after parse_for_command_or_comment returns NONE. It 
 * applies key signatures, parentheticals, etc.
 * IN: a full line, line number, buffer (passed
 *     by reference), and number of buffer elements (by reference)
 * Returns: FAILED, COMMAND, or NOTE from enum parse_status */
unsigned int get_line_buffer(char *line, int line_number, char ***buffer, int *buffer_elements);

/* Converts the string representation of a note to its intermediate
 * representation, returning it as a Note_Node struct. */
Note_Node *convert_from_string(char *string, Key_Map *keymap, int **freq_table, double tempo);

/* Converts a line buffer to intermediate representations. Handles
 * macro expansion and addition to the list of all notes in a song */
void buffer_to_intrep(char **buffer, int buf_size, Note_Node **start, Note_Node **tail, Key_Map *keymap, int **freq_table, double tempo);

/* converts intermediate representation into a bash 
 * script and writes to outfile. */
void write_to_file(Note_Node *representation, FILE *outfile, Note_Node *tail);

/* handles all input and translation, relying on functions
 * defined above. Note Key_Str, Tempo, and key are passed
 * by reference here */
unsigned int parse_infile(FILE *infile, FILE *outfile, char **Key_Str, int **freq_table, Key_Map **key);
#endif
