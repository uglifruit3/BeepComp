#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "timing.h"
#include "commands.h"
#include "parse.h"
#include "macros.h"

char *Key_Str;
int **freq_table;
Key_Map *key;

unsigned int read_input(FILE *infile, FILE *outfile, char **Key_Str, int **freq_table, Key_Map **key);

int main(int argc, char *argv[]) {
	unsigned int exit = NORMAL_EXIT;
	FILE *infile = NULL;
	FILE *outfile = NULL;
	char *outfile_name;
	exit = parse_cmdline(argv, argc, &infile, &outfile, &outfile_name);
	if( exit == ERROR_EXIT ) return ERROR_EXIT;
	else if( exit == HELP_EXIT ) return NORMAL_EXIT;
	
	Key_Str = malloc(5*sizeof(char)); strncpy(Key_Str, "C M", 5);
	freq_table = gen_freq_table(A4);
	key = gen_key_sig(Key_Str);

	unsigned int status = read_input(infile, outfile, &Key_Str, freq_table, &key);
	if( status == ERROR_EXIT ) exit = ERROR_EXIT;

	if( outfile != stdout && chmod(outfile_name, S_IRWXU) ) { 
		fprintf(stderr, "Error changing outfile permissions to executable.\n");
	} else if( outfile != stdout && exit == NORMAL_EXIT ) printf("All notes succesfully written to file. Exiting.\n");

	free(Key_Str);
	if( infile != stdin ) fclose(infile);
	if( outfile != stdout ) fclose(outfile);
	for( int i = 0; i < ROWS_IN_TABLE+1; i++ ) { free(freq_table[i]); }
	free(freq_table);
	free(key);

	return exit;
}

unsigned int read_input(FILE *infile, FILE *outfile, char **Key_Str, int **freq_table, Key_Map **key) {
	/* initializing line array (updated every iteration) */
	char *line = malloc(256*sizeof(char));
	char *stat;
	int line_number = 0;

	/* initializing linked list for intermediate rep */
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

	if( elements == FAILED && infile != stdin ) {
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


