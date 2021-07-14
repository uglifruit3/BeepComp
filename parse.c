#include <string.h>
#include <stdio.h>

#include "effects.h"
#include "parse.h"
#include "semantics.h"
#include "commands.h"
#include "frequency.h"
#include "timing.h"

void add2start(Note_Node **start, Note_Node *new) {
	new->next = *start;
	*start = new;
}
void add2end(Note_Node **start, Note_Node **tail, Note_Node *new) {
	if( *start == NULL ) add2start(start, new);
	else (*tail)->next = new;
	*tail = new;
}
void traverse(Note_Node *list) {
	Note_Node *temp = list;
	while( temp != NULL ) {
		printf("%d - %d\n", temp->frequency, temp->duration);
		temp = temp->next;
	}
}
void del_from_end(Note_Node **start, Note_Node **tail) {
	Note_Node *temp = *start;
	if( temp == *tail ) { free(*tail); return; }
	while( temp->next != *tail ) {
		temp = temp->next;
	}
	Note_Node *temp2 = *tail;
	*tail = temp;
	(*tail)->next = NULL;
	free(temp2);
}
void free_list(Note_Node **list, Note_Node **tail) {
	while( *list != *tail ) {
		Note_Node *temp = *list;
		*list = (*list)->next;
		free(temp);
	}
	free(*list);
}

unsigned int parse_cmdline(char **cmdline_args, int no_args, FILE **in, FILE **out, char **out_name) {
	//int in_flag = 0;
	//int out_flag = 0;
	int error_flag = 0;
	int help_flag = 0;
	int quiet_flag = 0;

	for( int i = 1; i < no_args; i++ ) {
		/* set of statements reads flags, opens files, and checks for 
		 * errors as appropriate */
		if( cmdline_args[i][0] == '-' ) {
			switch( cmdline_args[i][1] ) {
				case 'o':
					i++;
					if( i > no_args-1 ) { continue; }
					*out = fopen(cmdline_args[i], "w");
					*out_name = cmdline_args[i];
					//out_flag = 1;
					break;
				case 'f':
					i++;
					*in = fopen(cmdline_args[i], "r");
					//in_flag = 1;
					if( *in == NULL ) error_flag = 1;
					break;
				case 'h':
					help_flag = 1;
					break;
				default:
					printf(ANSI_BOLD "Error: " ANSI_RESET "invalid options have been supplied.\n");
					error_flag = 1; 
					break;
			}
		} else error_flag = 1;
	}
	/* ensuring that an input and output are specified. Printing 
	 * error messages if not */
	if( error_flag || help_flag ) {
		if( error_flag && *in == NULL ) {
			return 1;
		}
		printf("Usage: beepcomp [-h] [-f " ANSI_UNDR "infile" ANSI_RESET "] [-o " ANSI_UNDR "outfile" ANSI_RESET "]\n" );
		if( help_flag ) return 2;
		else return 1;
	}

	if( *in  == NULL ) *in = stdin;
	if( *out == NULL ) *out = stdout;
	return 0;
}

unsigned int frmtcmp(char *str, char *frmt) {
	/* counting number of words in response and template */
	int no_frmt_words = 0;
	if( strcmp(frmt, "")) {
		for( int i = 0; i < strlen(frmt); i++ ) {
			if( frmt[i] == ' ' ) no_frmt_words++;
		}
		no_frmt_words++;
	}
	
	int no_str_words = 0;
	if( strcmp(str, "") ) {
		for( int i = 0; i < strlen(str); i++ ) {
			if( str[i] == ' ' ) no_str_words++;
		}
		no_str_words++;
	}

	if( no_str_words < no_frmt_words ) return NO_ARGS_PASSED_ERROR;
	else if( no_str_words != no_frmt_words ) return ARG_ERROR;

	/* building space-delimited arrays of response and template */
	const char delim[] = " ";
	char *token;
	char **frmt_words = malloc(no_frmt_words*sizeof(char*));
	token = strtok(frmt, delim);
	frmt_words[0] = token;
	for( int i = 1; i < no_frmt_words; i++ ) {
		token = strtok(NULL, delim);
		frmt_words[i] = token;
	}
	char *token2;
	char **str_words = malloc(no_str_words*sizeof(char*));
	token2 = strtok(str, delim);
	str_words[0] = token2;
	for( int i = 1; i < no_str_words; i++ ) {
		token2 = strtok(NULL, delim);
		str_words[i] = token2;
	}

	for( int i = 0; i < no_frmt_words; i++ ) {
		/* validity_flag points to whether the passed string matches the format
		   1 denotes matching, 0 denotes not matching */
		int validity_flag = 1;
		if( frmt_words[i][0] == '%' ) {
			switch( frmt_words[i][1] ) {
				case 'i':
					validity_flag = atoi(str_words[i]);
					break;
				case 's':
					validity_flag = !atoi(str_words[i]);
					break;
				case 'f':
					validity_flag = atof(str_words[i]);
					break;
				case 'c':
					validity_flag = (strlen(str_words[i]) == 1 && str_words[i][0] <= 'z' && str_words[i][0] >= 'A') ? 1:0;
					break;
				default:
					validity_flag = 0;
					break;
			}
		}
		else {
			if( !strcmp(frmt_words[i], str_words[i]) ) validity_flag = 1;
			else validity_flag = 0;
		}
		
		if( validity_flag == 0 ) { free(frmt_words); free(str_words); return ARG_ERROR; }
	}

	free(frmt_words);
	free(str_words);
	return NORMAL;
}
	
unsigned int argchecker(char *argument, const char *format) {
	char temp_format[128];
	strncpy(temp_format, format, 128);
	int stat = frmtcmp(argument, temp_format);
	return stat;
}

unsigned int validate_command(char *command, char *argument) {
	int i = 0;
	while( commands[i] != "\0" ) {
		/* strcmp is negated because it returns zero if it identifies a match */

		/* this block specifically validates the argument to set key.
		 * it does so here because it adds minimal complication 
		 * elsewhere in the program */
		if( !strcmp(command, "key") ) {
			if( argument[1] != ' ' && argument[1] != '#' && argument[1] != 'b' && argument[1] != 'n' ) 
				return ARG_ERROR;
		}	

		if( !strcmp(command, commands[i] ))  
			return argchecker(argument, commands[i+1]); 
		i += 2;
	}
	return NAME_ERROR;
}

void print_error_line(char *line, int line_no, char *error_string) {
	/* substring matching to identify where in the line the error occurs */
	int error_index;
	int found_string = 0;
	for( int i = 0; i < strlen(line)-strlen(error_string)+1; i++ ) {
		if( line[i] == error_string[0] ) {
			found_string = 1;
			for( int k = i; k < strlen(error_string)+i; k++ ) { 
				if( line[k] != error_string[k-i] ) { found_string = 0; break; }
			}	
		}

		if( found_string ) { error_index = i; break; }
	}
				
	fprintf(stderr, "LINE %03i: ", line_no);
	fprintf(stderr, "%s\n", line);
	for( int i = 0; i < error_index+10; i++ ) { putchar(' '); }
	printf(ANSI_CYAN);
	putchar('^');
	for( int i = 0; i < strlen(error_string)-1; i++ ) { putchar('~'); }
	printf(" HERE\n" ANSI_RESET);
}

unsigned int parse_for_command(char *line, int line_number) {
	char first_str[128];
	sscanf(line, "%s", first_str);

	if( !strcmp(first_str, COMMAND_KEYWORD) ) {
		char cmd_name[128];
		sscanf(line, "%*s %s", cmd_name);

		char argument[128] = "";
		int j = 0;
		int i; 
		for( i = strlen(COMMAND_KEYWORD)+strlen(cmd_name)+2;
				 line[i] != '\0'; i++ ) {
			if( line[i+1] == COMMENT_CHAR ) break;
			else argument[j++] = line[i];	
		}
		argument[i+1] = '\0';


		switch( validate_command(cmd_name, argument) ) {
			case NORMAL:
				return COMMAND;
			case NAME_ERROR:
				fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "command \"%s\" does not exist.\n", line_number, cmd_name);
				print_error_line(line, line_number, cmd_name);
				return NONE;
				break;
			case ARG_ERROR:
				fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "invalid argument(s) given for command \"%s\".\n", line_number, cmd_name);
				print_error_line(line, line_number, argument);
				break;
			case NO_ARGS_PASSED_ERROR:
				fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "insufficient argument(s) given for command \"%s\".\n", line_number, cmd_name);
				print_error_line(line, line_number, cmd_name);
				break;
		}
		return FAILED;
	}
	else 
		return NONE;
}

Effect_Package parse_effects_macros(char *element) {
	int startof_macro = 0;
	Effect_Package effect;
	effect.name = NO_EFFECT;

	/* identify where the macro starts */
	while( element[startof_macro] != '[' &&
	       element[startof_macro] != '{' &&
	       element[startof_macro] != '<' ) {
		startof_macro++; 
		if( startof_macro > strlen(element) ) break;
	}

	if( startof_macro < 2 ) return effect;

	/* asses validity of macro invocation */
	if(((element[startof_macro+1] >= '0' &&
			 element[startof_macro+1] <= '9' ) ||
			(element[startof_macro+1] >= 'A' &&
			 element[startof_macro+1] <= 'F' ) )
			&&
		 ((element[startof_macro+2] >= '0' &&
			 element[startof_macro+2] <= '9' ) ||
			(element[startof_macro+2] >= 'A' &&
			 element[startof_macro+2] <= 'F' ) )
			&&
		 element[startof_macro+3] == element[startof_macro]+2 
		  && 
		 startof_macro+4 == strlen(element) ) {
		effect.param2 = hexchar_to_dec(element[startof_macro+1]);
		effect.param3 = hexchar_to_dec(element[startof_macro+2]);
		if( element[startof_macro] == '[' ) { 
			effect.name = ARPEGGIO; fx_macro = expand_arpeggio;
			effect.param4 = Arpeggio_Rate;
		} else if( element[startof_macro] == '<' ) {
			effect.name = PORTAMENTO; fx_macro = expand_portamento;
		} else { 
			effect.name = VIBRATO; fx_macro = expand_vibrato; }

		for( int i = startof_macro; i < strlen(element); i++ ) {
			element[i] = '\0';
		}
	}
	return effect;
}

unsigned int validate_buffer(int no_buffer_elements, char **buffer) {
	int status = NORMAL;
	int i;
	int time_pos;
	for( i = 0; i < no_buffer_elements; i++ ) {
		/* call to parse_effects_macros sanitizes the buffer, and
		 * prevents this function from flagging effects macros as 
		 * errors as it otherwise would */
		Effect_Package temp; temp.name = NO_EFFECT;
		temp = parse_effects_macros(buffer[i]);
		if( temp.name == PORTAMENTO && i == no_buffer_elements-1 )
			printf(ANSI_BOLD "Note: " ANSI_RESET "the portamento macro requires a following note on the same line to slide to; it will not be expanded on the last note in a line.\n");

		/* this block of if's and else's confirm the presence of a 
		 * note and octave, then the validity of time mods in that
		 * order. It also handles ties */
		int octave_pos = 1;
		if( buffer[i][0] >= 'A' && buffer[i][0] <= 'G' ) {
			if( buffer[i][1] == '#' || buffer[i][1] == 'b' || buffer[i][1] == 'n' ) {
				octave_pos = 2;
			}
			if( buffer[i][octave_pos] >= '1' && buffer[i][octave_pos] <= ROWS_IN_TABLE+47 ) {
				time_pos = octave_pos + 1;
				if( octave_pos == strlen(buffer[i])-1 ) continue;
				else if( buffer[i][time_pos] == 'o' || buffer[i][time_pos] == ',' || buffer[i][time_pos] == '^' || buffer[i][time_pos] == '.' ) { 
					if( buffer[i][++time_pos] == '\0' ) continue;
					else {
						char time_mod;
						if( buffer[i][time_pos-1] == 'o' || buffer[i][time_pos-1] == ',' ) time_mod = '.';
						else time_mod = buffer[i][time_pos];
						for( int j = time_pos; j < strlen(buffer[i]); j++ ) {
							if( buffer[i][j] != time_mod ) { 
								if( buffer[i][j] == '.' ) time_mod == '.'; 
								else { status = i+1; break; }
							}
						}
					}
				} else { status = i+1; break; }
			} else { status = i+1; break; }
		} else if( buffer[i][0] == 'r' ) {
			time_pos = 1;
			if( buffer[i][time_pos] == '\0' ) continue;
			else {
				char time_mod;
				if( buffer[i][time_pos] == 'o' || buffer[i][time_pos] == ',' ) time_mod = '.';
				else time_mod = buffer[i][time_pos];
				for( int j = time_pos+1; j < strlen(buffer[i]); j++ ) {
					if( buffer[i][j] != time_mod ) { 
						if( buffer[i][j] == '.' ) time_mod == '.'; 
						else { status = i+1; break; }
					}
				}	
			}
		} else if( buffer[i][1] == '\n' ) continue;
		else if( buffer[i][0] == '-' ) {
			char prev_note[3]; 
			char next_note[3];
			next_note[2] = prev_note[2] = '\0';
			if( strlen(buffer[i+1]) <= octave_pos ) { status=i+1; break;}
			for( int k = 0; k <= octave_pos; k++ ) {
				prev_note[k] = buffer[i-1][k]; 
				next_note[k] = buffer[i+1][k];
			}

			if( strcmp(prev_note, next_note) ) { 
				fprintf(stderr, ANSI_BOLD "Note: " ANSI_RESET "tied notes must be the same pitch.\n");
				status = i+1; break; 
			}
		} else { status = i+1; break; }
	}
	return status;
}

unsigned int get_line_buffer(char *line, int line_number, char ***buffer, int *buffer_elements) {
	/* helps resolve floating spaces at the ends of lines */
	int back_index = strlen(line)-1;
	while( line[back_index] == ' ' ) { 
		line[back_index] = '\0';
		back_index--;
	}
	/* identify number of elements in the line separated by spaces */
	int no_line_elements = 0;
	for( int i = 0; i < strlen(line); i++ ) {
		if( line[i] == COMMENT_CHAR && line[i-1] == ' ' ) { no_line_elements--; break; }
		else if( line[i] == ' ' ) { 
			no_line_elements++;
			while( line[i] == ' ' ) { i++; }
		}
	}
	no_line_elements++;

	/* construct an array of the line, delimited by spaces */
	char **line_elements = (char**)malloc(no_line_elements*sizeof(char*));
	int pos_in_line = 0;
	for( int i = 0; i < no_line_elements; i++ ) {
		line_elements[i] = (char*)malloc(32*sizeof(char));
		memset(line_elements[i], '\0', 32);
		int element_index = 0;
		while( line[pos_in_line] != ' ' && line[pos_in_line] != '\0' && line[pos_in_line] != '\n' ) {
			line_elements[i][element_index] = line[pos_in_line];
			element_index++;
			pos_in_line++;
		}
		line_elements[i][element_index+1] = '\0';
		while( line[pos_in_line] == ' ' ) { pos_in_line++; }
	}

	/* this switch handles checking the line for commands first */
	switch ( parse_for_command(line, line_number) ) {
		case COMMAND:
			*buffer = line_elements;
			*buffer_elements = no_line_elements;
			return COMMAND;
		case FAILED:
			free(line_elements);
			return FAILED;
	}

	/* it is now presumed the line specifies notes */
	int exit_status = NOTE;

	/* checking for parentheses. Uses linked lists to track the
	 * locations of all open and close parens in a buffer, as well
	 * as the time mods which will be applied within them */
	Note_Node *open_parens = NULL; Note_Node *open_tail = NULL;
	Note_Node *close_parens = NULL; Note_Node *close_tail = NULL;
	int no_open_parens = 0, no_close_parens = 0;
	int i;
	/* ensuring parentheses are invoked legally */
	for( i = 0; i < no_line_elements; i++ ) {
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
		fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "illegal character(s) in parenthesis phrase.\n", line_number);
		print_error_line(line, line_number, line_elements[i]);
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
	if( exit_status == FAILED ) {
		fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "open and close-parentheses do not match.\n", line_number);
	}
	/* expanding notes' time mods within parens */
	else {
		Note_Node *o_temp = open_tail;
		Note_Node *c_temp = close_tail;
		int local_time_mods = 0;
		int paren_counter = no_open_parens;
		while( paren_counter > 0 ) {
			local_time_mods = o_temp->NO_TIME_MODS;	
			int i;
			for( i = o_temp->INDEX+1; line_elements[i][0] != ')'; i++ ) {
				if( line_elements[i][0] == '-' && line_elements[i][1] == '\0' ) continue;
				char cpy[32]; strncpy(cpy, line_elements[i], 32);
				Effect_Package temp = parse_effects_macros(cpy);
				char macro_temp[5] = "\0";
				/* if a macro is present, this block splices the appended
				 * time mod between the note and macro so as to feed legal
				 * input to the buffer validation and conversion functions 
				 * later on */
				if( temp.name != NO_EFFECT ) {
					int startof_macro = 0;
					while( line_elements[i][startof_macro] != '[' &&
								 line_elements[i][startof_macro] != '{' &&
								 line_elements[i][startof_macro] != '<' ) {
						startof_macro++; 
					}
					for( int k = startof_macro; k < startof_macro+5; k++ ) {
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

		/* adjusting the number of elements in the buffer array and 
		 * removing parentheses after expansion */
		if( no_open_parens > 0 ) {
			int no_new_elements = no_line_elements - (2 * no_open_parens);
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
			for( int i = 0; i < no_line_elements; i++ ) { free(line_elements[i]); }
			free(line_elements);
			line_elements = new_elements;
			no_line_elements = no_new_elements;
		}
	}

	/* checks the validity of the buffer once fully assembled */
	if( exit_status != FAILED ) {
		char **temp_buffer = malloc(no_line_elements*sizeof(char*));
		for( int i = 0; i < no_line_elements; i++ ) {
			temp_buffer[i] = malloc(32*sizeof(char));
			memset(temp_buffer[i], '\0', 32);
			strncpy(temp_buffer[i], line_elements[i], 32);
		}
		int stat = validate_buffer(no_line_elements, temp_buffer);
		for( int i = 0; i < no_line_elements; i++ ) {
			free(temp_buffer[i]);
		}
		free(temp_buffer);
		if( stat != NORMAL ) {
			fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "illegal character(s) in note.\n", line_number);
			print_error_line(line, line_number, line_elements[stat-1]);
			exit_status = FAILED;
		}
	}

	*buffer = line_elements;
	*buffer_elements = no_line_elements;
	return exit_status;
}

Note_Node *convert_from_string(char *string, Key_Map *keymap, int **freq_table, double tempo) {
	Note_Node *int_rep = malloc(sizeof(Note_Node));
	/* obtain ascii pitch representation */	
	char pitch[3] = "";
	int i = 0;
	while( string[i] != 'o' 
			&& string[i] != '^' 
			&& string[i] != '.' 
			&& string[i] != ',' 
			&& string[i] != '\0' ) {	
		pitch[i] = string[i];
		i++;
	}
	pitch[3] = '\0';
	/* applying the key signature */
	int k = 0;
	while( keymap[k].name != NULL ) {
		if( pitch[0] == keymap[k].name[0] ) {
			if( pitch[1] != '#' && pitch[1] != 'b' && pitch[1] != 'n' ) {
				pitch[2] = pitch[1]; 
			  pitch[1] = keymap[k].accidental; 
			}
		}
		k++;
	}
	/* getting frequency */
	int_rep->frequency = get_freq_from_string(pitch, freq_table);

	/* obtain ascii timing representation */
	char time[32];
	memset(time, '\0', 32);
	int j = 0;
	while( i < strlen(string) ) {
		time[j] = string[i];
		i++; j++;
	}
	/* getting duration */
	int_rep->duration  = get_duration_from_string(time, tempo);

	return int_rep;
}

void buffer_to_intrep(char **buffer, int buff_size, Note_Node **start, Note_Node **tail, Key_Map *keymap, int **freq_table, double tempo) {
	Effect_Package effect;
	Note_Node *int_rep;

	for( int i = 0; i < buff_size; i++ ) {
		effect = parse_effects_macros(buffer[i]);

		/* if an effect macro has been invoked */
		if( effect.name ) {
			/* param1 is the same for all effects */
			Note_Node *base_note = convert_from_string(buffer[i], keymap, freq_table, tempo);
			effect.param1 = *base_note;
			/* extra condition prevents portamento from being used if 
			 * there is no follow-on note */
			if( effect.name == PORTAMENTO && i != buff_size-1 ) {
				Note_Node *temp = convert_from_string(buffer[i+1], keymap, freq_table, tempo);
				effect.param4 = temp->frequency;
				free(temp);
			} else if( effect.name == ARPEGGIO ) {
				int base_hsteps = hsteps_from_A4(effect.param1.frequency);
				effect.param2 = round_dbl(calc_freq(base_hsteps+effect.param2));
				effect.param3 = round_dbl(calc_freq(base_hsteps+effect.param3));
			}

			fx_macro(start, tail, effect);
			free(base_note);

		/* if a tie has been invoked */
		} else if( buffer[i][0] == '-' ) {
			i++;
			Note_Node *temp_rep = convert_from_string(buffer[i], keymap, freq_table, tempo);
			(*tail)->duration += temp_rep->duration;

		/* regular case */
		} else {
			int_rep = convert_from_string(buffer[i], keymap, freq_table, tempo);
			add2end(start, tail, int_rep);	
		}		
	}
}

void write_to_file(Note_Node *representation, FILE *outfile, Note_Node *tail) {
	Note_Node *temp = representation;
	int start = 1;
	
	fprintf(outfile, "#!/bin/sh\nbeep ");
	/* list traversal */
	while( temp != tail ) {
		if( start == 1 ) fprintf(outfile, "-f %d -l %f \\\n", temp->frequency, temp->duration);
		else fprintf(outfile, "-n -f %d -l %f \\\n", temp->frequency, temp->duration);
		start = 0;
		
		temp = temp->next;
	}
	fprintf(outfile, "-n -f %d -l %f \\\n", temp->frequency, temp->duration);
}

unsigned int parse_infile(FILE *infile, FILE *outfile, char **Key_Str, int **freq_table, Key_Map **key) {
	/* initializing line array (updated every iteration) */
	char *line = malloc(256*sizeof(char));
	char *stat;
	int line_number = 0;

	/* initializing linked list for intermediate rep */
	unsigned int linked_list_nodes = 0;
	Note_Node *Notes_Array = NULL;
	Note_Node *tail = NULL;

	/* initializing buffer */
	char **buffer = NULL;
	int elements= 0;
	int no_buff_elements;

	/* this loop cycles through each line of input, builds a
	 * buffer from it, then converts it into an intermediate
	 * representation */
	int exit = 0;
	while( 1 ) {
		fflush(stdin);
		line_number++;
		stat = fgets(line, 256, infile);
		if( stat == NULL ) break; /* breaks at EOF */
		if( line[0] == '\n' || line[0] == COMMENT_CHAR ) continue; /* ignores empty lines */
		line[strlen(line)-1] = '\0'; /*ensures null terminate */

		elements = get_line_buffer(line, line_number, &buffer, &no_buff_elements);
		/* program will not abort on bad input if input source
		 * is stdin; otherwise exit */
		if( elements == FAILED ) { 
			if( infile == stdin ) continue;
			else { exit = 1; break; }
		} else if( elements == COMMAND ) { 
			if( !strcmp(buffer[1], "tempo") ) command_tempo(atoi(buffer[2]), &Tempo);
			else if( !strcmp(buffer[1], "key") ) command_key(buffer[2], buffer[3], *Key_Str, key);
			else if( !strcmp(buffer[1], "arprate") ) command_arprate(atoi(buffer[2]), &Arpeggio_Rate);
			for( int i = 0; i < no_buff_elements; i++ ) { free(buffer[i]); }
			free(buffer);
			continue;
		}
		/* building intermediate representation from buffer */	
		buffer_to_intrep(buffer, no_buff_elements, &Notes_Array, &tail, *key, freq_table, Tempo);
		for( int i = 0; i < no_buff_elements; i++ ) { free(buffer[i]); }
		free(buffer);
	}

	if( elements == FAILED && outfile != stdout ) {
		for( int i = 0; i < no_buff_elements; i++ ) { free(buffer[i]); }
		free(buffer);
		fprintf(stderr, "Input not succesfully written to file.\n");
		exit = 1;
	}
	else if( Notes_Array != NULL ) write_to_file(Notes_Array, outfile, tail);

	free(line);
	free_list(&Notes_Array, &tail); 
	return exit;
}
