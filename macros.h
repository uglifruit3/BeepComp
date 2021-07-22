#ifndef effects_inc
#define effects_inc

/* header for defining related types, enumerators, and 
 * functions for effects */

#include <stdlib.h>
#include <stdio.h>

/* this value is given in tones/second */
static int Arpeggio_Rate = 60;

enum Effect_Name { NO_EFFECT, ARPEGGIO };

typedef struct mnode {
	char name[32];
	char macro[256];
	struct mnode *next;
} Macro_Node;

static Macro_Node *Arp_Macros;
static Macro_Node *Cus_Macros;

/* struct for dealing with effects */
typedef struct fx_node Effect_Package;
	/*int name;
	  Note_Node param1; 
	  int param2;
	  int param3;
	  int param4; 

 * table of effect values for Effect_Package 
 *
 *   NAME    |   param1  |  param2   |  param3   |    param4     |
 *=================================================================
 * arpeggio  | base note | interval1 | interval2 | arpeggio rate */

#include "parse.h"

void m_add2start(Macro_Node **start, Macro_Node *new);
Macro_Node *m_search(Macro_Node *list, char *term);
void m_free_list(Macro_Node **list);

unsigned short int hexchar_to_dec(char hexchar);
long int max(long int a, long int b);

void store_macro(char *line, Macro_Node **macro_list);
void *parse_arp_macro(char *element, Macro_Node *list);
void expand_arp_macro(Note_Node *base_note, Macro_Node *macro, Note_Node **start, Note_Node **tail, int arp_rate);
unsigned int expand_cus_macro(char ***buffer, int *no_buffer_elements, char *line, int line_no, Macro_Node *list);

int expand_parens(char **line_elements, int *no_line_elements, char *line, int line_no);

static void (*fx_macro)(Note_Node **start, Note_Node **tail, Effect_Package effect);
void expand_arpeggio(Note_Node **start, Note_Node **tail, Effect_Package effect);

#endif
