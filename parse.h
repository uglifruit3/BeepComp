#ifndef parse
#define parse

/* Header file for dealing with text processing, error checking,
 * and general i/o */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "frequency.h"

typedef struct node {
	int frequency;
	double duration;
	struct node *next;
} Note_Node;

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
#define INDEX        frequency
#define NO_TIME_MODS duration

#define ANSI_RED   "\x1b[1;31m"
#define ANSI_CYAN  "\x1b[36m"	
#define ANSI_BOLD  "\x1b[1m"
#define ANSI_UNDR  "\x1b[4;37m"
#define ANSI_RESET "\x1b[0m"

#define TRUE  1
#define FALSE 0

#define COMMENT_CHAR      '%'
#define ARP_MACRO_CHAR    '@'
#define CUS_MACRO_CHAR    '$'
#define COMMAND_KEYWORD   "set"
#define MACRO_DEF_KEYWORD "define"

enum Error_State  { NORMAL, TIME_ERROR, NOTE_ERROR, NAME_ERROR, ARG_ERROR, NO_ARGS_PASSED_ERROR }; 
enum Parse_Status { COMMAND, NOTE, MACRO, FAILED, NONE };
enum Exit_Status { NORMAL_EXIT, ERROR_EXIT, HELP_EXIT };

/* linked list helper functions */
void add2start(Note_Node **start, Note_Node *new);
void add2end(Note_Node **start, Note_Node **tail, Note_Node *new);
void traverse(Note_Node *start);
void del_from_end(Note_Node **start, Note_Node **tail);
void free_list(Note_Node **list, Note_Node **tail);

void free_array(char **array, int no_elements);

unsigned int parse_cmdline(char **cmdline_args, int no_args, FILE **in, FILE **out, char **out_name);

/* Performs a similar function to sscanf(), albeit with less
 * segfault-y behavior. Compares a string and a format.
 * Returns: 0 for match, 1 for no match */
unsigned int frmtcmp(char *str, char *frmt);

/* Returns: NORMAL or ARG_ERROR from enum Error_State */
unsigned int argchecker(char *argument, const char *format);

/* Returns: NAME_ERROR, ARG_ERROR, or NORMAL */
unsigned int validate_command(char *command, char *argument);

/* IN: Error line, line number, and string with the flagged error */
void print_error_line(char *line, int line_no, char *error_string);

/* Returns: COMMAND, FAILED, or NONE from enum Parse_Status */
unsigned int parse_command_or_macrodef(char *line, int line_number);

/* Parses a given element in a buffer, identifying a correctly
 * placed effects macro and constructing a more useful 
 * representation. It also alters the element, removing the macro
 * in order to allow it to be validated (returned effect info not
 * used) or converted into an intermediate representation */
Effect_Package parse_effects_macros(char *element);

/* Returns: status from enum Error_State */
unsigned int validate_buffer(int no_buffer_elements, char **buffer);

/* applies key signatures, parentheticals, etc.
 * IN: a full line, line number, buffer (passed
 *     by reference), and number of buffer elements (by reference)
 * Returns: FAILED, COMMAND, or NOTE from enum parse_status */
unsigned int get_line_buffer(char *line, int line_number, char ***buffer, int *buffer_elements);

Note_Node *convert_from_string(char *string, Key_Map *keymap, int **freq_table, double tempo);

void buffer_to_intrep(char **buffer, int buf_size, Note_Node **start, Note_Node **tail, Key_Map *keymap, int **freq_table, double tempo);

void write_to_file(Note_Node *representation, FILE *outfile, Note_Node *tail);

unsigned int parse_infile(FILE *infile, FILE *outfile, char **Key_Str, int **freq_table, Key_Map **key);
#endif
