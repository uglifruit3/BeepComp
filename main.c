#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "parse.h"
#include "frequency.h"
#include "semantics.h"
#include "timing.h"
#include "commands.h"

int main(int argc, char *argv[]) {
	/* parsing command line invocation */
	FILE *infile;
	FILE *outfile;
	char *outfile_name;
	if( parse_cmdline(argv, argc, &infile, &outfile, &outfile_name) ) return 1;

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

	/* writing to file and changing outfile to executable */
	alt_write_to_file(Notes_Array, outfile, linked_list_nodes);
	if( outfile != stdout && chmod(outfile_name, S_IRWXU) ) { 
		printf("Error changing outfile permissions to executable.\n");
	} else if( outfile != stdout ) printf("All notes succesfully written to file. Exiting.\n");

	/* freeing all allocated memory */
	free(line);
	if( infile != stdin ) fclose(infile); 
	if( outfile != stdout ) fclose(outfile); 
	free_list(&Notes_Array, linked_list_nodes); 
	for( int i = 0; i < ROWS_IN_TABLE; i++ ) { free(freq_table[i]); }
	free(freq_table);
	free(key);
	return 0;
}
