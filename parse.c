#include <string.h>
#include <stdio.h>

#include "parse.h"
#include "semantics.h"
#include "commands.h"
#include "frequency.h"
#include "timing.h"

#define ANSI_RED      "\x1b[1;31m"
#define ANSI_CYAN     "\x1b[36m"	
#define ANSI_BOLD     "\x1b[1m"
#define ANSI_RESET    "\x1b[0m"

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
void free_list(Note_Node **list, int elements) {
	for( int i = 0; i < elements; i++ ) {
		Note_Node *temp = *list;
		*list = (*list)->next;
		free(temp);
	}
}

int parse_cmdline(char **cmdline_args, int no_args, FILE **in, FILE **out, char **out_name) {
	int in_flag = 0;
	int out_flag = 0;
	int error_flag = 0;

	for( int i = 1; i < no_args; i++ ) {
		if( !strcmp(cmdline_args[i], "-o") ) {
			i++;
			*out = fopen(cmdline_args[i], "w");
			*out_name = cmdline_args[i];
			out_flag = 1;
		} else if( !strcmp(cmdline_args[i], "-f") ) {
			i++;
			*in = fopen(cmdline_args[i], "r");
			if( *in == NULL ) { 
				printf("Specified input file does not exist.\n"); 
				return 1;
			}
			in_flag = 1;
		} else if( !strcmp(cmdline_args[i], "-s") ) {
			*in = stdin;
			in_flag = 1;
		} else if( !strcmp(cmdline_args[i], "-u") ) {
			*out = stdout;
			out_flag = 1;
		} else error_flag = 1;
	}

	if( !in_flag || !out_flag || error_flag ) {
		printf("Usage: beepcomp [input mode] [output mode]\n");
		printf("Input modes:\n\t-f [infile]  - specify a file to read notes from.\n\t-s           - specify input from stdin.\n");
		printf("Output modes:\n\t-o [outfile] - specify a file to output beep commands to.\n\t-u           - specify output to stdout.\n");
		return 1;
	}
	else return 0;
}

int frmtcmp(char *str, char *frmt) {
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

	if( no_str_words != no_frmt_words ) return 1;

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
		// validity_flag points to whether the passed string matches the format
		// 1 denotes matching, 0 denotes not matching
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
		
		if( validity_flag == 0 ) { free(frmt_words); free(str_words); return 1; }
	}

	free(frmt_words);
	free(str_words);
	return 0;
}
	
int argchecker(char *argument, const char *format) {
	char temp_format[128];
	strncpy(temp_format, format, 128);
	int stat = frmtcmp(argument, temp_format);
	if( stat == 1 ) return ARG_ERROR;
	else return NORMAL;
	/* BEGIN EDITED PORTION */
	//int stat = 
}

int validate_command(char *command, char *argument) {
	int i = 0;
	while( commands[i] != "\0" ) {
		// strcmp is negated because it returns zero if it identifies a match
		if( !strcmp(command, commands[i] )) { return argchecker(argument, commands[i+1]); }
		i += 2;
	}
	return NAME_ERROR;
}

void print_error_line(char *line, int line_no, char *error_string) {
	// substring matching to identify where in the line the error occurs
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
				
	// getting strings to represent the parts of the line before and after the error
	char before_error[128];
	char after_error[128] = "";
	strncat(before_error, line, error_index);
	int j = 0;
	if( error_index + strlen(error_string) != strlen(line) ) {
		for( int i = error_index+strlen(error_string); i < strlen(line); i++ ) { 
			after_error[j++] = line[i]; 
		}
	}	

	printf("[%03i:%03i] ", line_no, error_index+1);
	for( int i = 0; i < error_index; i++ ) { putchar(line[i]); }
	printf(ANSI_RED);
	for( int i = error_index; i < error_index+strlen(error_string); i++ ) { putchar(line[i]); }
	printf(ANSI_RESET);
	for( int i = error_index + strlen(error_string); i < strlen(line); i++ ) { putchar(line[i]); }
	printf("\n");
	for( int i = 0; i < error_index+10; i++ ) { putchar(' '); }
	printf(ANSI_CYAN);
	putchar('^');
	for( int i = 0; i < strlen(error_string)-1; i++ ) { putchar('~'); }
	printf(" HERE\n" ANSI_RESET);
}

int parse_for_command(char *line, int line_number) {
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
			case NAME_ERROR:
				printf(ANSI_BOLD "Error on line %i: " ANSI_RESET "command \"%s\" does not exist.\n", line_number, cmd_name);
				fflush(stdout);
				print_error_line(line, line_number, cmd_name);
				return FAILED;
				break;
			case ARG_ERROR:
				printf(ANSI_BOLD "Error on line %i: " ANSI_RESET "invalid argument(s) given for command \"%s\".\n", line_number, cmd_name);
				fflush(stdout);
				print_error_line(line, line_number, argument);
				return FAILED;
				break;
			case NORMAL:
				return COMMAND;
				break;
		}
	}
	else 
		return NONE;
}

int validate_buffer(int no_buffer_elements, char **buffer) {
	int status = NORMAL;
	int i;
	int time_pos;
	for( i = 0; i < no_buffer_elements; i++ ) {
		int octave_pos = 1;
		if( buffer[i][0] >= 'A' && buffer[i][0] <= 'G' ) {
			if( buffer[i][1] == '#' || buffer[i][1] == 'b' || buffer[i][1] == 'n' ) {
				octave_pos = 2;
			}
			if( buffer[i][octave_pos] >= 1+48 && buffer[i][octave_pos] <= ROWS_IN_TABLE+47 ) {
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
		} else { status = i+1; break; }
	}
	return status;
}

int get_line_buffer(char *line, int line_number, char ***buffer, int *buffer_elements) {
	/* Note structure: Ab4^^
	 * A       b            4         ^^ 
	 * Note    Accidental   Octave    Time mod(s)
	 *         (optional)							(optional) */

	/* Structure for paren-enclosed timing mods:
	 * ^^( Ab4 Ab4 C4 D4 ) 
	 *    ^             ^
	 *    |             |
	 * Parens at this time must be delineated by spaces! */

	/* A comment character must be preceded by a space or newline in order to be properly parsed */

	/* Order of operations:
	 * 1. Read the line into a buffer of strings delimited by spaces
	 * 2. Read buffer, find any notes enclosed by parentheses preceded by a time mod, remove the
	 *    parens and time mod, and append it to each note individually
	 * 3. Gracefully exit and report errors if any should appear */

	const char delim[] = " ";

	int back_index = strlen(line)-1;
	while( line[back_index] == ' ' ) { 
		line[back_index] = '\0';
		back_index--;
	}

	// building the buffer - commented phrases are also identified, and ignored if present
	int no_line_elements = 0;
	for( int i = 0; i < strlen(line); i++ ) {
		if( line[i] == COMMENT_CHAR && line[i-1] == ' ' ) { no_line_elements--; break; }
		else if( line[i] == ' ' ) no_line_elements++;
	}
	no_line_elements++;

	char **line_elements = (char**)malloc(no_line_elements*sizeof(char*));
	int pos_in_line = 0;

	switch ( parse_for_command(line, line_number) ) {
		case COMMAND:
			for( int i = 0; i < no_line_elements; i++ ) {
				line_elements[i] = (char*)malloc(32*sizeof(char));
				memset(line_elements[i], '\0', 32);

				int element_index = 0;
				while( line[pos_in_line] != ' ' && line[pos_in_line] != '\0' && line[pos_in_line] != '\n' ) {
					line_elements[i][element_index] = line[pos_in_line];
					element_index++;
					pos_in_line++;
				}
				pos_in_line++;
				line_elements[i][element_index+1] = '\0';
			}
			*buffer = line_elements;
			*buffer_elements = no_line_elements;
			return COMMAND;
		case FAILED:
			free(line_elements);
			return FAILED;
		case NONE:
			break;
	}

	char local_time_mod[8] = "";
	int open_paren = 0; // set to 1 if the compiler has found a valid open paren
	int close_paren = 0; // set to 1 if the compiler has found a valid close paren

	/* assembling the buffer, separated by spaces
	 * this block of code also applies parenthesis expansion and comment ignorance */
	for( int i = 0; i < no_line_elements; i++ ) {
		line_elements[i] = malloc(32*sizeof(char));
		memset(line_elements[i], '\0', 32);

		int element_index = 0;
		while( line[pos_in_line] != ' ' && line[pos_in_line] != '\0' && line[pos_in_line] != '\n' ) {
			line_elements[i][element_index] = line[pos_in_line];
			element_index++;
			pos_in_line++;
		}
		pos_in_line++;
		line_elements[i][element_index+1] = '\0';

		if( line_elements[i][0] == '^' ) {
			int j = 0;
			while( line_elements[i][j] == '^' ) {
				char temp[2];
				temp[0] = line_elements[i][j];
				temp[1] = '\0';
				strcat(local_time_mod, temp);
				j++;
			}
			if( line_elements[i][j] != '(' || j != strlen(line_elements[i])-1 ) { 
				printf(ANSI_BOLD "Error on line %i: " ANSI_RESET "illegal character in time mod string.\n", line_number);
				print_error_line(line, line_number, line_elements[i]);
				free(line_elements);
				return FAILED;
			} else if( open_paren == 1 ) {
				printf(ANSI_BOLD "Error on line %i: " ANSI_RESET "new parenthesis pair begins before closing previous.\n", line_number);
				print_error_line(line, line_number, line_elements[i]);
				free(line_elements);
				return FAILED;
			} else {
				free(line_elements[i]);
				--i;
				no_line_elements--;
				open_paren = 1;
				close_paren = 0;
			}

		} else if( line_elements[i][0] == ')' ) {
			if( strlen(line_elements[i]) == 1 && open_paren == 1 ) { 
				free(line_elements[i]);
				--i;
				no_line_elements--;
				close_paren = 1;
				open_paren = 0;
				strcpy(local_time_mod, ""); 
			} else if( open_paren == 0 ) {
				printf(ANSI_BOLD "Error on line %i: " ANSI_RESET "unmatched close-parenthesis.\n", line_number);
				print_error_line(line, line_number, line_elements[i]);
				free(line_elements);
				return FAILED;
			} else {
				printf(ANSI_BOLD "Error on line %i: " ANSI_RESET "illegal character in close-paren clause.\n", line_number);
				print_error_line(line, line_number, line_elements[i]);
				free(line_elements);
				return FAILED;
			}

		} else strcat(line_elements[i], local_time_mod);
		  
	}
	if( open_paren == 1 && close_paren == 0 ) {
		printf(ANSI_BOLD "Error on line %i: " ANSI_RESET "unmatched open-parenthesis.\n", line_number);
		print_error_line(line, line_number, "(");
		free(line_elements);
		return FAILED;
	}

	int stat = validate_buffer(no_line_elements, line_elements);
	if( stat != NORMAL ) {
		printf(ANSI_BOLD "Error on line %i: " ANSI_RESET "illegal character(s) in note.\n", line_number);
		print_error_line(line, line_number, line_elements[stat-1]);
		free(line_elements);
		return FAILED;
	}

	*buffer = line_elements;
	*buffer_elements = no_line_elements;
	return NOTE;
}


Note_Node *convert_from_string(char *string, Key_Map *keymap, double **freq_table, double tempo) {
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

void alt_write_to_file(Note_Node *representation, FILE *outfile, int elements) {
	Note_Node *temp = representation;
	int start = 1;
	
	fprintf(outfile, "#!/bin/sh\nbeep ");
	for( int i = 0; i < elements; i++ ) {
		if( temp->frequency == 0 ) temp->frequency = 1;
		if( start == 1 ) fprintf(outfile, "-f %f -l %f \\\n", temp->frequency, temp->duration);
		else fprintf(outfile, "-n -f %f -l %f \\\n", temp->frequency, temp->duration);
		start = 0;
		
		temp = temp->next;
	}
}
