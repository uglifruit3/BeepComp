#ifndef parse
#define parse
/*******************************************************************
 * Header file for parsing individual notescript lines
 * Works with and relies upon syntax.h
 * Draws semantic and syntactic elements from semantics.h
 *******************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "semantics.h"
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
 * these macros aim to provide semantic clarity to that end */
#define INDEX frequency
#define NO_TIME_MODS duration

/* color codes for printing errors and notes */
#define ANSI_RED      "\x1b[1;31m"
#define ANSI_CYAN     "\x1b[36m"	
#define ANSI_BOLD     "\x1b[1m"
#define ANSI_UNDR     "\x1b[4;37m"
#define ANSI_RESET    "\x1b[0m"

/* linked list helper functions */
void add2start(Note_Node **start, Note_Node *new);
void add2end(Note_Node **start, Note_Node **tail, Note_Node *new);
void traverse(Note_Node *start);
void del_from_end(Note_Node **start, Note_Node **tail);
void free_list(Note_Node **list, Note_Node **tail);

/* Parses the command line invocation of beepcomp, 
 * initializing in and outfiles.
 * IN: argv, arc; infile, outfile, and outfile name
 * (by reference)
 * OUT: 1 for failure, 0 for succes */
unsigned int parse_cmdline(char **cmdline_args, int no_args, FILE **in, FILE **out, char **out_name);

/* Performs a similar function to sscanf(), albeit with less
 * segfault-y behavior. Compares a string and a format.
 * IN: string to be analyzed, string representing format
 * OUT:
 *    0: string matches format
 *    1: string does not match format */
unsigned int frmtcmp(char *str, char *frmt);

/* Validates the argument of a command against its format specified
 * in semantics.h 
 * IN: an argument and format to be compared against 
 * OUT: NORMAL or ARG_ERROR from enum State */
unsigned int argchecker(char *argument, const char *format);

/* Validates a command, in part using argchecker.
 * IN: a command and argument
 * OUT: NAME_ERROR, ARG_ERROR, or NORMAL */
unsigned int validate_command(char *command, char *argument);

/* Prints the text wherein an error occurs
 * IN: Error line, line number, and string with the flagged error */
void print_error_line(char *line, int line_no, char *error_string);

/* Parses a line from notescript, to include checking syntax and
 * printing relevant arguments to the beep command 
 * IN: a full line 
 * OUT: COMMAND, FAILED, or NONE from enum Parse_Status */
unsigned int parse_for_command(char *line, int line_number);

/* Parses a given element in a buffer, identifying a correctly
 * placed effects macro and constructing a more useful 
 * representation. It also alters the element, removing the macro
 * in order to allow it to be validated (returned effect info not
 * used) or converted into an intermediate representation */
Effect_Package parse_effects_macros(char *element);

/* Parses a line buffer for illegal syntax.
 * IN: a buffer and number of elements therein
 * OUT: status from enum State */
unsigned int validate_buffer(int no_buffer_elements, char **buffer);

/* Parses a line after parse_for_command_or_comment returns NONE. It 
 * applies key signatures, parentheticals, etc.
 * IN: a full line, line number, buffer (passed
 *     by reference), and number of buffer elements (by reference)
 * OUT: FAILED, COMMAND, or NOTE from enum parse_status */
unsigned int get_line_buffer(char *line, int line_number, char ***buffer, int *buffer_elements);

/* Converts the string representation of a note to its intermediate
 * representation, using the struct Note_Node.
 * IN: string rep; keymap, frequency table, and tempo in use
 * OUT: a pointer to the note's intermediate representation */
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
