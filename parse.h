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
struct node {
	double frequency;
	double duration;
	struct node *next;
}; 
typedef struct node Note_Node;

/* linked list helper functions */
void add2start(Note_Node **start, Note_Node *new);
void add2end(Note_Node **start, Note_Node **tail, Note_Node *new);
void traverse(Note_Node *start);
void free_list(Note_Node **list, int elements);

/* Parses the command line invocation of beepcomp, 
 * initializing in and outfiles.
 * IN: argv, arc; infile, outfile, and outfile name
 * (by reference)
 * OUT: 1 for failure, 0 for succes */
int parse_cmdline(char **cmdline_args, int no_args, FILE **in, FILE **out, char **out_name);

/* Performs a similar function to sscanf(), albeit with less
 * segfault-y behavior. Compares a string and a format.
 * IN: string to be analyzed, string representing format
 * OUT:
 *    0: string matches format
 *    1: string does not match format */
int frmtcmp(char *str, char *frmt);

/* Validates the argument of a command against its format specified
 * in semantics.h 
 * IN: an argument and format to be compared against 
 * OUT: NORMAL or ARG_ERROR from enum State */
int argchecker(char *argument, const char *format);

/* Validates a command, in part using argchecker.
 * IN: a command and argument
 * OUT: NAME_ERROR, ARG_ERROR, or NORMAL */
int validate_command(char *command, char *argument);

/* Prints the text wherein an error occurs, with the following format:
 * [line:column] text ERROR
 *                    ^~~~~
 * IN: Error line, line number, and string with the flagged error */
void print_error_line(char *line, int line_no, char *error_string);

/* Parses a line from notescript, to include checking syntax and
 * printing relevant arguments to the beep command 
 * IN: a full line 
 * OUT: COMMAND, FAILED, or NONE from enum Parse_Status */
int parse_for_command(char *line, int line_number);

/* Parses a line buffer for illegal syntax.
 * IN: a buffer and number of elements therein
 * OUT: status from enum State */
int validate_buffer(int no_buffer_elements, char **buffer);

/* Parses a line after parse_for_command_or_comment returns NONE. It 
 * applies key signatures, parentheticals, etc.
 * IN: a full line, line number, buffer (passed
 *     by reference), and number of buffer elements (by reference)
 * OUT: FAILED, COMMAND, or NOTE from enum parse_status */
int get_line_buffer(char *line, int line_number, char ***buffer, int *buffer_elements);

/* Converts the string representation of a note to its intermediate
 * representation, using the struct Note_Node.
 * IN: string rep; keymap, frequency table, and tempo in use
 * OUT: a pointer to the note's intermediate representation */
Note_Node *convert_from_string(char *string, Key_Map *keymap, double **freq_table, double tempo);

void alt_write_to_file(Note_Node *representation, FILE *outfile, int elements);
#endif
