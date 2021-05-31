#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "parse.h"
#include "frequency.h"
#include "semantics.h"
#include "timing.h"
#include "commands.h"

int main(int argc, char *argv[]) {
	int in_flag = 0;
	int out_flag = 0;
	int error_flag = 0;
	FILE *infile;
	FILE *outfile;
	char *outfile_name;

	for( int i = 1; i < argc; i++ ) {
		if( !strcmp(argv[i], "-o") ) {
			i++;
			outfile = fopen(argv[i], "w");
			outfile_name = argv[i];
			out_flag = 1;
		} else if( !strcmp(argv[i], "-f") ) {
			i++;
			infile = fopen(argv[i], "r");
			if( infile == NULL ) { 
				printf("Specified input file does not exist.\n"); 
				return 1;
			}
			in_flag = 1;
		} else if( !strcmp(argv[i], "-s") ) {
			infile = stdin;
			in_flag = 1;
		} else if( !strcmp(argv[i], "-u") ) {
			outfile = stdout;
			out_flag = 1;
		} else error_flag = 1;
	}

	if( !in_flag || !out_flag || error_flag ) {
		printf("Usage: beepcomp [input mode] [output mode]\n");
		printf("Input modes:\n\t-f [infile]  - specify a file to read notes from.\n\t-s           - specify input from stdin.\n");
		printf("Output modes:\n\t-o [outfile] - specify a file to output beep commands to.\n\t-u           - specify output to stdout.\n");
		return 1;
	}
		
	/* initializing default values */
	double Tempo = 90;
	char Key_Str[5] = "C M";
	double **freq_table = gen_freq_table(A4);
	Key_Map *key = gen_key_sig(Key_Str);

	/* initializing arrays */
	char *line = malloc(256*sizeof(char));
	char *stat;
	int line_number = 0;

	int linked_list_nodes = 0;
	Note_Node *Notes_Array = NULL;
	Note_Node *tail = NULL;
	Note_Node *int_rep;

	char **buffer = NULL;
	int elements= 0;
	int no_buff_elements;
	while( 1 ) {
		fflush(stdin);
		line_number++;
		stat = fgets(line, 256, infile);
		if( stat == NULL ) break;
		if( line[0] == '\n' || line[0] == COMMENT_CHAR ) continue;
		line[strlen(line)-1] = '\0';

		elements = get_line_buffer(line, line_number, &buffer, &no_buff_elements);
		if( elements == FAILED && infile == stdin ) continue;
		else if( elements == FAILED ) break;
		else if( elements == COMMAND ) { 
			if( !strcmp(buffer[1], "tempo") ) command_tempo(atoi(buffer[2]), &Tempo);
			else if( !strcmp(buffer[1], "key") ) {
				free(key);
				command_key(buffer[2], buffer[3], Key_Str, &key);
			}
			for( int i = 0; i < no_buff_elements; i++ ) { free(buffer[i]); 
			}
			free(buffer);
			continue;
		}
		
		for( int i = 0; i < no_buff_elements; i++ ) {
			int_rep = convert_from_string(buffer[i], key, freq_table, Tempo);
			add2end(&Notes_Array, &tail, int_rep);	
		}
		linked_list_nodes += no_buff_elements;
		for( int i = 0; i < no_buff_elements; i++ ) { free(buffer[i]); }
		free(buffer);
	}
	if( elements == FAILED ) printf("Input not succesfully written to file.\n");

	//alt_write_to_file(Notes_Array, outfile);
	alt_write_to_file(Notes_Array, outfile, linked_list_nodes);
	if( outfile != stdout && chmod(outfile_name, S_IRWXU) ) { 
		printf("Error changing outfile permissions to executable.\n");
	} else if( outfile != stdout ) printf("All notes succesfully written to file. Exiting.\n");

	free(line);
	if( infile != stdin ) { fclose(infile); }
	if( outfile != stdout ) { fclose(outfile); }
	free_list(&Notes_Array, linked_list_nodes); //edit
	//free(Notes_Array);
	for( int i = 0; i < ROWS_IN_TABLE; i++ ) { free(freq_table[i]); }
	free(freq_table);
	free(key);
	return 0;
}
