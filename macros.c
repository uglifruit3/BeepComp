#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "parse.h"
#include "macros.h"
#include "frequency.h"
#include "timing.h"

void m_add2start(Macro_Node **start, Macro_Node *new) {
	if( start == NULL ) {
		*start = new;
		new->next = NULL;
		return;
	}

	new->next = *start;
	*start = new;
}
Macro_Node *m_search(Macro_Node *list, char *term) {
	Macro_Node *temp = list;
	while( temp != NULL ) {
		if( !strcmp(term, temp->name) ) break;
		temp = temp->next;
	}

	return temp;
}
void m_free_list(Macro_Node **list) {
	while( *list != NULL ) {
		Macro_Node *temp = *list;
		*list = (*list)->next;
		free(temp);
	}
	free(*list);
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

void store_macro(char *line, Macro_Node **macro_list) {
	Macro_Node *macro = malloc(sizeof(Macro_Node));
	memset(macro->name, '\0', 32); memset(macro->macro, '\0', 256);
	int no_line_elements = get_no_elements(line);
	char **line_elements = get_elements_array(line, no_line_elements);
	strncpy(macro->name, line_elements[1], 32);

	char close_char;
	if( macro->name[0] == '$' ) close_char = '\"';
	else                        close_char = ']';  
	for( int i = 2; i < no_line_elements; i++ ) {
		if( i == 2 ) {
			/* isolated open character */
			if( strlen(line_elements[i]) == 1 ) continue;
			/* macro is one element, surrounded by open/close with no spaces*/
			else if( no_line_elements == 3 ) { 
				int j = 1;
				while( line_elements[i][j] != close_char ) {
					macro->macro[j-1] = line_elements[i][j];
					j++;
				}
				break;
			/* no space between open character and element */
			} else {
				int j = 1;
				while( line_elements[i][j] != '\0' ) {
					macro->macro[j-1] = line_elements[i][j];
					j++;
				}
				macro->macro[j-1] = ' '; macro->macro[j] = '\0';
				continue;
			}
		} else if( i == no_line_elements-1 ) {
			/* isolated open character */
			if( strlen(line_elements[i]) == 1 ) continue;
			/* no space between close character and element */
			else line_elements[i][strlen(line_elements[i])-1] = '\0';
		}
		strncat(macro->macro, line_elements[i], 32);
		strcat(macro->macro, " ");
	}

	Macro_Node *temp = m_search(*macro_list, macro->name);
	if( temp == NULL ) m_add2start(macro_list, macro);
	else {              
		memset(temp->name, '\0', 32);
		memset(temp->macro, '\0', 256);
		strncpy(temp->name, macro->name, 32);
		strncpy(temp->macro, macro->macro, 256);
		free(macro);
	}

	free_array(line_elements, no_line_elements);
}

void *parse_arp_macro(char *element, Macro_Node *list) {
	void *out_ptr;
	int fail  = FAILED;
	int fail2 = 2*FAILED; /* fail code for if element contains two macros */
	int none  = NONE;
	int startof_macro = 0;

	/* identify where the macro starts */
	while( element[startof_macro] != '@' ) {
		startof_macro++; 
		if( startof_macro > strlen(element) ) {
			out_ptr = &none;
			return out_ptr;
		}
	}

	if( startof_macro < 2 ) {
		out_ptr = &none;
		return out_ptr;
	}

	char temp_clone[32];
	strncpy(temp_clone, element, 32);
	if( parse_effects_macros(temp_clone).name != NO_EFFECT ) {
		out_ptr = &fail2;
		return out_ptr;
	}
	for( int i = startof_macro; i < strlen(element); i++ ) {
		temp_clone[i] = '\0';
	}
	if( parse_effects_macros(temp_clone).name != NO_EFFECT ) {
		out_ptr = &fail2;
		return out_ptr;
	}

	char macro_name[32];
	memset(macro_name, '\0', 32);
	int length = strlen(element);
	for( int i = startof_macro; i < length; i++ ) {
		macro_name[i-startof_macro] = element[i];
		element[i] = '\0';
	}

	Macro_Node *arp_macro = m_search(list, macro_name);
	if( arp_macro == NULL ) out_ptr = &fail;
	else                    out_ptr = arp_macro;

	return out_ptr;
}

void expand_arp_macro(Note_Node *base_note, Macro_Node *macro, Note_Node **start, Note_Node **tail, int arp_rate) {
	int no_macro_elements = get_no_elements(macro->macro);
	char **macro_elements = get_elements_array(macro->macro, no_macro_elements);

	long double dbl_total_notes = ((long double)arp_rate/1000.0) * base_note->duration;
	long int total_notes = (long int)dbl_total_notes;
	long double note_len = base_note->duration/dbl_total_notes;

	/* accounts for uneven length of note */
	double final_note_len = note_len;
	if( dbl_total_notes - total_notes > 0.005 ) {
		final_note_len = note_len * (dbl_total_notes-total_notes);
		total_notes++;
	}

	int *frequencies = malloc((no_macro_elements+1)*sizeof(int));
	frequencies[0] = base_note->frequency;
	int base_hsteps = hsteps_from_A4(frequencies[0]);
	for( int i = 1; i < no_macro_elements+1; i++ ) {
		frequencies[i] = round_dbl(calc_freq(base_hsteps+atoi(macro_elements[i-1])));
	}


	for( int i = 0; i < total_notes; i++ ) {
		Note_Node *int_rep = malloc(sizeof(Note_Node));
		/* cycles through all arpeggio tones */
		int_rep->frequency = frequencies[i % (no_macro_elements+1)];
		/* max function deals with edge case that total_notes-1 is 0 */
		int_rep->duration = note_len - (note_len-final_note_len)*(i/(max(total_notes-1, 1)));
		add2end(start, tail, int_rep);	
	}

	free_array(macro_elements, no_macro_elements);
	free(frequencies);
}

unsigned int expand_cus_macro(char ***buffer, int *no_buffer_elements, char *line, int line_no, Macro_Node *list) {
	Macro_Node *temp;
	/* search backward for macro name */
	int macro_found = FALSE;
	int i;
	for( i = *no_buffer_elements-1; i >= 0; i-- ) {
		if( buffer[0][i][0] == '$' ) { 
			temp = m_search(list, buffer[0][i]);
			if( temp != NULL ) { macro_found = TRUE; break; }
			else { 
				fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "macro \"%s\" not defined.\n", line_no, buffer[0][i]);
				print_error_line(line, line_no, buffer[0][i]);
				return FAILED;
			}
		}
	}

	if( !macro_found ) return NOTE;

	int macro_elements = get_no_elements(temp->macro);
	char **macro_buffer = get_elements_array(temp->macro, macro_elements);
	char **end_plc = NULL;

	int no_temp_elements = *no_buffer_elements+macro_elements-1;
	char **temp_buffer = malloc(no_temp_elements*sizeof(char*));
	for( int j = 0; j < i; j++ ) {
		temp_buffer[j] = malloc(32*sizeof(char));
		memset(temp_buffer[j], '\0', 32);
		strncpy(temp_buffer[j], buffer[0][j], 32);
	}

	if( i < *no_buffer_elements-1) {
		end_plc = malloc((*no_buffer_elements-i-1)*sizeof(char*));
		/* cut off the buffer that follows the macro */
		for( int j = i+1; j < *no_buffer_elements; j++ ) {
			end_plc[j-i-1] = malloc(32*sizeof(char)); 
			memset(end_plc[j-i-1], '\0', 32);
			strncpy(end_plc[j-i-1], buffer[0][j], 32);
			memset(buffer[0][j], '\0', 32);
		}
	}
	memset(buffer[0][i], '\0', 32);

	/* splice macro text into buffer */
	for( int j = 0; j < macro_elements; j++ ) {
		temp_buffer[i+j] = malloc(32*sizeof(char));
		strncpy(temp_buffer[i+j], macro_buffer[j], 32);
	}

	/* splice buffer end after macro is inserted */
	int k         = macro_elements+i;
	int plc_index = 0;
	while( plc_index < *no_buffer_elements-i-1 ) {
		temp_buffer[k] = malloc(32*sizeof(char));
		strncpy(temp_buffer[k], end_plc[plc_index], 32);
		k++; plc_index++;
	}

	free_array(macro_buffer, macro_elements);
	free_array(end_plc, *no_buffer_elements-i-1);
	free_array(*buffer, *no_buffer_elements);
	/* expand the number of buffer elements to reflect expanded macro size */
	*no_buffer_elements = *no_buffer_elements+macro_elements-1;
	*buffer = temp_buffer;

	return expand_cus_macro(buffer, no_buffer_elements, line, line_no, list);
}

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
		free_list(&close_parens, &close_tail);
		free_list(&open_parens, &open_tail);
		return FAILED;
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
		int local_time_mods = 0;
		int paren_counter = no_open_parens;
		while( paren_counter > 0 ) {
			local_time_mods = open_tail->NO_TIME_MODS;	
			int i;
			for( i = open_tail->INDEX+1; line_elements[i][0] != ')'; i++ ) {
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
				int cust_arp_macro_index = -1;
				for( int j = 2; j < strlen(line_elements[i]); j++ ) {
					if( line_elements[i][j] == '@' ) { cust_arp_macro_index = j; break; }
				}
				if( temp.name != NO_EFFECT || 
						line_elements[i][length-1] == '*' ||
						cust_arp_macro_index != -1 ) {
					int startof_macro = 0;
					while( line_elements[i][startof_macro] != '[' &&
								 line_elements[i][startof_macro] != '*' &&
								 line_elements[i][startof_macro] != '@' ) {
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
			line_elements[i][0] = line_elements[open_tail->INDEX][0] = '\0';
			del_from_end(&open_parens, &open_tail);
			del_from_end(&close_parens, &close_tail);
			paren_counter--;
		}
	} else {
		int paren_counter = no_open_parens;
		while( paren_counter > 0 ) {
			del_from_end(&open_parens, &open_tail);
			del_from_end(&close_parens, &close_tail);
			paren_counter--;
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
