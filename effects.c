#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "parse.h"
#include "effects.h"
#include "frequency.h"
#include "timing.h"

int expand_parens(char **line_elements, int *no_line_elements, char *line, int line_no) {
	int exit_status = NOTE;
/* checking for parentheses. Uses linked lists to track the
	 * locations of all open and close parens in a buffer, as well
	 * as the time mods which will be applied within them */
	Note_Node *open_parens = NULL; Note_Node *open_tail = NULL;
	Note_Node *close_parens = NULL; Note_Node *close_tail = NULL;
	int no_open_parens = 0, no_close_parens = 0;

	int i;
	/* ensuring parentheses are invoked legally */
	for( i = 0; i < *no_line_elements; i++ ) {
		int timemods_at_paren = 0;
		if( line_elements[i][0] == '^' ) {
			int j = 0;
			while( line_elements[i][j] != '(' ) {
				if( line_elements[i][j] != '^' ) {
					exit_status = FAILED; break;
				}
				timemods_at_paren++;			
				j++;
			}
			if( j != strlen(line_elements[i])-1 ) exit_status = FAILED;
			if( exit_status == FAILED ) {
				break;
			}
			Note_Node *temp = malloc(sizeof(Note_Node));
			temp->INDEX = i; temp->NO_TIME_MODS = timemods_at_paren;
			add2end(&open_parens, &open_tail, temp); no_open_parens++;
		}
		else if( line_elements[i][0] == ')' ) {
			if( line_elements[i][1] != '\0' ) { exit_status = FAILED; break; }
			Note_Node *temp = malloc(sizeof(Note_Node));
			temp->INDEX = i; temp->NO_TIME_MODS = 0;
			add2end(&close_parens, &close_tail, temp); no_close_parens++;
			}
	}

	if( exit_status == FAILED ) {
		fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "illegal character(s) in parenthesis phrase.\n", line_no);
		print_error_line(line, line_no, line_elements[i]);
	} 
	/* ensuring each open paren corresponds to a closed one */
	else {
		Note_Node *o_temp = open_parens;
		Note_Node *c_temp = close_parens;
		if( no_open_parens == no_close_parens ) {
			for( int k = 0; k < no_close_parens; k++ ) {
				if( o_temp->INDEX > c_temp->INDEX ) {
					exit_status = FAILED; break; 
				}
				o_temp = o_temp->next;
				c_temp = c_temp->next;
			}
		} else exit_status = FAILED;
	}

	if( exit_status == FAILED ) 
		fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "open and close-parentheses do not match.\n", line_no);

	/* expanding notes' time mods within parens */
	if( exit_status != FAILED ) {
		Note_Node *o_temp = open_tail;
		Note_Node *c_temp = close_tail;
		int local_time_mods = 0;
		int paren_counter = no_open_parens;
		while( paren_counter > 0 ) {
			local_time_mods = o_temp->NO_TIME_MODS;	
			int i;
			for( i = o_temp->INDEX+1; line_elements[i][0] != ')'; i++ ) {
				int length = strlen(line_elements[i]);
				if( line_elements[i][0] == '-' && line_elements[i][1] == '\0' ) continue;
				char cpy[32]; strncpy(cpy, line_elements[i], 32);
				Effect_Package temp = parse_effects_macros(cpy);
				char macro_temp[32];
				memset(macro_temp, '\0', 32);
				/* if a macro is present, this block splices the appended
				 * time mod between the note and macro so as to feed legal
				 * input to the buffer validation and conversion functions 
				 * later on */
				if( temp.name != NO_EFFECT || line_elements[i][length-1] == '*' ) {
					int startof_macro = 0;
					while( line_elements[i][startof_macro] != '[' &&
								 line_elements[i][startof_macro] != '*' ) {
						startof_macro++; 
					}
					for( int k = startof_macro; k < length; k++ ) {
						macro_temp[k-startof_macro] = line_elements[i][k];
						line_elements[i][k] = '\0';
					}
				}
				/* append time mod specified by parens */
				for( int j = 0; j < local_time_mods; j++ ) {
					if( line_elements[i][0] == '\0' ) continue; 
					strcat(line_elements[i], "^\0");
				}
				strcat(line_elements[i], macro_temp);
			}
			/* cleaning up */
			line_elements[i][0] = line_elements[o_temp->INDEX][0] = '\0';
			del_from_end(&open_parens, &open_tail);
			del_from_end(&close_parens, &close_tail);
			paren_counter--;
			o_temp = open_tail;
			c_temp = close_tail;
		}
	} else {
		Note_Node *o_temp = open_tail;
		Note_Node *c_temp = close_tail;
		int paren_counter = no_open_parens;
		while( paren_counter > 0 ) {
			del_from_end(&open_parens, &open_tail);
			del_from_end(&close_parens, &close_tail);
			paren_counter--;
			o_temp = open_tail;
			c_temp = close_tail;
		}
	}

	/* adjusting the number of elements in the buffer array and 
	 * removing parentheses after expansion */
	if( no_open_parens > 0 && exit_status != FAILED ) {
		int no_new_elements = *no_line_elements - (2 * no_open_parens);
		char **new_elements = malloc(no_new_elements*sizeof(char*));
		int old_index = 0;
		for( int i = 0; i < no_new_elements; i++ ) {
			if( line_elements[old_index][0] == '\0' ) {
				i--; old_index++;
				continue;
			}
			new_elements[i] = malloc(32*sizeof(char));
			memset(new_elements[i], '\0', 32);
			strncpy(new_elements[i], line_elements[old_index], 32);
			old_index++;
		}
		for( int i = 0; i < *no_line_elements; i++ ) {
			memset(line_elements[i], '\0', 32);
			if( i < no_new_elements ) {
				strncpy(line_elements[i], new_elements[i], 32);
				free(new_elements[i]);
			} 
		}
		*no_line_elements = no_new_elements;
		free(new_elements);
	}
	return exit_status;
}

unsigned short int hexchar_to_dec(char hexchar) {
	if     ( hexchar >= '0' && hexchar <= '9' )
		return hexchar - 48;
	else if( hexchar >= 'A' && hexchar <= 'F' )
		return hexchar - 55;
	else return 16;
}
long int max(long int a, long int b) {
	if( a > b ) return a;
	return b;
}

void expand_arpeggio(Note_Node **start, Note_Node **tail, Effect_Package effect) {
	long double dbl_total_notes = ((long double)effect.param4/1000.0) * effect.param1.duration;
	long int total_notes = (long int)dbl_total_notes;
	long double note_len = effect.param1.duration/dbl_total_notes;

	/* accounts for uneven length of note */
	double final_note_len = note_len;
	if( dbl_total_notes - total_notes > 0.005 ) {
		final_note_len = note_len * (dbl_total_notes-total_notes);
		total_notes++;
	}

	int frequencies[3] = { effect.param1.frequency, effect.param2, effect.param3 };

	for( int i = 0; i < total_notes; i++ ) {
		Note_Node *int_rep = malloc(sizeof(Note_Node));
		/* cycles through the 3 arpeggio tones */
		int_rep->frequency = frequencies[i % 3];
		/* max function deals with edge case that total_notes-1 is 0 */
		int_rep->duration = note_len - (note_len-final_note_len)*(i/(max(total_notes-1, 1)));
		add2end(start, tail, int_rep);	
	}
}
