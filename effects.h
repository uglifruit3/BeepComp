#ifndef effects_inc
#define effects_inc

/* header for defining related types, enumerators, and 
 * functions for effects */

#include <stdlib.h>
#include <stdio.h>

/* this value is given in tones/second */
static int Arpeggio_Rate = 60;

enum Effect_Name { NO_EFFECT, ARPEGGIO, PORTAMENTO, VIBRATO };

/* struct for dealing with effects */
typedef struct fx_node Effect_Package;
	/*int name;
	  Note_Node param1; 
	  int param2;
	  int param3;
	  int param4; 

 * table of effect values for Effect_Package 
 *
 *    NAME    |   param1  |  param2   |  param3   |    param4     |
 *=================================================================
 * arpeggio   | base note | interval1 | interval2 | arpeggio rate */

#include "parse.h"


unsigned short int hexchar_to_dec(char hexchar);

static void (*fx_macro)(Note_Node **start, Note_Node **tail, Effect_Package effect);
void expand_arpeggio(Note_Node **start, Note_Node **tail, Effect_Package effect);

#endif
