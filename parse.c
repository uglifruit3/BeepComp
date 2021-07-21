#include <string.h>
#include <stdio.h>

#include "macros.h"
#include "parse.h"
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

void free_array(char **array, int no_elements) {
		for( int i = 0; i < no_elements; i++ ) {
			free(array[i]);
		}
		free(array);
}

unsigned int parse_cmdline(char **cmdline_args, int no_args, FILE **in, FILE **out, char **out_name) {
	int error_flag = FALSE;
	int help_flag = FALSE;
	int quiet_flag = FALSE;

	for( int i = 1; i < no_args; i++ ) {
		if( cmdline_args[i][0] == '-' ) {
			switch( cmdline_args[i][1] ) {
				case 'o':
					i++;
					if( i > no_args-1 ) { continue; }
					*out = fopen(cmdline_args[i], "w");
					*out_name = cmdline_args[i];
					break;
				case 'f':
					i++;
					*in = fopen(cmdline_args[i], "r");
					if( *in == NULL ) error_flag = TRUE;
					break;
				case 'h':
					help_flag = TRUE;
					break;
				default:
					printf(ANSI_BOLD "Error: " ANSI_RESET "invalid options have been supplied.\n");
					error_flag = TRUE; 
					break;
			}
		} else error_flag = TRUE;
	}

	if( error_flag || help_flag ) {
		if( error_flag && *in == NULL ) {
			return ERROR_EXIT;
		}
		printf("Usage: beepcomp [-h] [-f " ANSI_UNDR "infile" ANSI_RESET "] [-o " ANSI_UNDR "outfile" ANSI_RESET "]\n" );

		if( help_flag ) return HELP_EXIT;
		else return ERROR_EXIT;
	}

	if( *in  == NULL ) *in = stdin;
	if( *out == NULL ) *out = stdout;
	return NORMAL_EXIT;
}

unsigned int frmtcmp(char *str, char *frmt) {
	/* counting number of words in response and template */
	int no_frmt_words = 0;
	if( strcmp(frmt, "")) {
		for( int i = 0; i < strlen(frmt); i++ ) {
			if( frmt[i] == ' ' ) {
				while( frmt[i] == ' ' && i < strlen(frmt) ) { i++; }
				no_frmt_words++;
			}
		}
		no_frmt_words++;
	}
	
	int no_str_words = 0;
	if( strcmp(str, "") ) {
		for( int i = 0; i < strlen(str); i++ ) {
			if( str[i] == ' ' ) {
				while( frmt[i] == ' ' && i < strlen(frmt) ) { i++; }
				no_str_words++;
			}
		}
		no_str_words++;
	}

	if( no_str_words != no_frmt_words ) return NO_ARGS_PASSED_ERROR;

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
		int match_flag = TRUE;
		if( frmt_words[i][0] == '%' ) {
			switch( frmt_words[i][1] ) {
				case 'i':
					match_flag = atoi(str_words[i]);
					break;
				case 's':
					match_flag = !atoi(str_words[i]);
					break;
				case 'f':
					match_flag = atof(str_words[i]);
					break;
				case 'c':
					match_flag = (strlen(str_words[i]) == 1 && str_words[i][0] <= 'z' && str_words[i][0] >= 'A') ? TRUE:FALSE;
					break;
				default:
					match_flag = FALSE;
					break;
			}
		}
		else {
			if( !strcmp(frmt_words[i], str_words[i]) ) match_flag = TRUE;
			else match_flag = FALSE;
		}
		
		if( !match_flag ) { free(frmt_words); free(str_words); return ARG_ERROR; }
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
			if( argument[1] != '\0' && argument[1] != ' ' && argument[1] != '#' && argument[1] != 'b' && argument[1] != 'n' ) 
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
	int found_string = FALSE;
	for( int i = 0; i < strlen(line)-strlen(error_string)+1; i++ ) {
		if( line[i] == error_string[0] ) {
			found_string = TRUE;
			for( int k = i; k < strlen(error_string)+i; k++ ) { 
				if( line[k] != error_string[k-i] ) { found_string = FALSE; break; }
			}	
		}

		if( found_string == TRUE ) { error_index = i; break; }
	}
				
	fprintf(stderr, "LINE %03i: ", line_no);
	fprintf(stderr, "%s\n", line);
	for( int i = 0; i < error_index+10; i++ ) { putchar(' '); }
	printf(ANSI_CYAN);
	putchar('^');
	for( int i = 0; i < strlen(error_string)-1; i++ ) { putchar('~'); }
	printf(" HERE" ANSI_RESET "\n");
}

unsigned int parse_command_or_macrodef(char *line, int line_number) {
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
				break;
			case ARG_ERROR:
				fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "invalid argument(s) supplied for command \"%s\".\n", line_number, cmd_name);
				print_error_line(line, line_number, argument);
				break;
			case NO_ARGS_PASSED_ERROR:
				fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "incorrect number of argument(s) supplied for command \"%s\".\n", line_number, cmd_name);
				print_error_line(line, line_number, cmd_name);
				break;
		}
		return FAILED;
	}
	else if( !strcmp(first_str, MACRO_DEF_KEYWORD) ) {
		char macro_name[64]; memset(macro_name, '\0', 64);
		sscanf(line, "%*s %s", macro_name);
		if( macro_name[0] == ARP_MACRO_CHAR ) return ARP_MDEF;
		if( macro_name[0] == CUS_MACRO_CHAR ) return CUS_MDEF;
		
		fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "macro incorrectly defined.\n", line_number );
		if( macro_name[0] == '\0' ) print_error_line(line, line_number, "define");
		else                        print_error_line(line, line_number, macro_name);
		return FAILED;
	} 
	return NONE;
}

Effect_Package parse_effects_macros(char *element) {
	int startof_macro = 0;
	Effect_Package effect;
	effect.name = NO_EFFECT;

	/* identify where the macro starts */
	while( element[startof_macro] != '[' ) {
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
		}

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
								else if( buffer[i][j] == '*' && buffer[i][j+1] == '\0' ) continue;
								else { status = i+1; break; }
							}
						}
					}
				} else if( buffer[i][time_pos] == '*' && buffer[i][time_pos+1] == '\0' ) continue;
				  else { status = i+1; break; }
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
						if( buffer[i][j] == '.' ) time_mod = '.'; 
						else { status = i+1; break; }
					}
				}	
			}
		} else if( buffer[i][1] == '\n' ) continue;
		else if( buffer[i][0] == '-' ) {
			if( i == no_buffer_elements-1 || i == 0 ) {
				fprintf(stderr, ANSI_BOLD "Note: " ANSI_RESET "a tie must be preceded and followed by another note.\n");
				status = i+1; break; 
			}

			char prev_note[3]; 
			char next_note[3];
			int next_note_index = i+1;
			int last_note_index = i-1;
			while ( buffer[next_note_index][0] == ')' || buffer[next_note_index][0] == '^' ) {
				next_note_index++;
			}
			while ( buffer[last_note_index][0] == ')' || buffer[last_note_index][0] == '^' ) {
				last_note_index--;
			}
			next_note[2] = prev_note[2] = '\0';
			if( strlen(buffer[next_note_index]) <= octave_pos ) { status=next_note_index; break;}
			for( int k = 0; k <= octave_pos; k++ ) {
				prev_note[k] = buffer[last_note_index][k]; 
				next_note[k] = buffer[next_note_index][k];
			}

			if( strcmp(prev_note, next_note) ) { 
				fprintf(stderr, ANSI_BOLD "Note: " ANSI_RESET "tied notes must be the same pitch.\n");
				status = i+1; break; 
			}
		} else if( buffer[i][0] == '^' ) {
			int k;
			for( k = 0; k < strlen(buffer[i])-1; k++ ) {
				if( buffer[i][k] != '^' ) { status = i+1; break; }
			}
			if( buffer[i][k] != '(' ) { status = i+1; break; }
			continue;
		} else if( buffer[i][0] == ')' ) continue;
		else { status = i+1; break; }
	}
	return status;
}

unsigned int validate_macrodef(int no_buffer_elements, char **buffer, int macro_type, char *line, int line_number) {
	char mtype[24]; memset(mtype, '\0', 24);
	char open_char, close_char;
	if( macro_type == CUS_MDEF ) { strncat(mtype, "text", 20);            open_char =      close_char = '\"'; }
	else                         { strncat(mtype, "custom arpeggio", 20); open_char = '['; close_char = ']';  }
	if( no_buffer_elements < 3 ) {
		fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "%s macro missing definition.\n", line_number, mtype);
		print_error_line(line, line_number, buffer[1]);
		return FALSE;
	} else if( buffer[2][0] != open_char || buffer[no_buffer_elements-1][strlen(buffer[no_buffer_elements-1])-1] != close_char ) {
		fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "%s macro incorrectly defined.\n", line_number, mtype);
		print_error_line(line, line_number, line);
		return FALSE;
	}

	if( macro_type == CUS_MDEF ) return TRUE;

	int i = 2;
	int no_temp_elements = no_buffer_elements-2;
	if( strlen(buffer[2]) == 1 ) { no_temp_elements--; i++; }
	if( strlen(buffer[no_buffer_elements-1]) == 1 ) no_temp_elements--;

	char **temp_buffer = malloc(no_temp_elements*sizeof(char*));
	int temp_start = i;
	for( i; i < no_temp_elements+temp_start; i++ ) {
		temp_buffer[i-temp_start] = malloc(32*sizeof(char));
		memset(temp_buffer[i-temp_start], '\0', 32);
		if( i == 2 ) {
			int j = 1;
			while( buffer[i][j] != '\0' ) {
				temp_buffer[i-temp_start][j-1] = buffer[i][j];
				j++;
			}
			temp_buffer[i-temp_start][j] = '\0'; 
		} else strncpy(temp_buffer[i-temp_start], buffer[i], 32);
	}

	int exit = TRUE;
	for( i = 0; i < no_temp_elements; i++ ) {
		if( atoi(temp_buffer[i]) == 0 && strcmp(temp_buffer[i], "0") ) { 
			fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "arpeggio macro definition contains invalid parameters.\n", line_number);
			print_error_line(line, line_number, temp_buffer[i]);
			exit = FALSE;
		}
	}

	free_array(temp_buffer, no_temp_elements);
	return exit;
}

unsigned int get_no_elements(char *string) {
	/* helps resolve floating spaces at the ends of lines */
	int back_index = strlen(string)-1;
	while( string[back_index] == ' ' ) { 
		string[back_index] = '\0';
		back_index--;
	}
	/* identify number of elements in the line separated by spaces */
	int no_line_elements = 0;
	for( int i = 0; i < strlen(string); i++ ) {
		if( string[i] == COMMENT_CHAR && string[i-1] == ' ' ) { no_line_elements--; break; }
		else if( string[i] == ' ' ) { 
			no_line_elements++;
			while( string[i+1] == ' ' ) { i++; }
		}
	}
	return ++no_line_elements;
}

char **get_elements_array(char *line, int no_elements) {
	char **line_elements = malloc(no_elements*sizeof(char*));
	int pos_in_line = 0;
	for( int i = 0; i < no_elements; i++ ) {
		line_elements[i] = malloc(32*sizeof(char));
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
	return line_elements;
}

unsigned int get_line_buffer(char *line, int line_number, char ***buffer, int *buffer_elements) {
	int no_line_elements = get_no_elements(line);
	char **line_elements = get_elements_array(line, no_line_elements);

	/* this switch handles checking the line for commands first */
	int stat = parse_command_or_macrodef(line, line_number);
	switch ( stat ) {
		case NONE:
			break;
		case COMMAND:
			*buffer = line_elements;
			*buffer_elements = no_line_elements;
			return COMMAND;
		case FAILED:
			*buffer = line_elements;
			*buffer_elements = no_line_elements;
			return FAILED;
		case CUS_MDEF:
		case ARP_MDEF:
			int valid = validate_macrodef(no_line_elements, line_elements, stat, line, line_number);	
			if( !valid ) {
				*buffer = line_elements;
				*buffer_elements = no_line_elements;
				stat = FAILED;
			} else { //free_array(line_elements, no_line_elements);
				for( int i = 0; i < no_line_elements; i++ ) { free(line_elements[i]); }
				free(line_elements);
			}

			return stat;
	}

	/* it is now presumed the line specifies notes */
	int exit_status = NOTE;
	exit_status = expand_cus_macro(&line_elements, &no_line_elements, line, line_number, Cus_Macros);
	if( exit_status == FAILED ) {
		*buffer = line_elements;
		*buffer_elements = no_line_elements;
		return FAILED;
	}

	char **temp_buffer = malloc(no_line_elements*sizeof(char*));
	for( int i = 0; i < no_line_elements; i++ ) {
		temp_buffer[i] = malloc(32*sizeof(char));
		memset(temp_buffer[i], '\0', 32);
		strncpy(temp_buffer[i], line_elements[i], 32);
	}
	stat = validate_buffer(no_line_elements, temp_buffer);
	free_array(temp_buffer, no_line_elements);
	if( stat != NORMAL ) {
		fprintf(stderr, ANSI_BOLD "Error on line %i: " ANSI_RESET "illegal character(s) in element.\n", line_number);
		print_error_line(line, line_number, line_elements[stat-1]);
		*buffer = line_elements;
		*buffer_elements = no_line_elements;
		return FAILED;
	}

	int no_old_elements = no_line_elements;
	exit_status = expand_parens(line_elements, &no_line_elements, line, line_number);
	for( int i = 0; i < no_old_elements; i++ ) {
		if( line_elements[i][0] == '\0' ) free(line_elements[i]);
	}

	*buffer = line_elements;
	*buffer_elements = no_line_elements;
	return exit_status;
}

Note_Node *convert_from_string(char *string, Key_Map *keymap, int **freq_table, double tempo) {
	Note_Node *int_rep = malloc(sizeof(Note_Node));

	char pitch[3] = "";
	int i = 0;
	while( string[i] != 'o' 
			&& string[i] != '^' 
			&& string[i] != '.' 
			&& string[i] != ',' 
			&& string[i] != '*'
			&& string[i] != '\0' ) {	
		pitch[i] = string[i];
		i++;
	}
	pitch[3] = '\0';
	
	int k = 0;
	while( keymap[k].name != NULL ) {
		if( pitch[0] == keymap[k].name[0] && 
				( pitch[1] != '#' && pitch[1] != 'b' && pitch[1] != 'n' ) ) {
				pitch[2] = pitch[1]; 
			  pitch[1] = keymap[k].accidental; 
		}
		k++;
	}

	int_rep->frequency = get_freq_from_string(pitch, freq_table);

	int string_len = strlen(string);
	char time[32];
	memset(time, '\0', 32);
	int j = 0;
	while( i < string_len ) {
		time[j] = string[i];
		i++; j++;
	}

	int_rep->duration = get_duration_from_string(time, tempo);

	if( string[string_len-1] == '*' ) {
		if( int_rep->duration-Staccato_Time > 0 ) int_rep->duration -= Staccato_Time;
		else string[string_len-1] = ' '; /* negates staccato if a rest greater than the length of the note is called for */
	}

	return int_rep;
}

void buffer_to_intrep(char **buffer, int buff_size, Note_Node **start, Note_Node **tail, Key_Map *keymap, int **freq_table, double tempo) {
	Effect_Package effect;
	Note_Node *int_rep;

	for( int i = 0; i < buff_size; i++ ) {
		effect = parse_effects_macros(buffer[i]);

		if( effect.name ) {
			/* param1 is the same for all effects */
			Note_Node *base_note = convert_from_string(buffer[i], keymap, freq_table, tempo);
			effect.param1 = *base_note;
			if( effect.name == ARPEGGIO ) {
				int base_hsteps = hsteps_from_A4(effect.param1.frequency);
				effect.param2 = round_dbl(calc_freq(base_hsteps+effect.param2));
				effect.param3 = round_dbl(calc_freq(base_hsteps+effect.param3));
			}

			fx_macro(start, tail, effect);
			free(base_note);

		} else if( buffer[i][0] == '-' ) {
			i++;
			Note_Node *temp_rep = convert_from_string(buffer[i], keymap, freq_table, tempo);
			(*tail)->duration += temp_rep->duration;
			free(temp_rep);

		/* regular case */
		} else {
			int_rep = convert_from_string(buffer[i], keymap, freq_table, tempo);
			add2end(start, tail, int_rep);	
		}		

		if( buffer[i][strlen(buffer[i])-1] == '*' ) {
			int_rep = malloc(sizeof(Note_Node));
			int_rep->frequency = FALSE;
			int_rep->duration  = Staccato_Time;
			add2end(start, tail, int_rep);
		}
	}
}

void write_to_file(Note_Node *representation, FILE *outfile, Note_Node *tail) {
	Note_Node *temp = representation;
	fprintf(outfile, "#!/bin/sh\nbeep ");
	fprintf(outfile, "-f %d -l %f ", temp->frequency, temp->duration);
	temp = temp->next;

	/* list traversal */
	int silent_time = FALSE;
	while( temp != tail ) {
		if( temp->frequency == FALSE ) {
			silent_time += temp->duration;
		} else { 
			fprintf(outfile, "-D %i \\\n", silent_time);
			fprintf(outfile, "-n -f %d -l %f ", temp->frequency, temp->duration);
			silent_time = FALSE;
		}
		temp = temp->next;
	}
	fprintf(outfile, "-D %i \\\n", silent_time);
	fprintf(outfile, "-n -f %d -l %f -D 0\n", temp->frequency, temp->duration);
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
	Arp_Macros = Cus_Macros = NULL;

	/* initializing buffer */
	char **buffer = NULL;
	int elements = 0;
	int no_buff_elements;

	/* this loop cycles through each line of input, builds a
	 * buffer from it, then converts it into an intermediate
	 * representation */
	int exit = NORMAL_EXIT;
	while( 1 ) {
		fflush(stdin);
		line_number++;
		stat = fgets(line, 256, infile);
		if( stat == NULL ) break; /* breaks at EOF */
		if( line[0] == '\n' || line[0] == COMMENT_CHAR ) continue; 
		line[strlen(line)-1] = '\0'; 

		elements = get_line_buffer(line, line_number, &buffer, &no_buff_elements);
		if( elements == FAILED ) { 
			if( infile == stdin ) { 
				free_array(buffer, no_buff_elements);
				continue;
			} else { exit = ERROR_EXIT; break; }
		} else if( elements == COMMAND ) { 
			if     ( !strcmp(buffer[1], "tempo") )    command_tempo(atoi(buffer[2]), &Tempo);
			else if( !strcmp(buffer[1], "key") )      command_key(buffer[2], buffer[3], *Key_Str, key);
			else if( !strcmp(buffer[1], "arprate") )  command_arprate(atoi(buffer[2]), &Arpeggio_Rate);
			else if( !strcmp(buffer[1], "staccato") ) command_staccato(atof(buffer[2]), &Staccato_Time);

			free_array(buffer, no_buff_elements);
			continue;
		} else if( elements == ARP_MDEF ) {
			store_macro(line, &Arp_Macros);
			continue;
		} else if( elements == CUS_MDEF ) {
			store_macro(line, &Cus_Macros);
			continue;
		}
		/* building intermediate representation from buffer */	
		buffer_to_intrep(buffer, no_buff_elements, &Notes_Array, &tail, *key, freq_table, Tempo);
		free_array(buffer, no_buff_elements);
	}

	if( elements == FAILED && outfile != stdout ) {
		free_array(buffer, no_buff_elements);
		fprintf(stderr, "Input not succesfully written to file.\n");
		exit = ERROR_EXIT;
	}
	else if( Notes_Array != NULL ) write_to_file(Notes_Array, outfile, tail);

	free(line);
	free_list(&Notes_Array, &tail); 
	m_free_list(&Arp_Macros);
	m_free_list(&Cus_Macros);
	return exit;
}
