#include <stdio.h>
#include <stdlib.h>

#include "parse.h"
#include "frequency.h"
#include "semantics.h"
#include "timing.h"

int main(int argc, char *argv[]) {
	/* TODO:
	 * - write error reporting into validate_buffer
	 * - figure out how to condense routines in main
	 * - write file i/o support */
	double **freq_table = gen_freq_table(A4);
	Key_Map *key = gen_key_sig("C M");
	double Tempo = 60;

	char **buffer;
	int elements = get_line_buffer(argv[1], 1, &buffer);
	if( elements == 0 ) return 1;
	
	int stat = validate_buffer(elements, buffer);
	if( stat != NORMAL ) {
		printf("Error on line 1: illegal syntax or characters in note.\n");
		print_error_line(argv[1], 1, buffer[stat-1]);
		return 1;
	}

	Note_Node *Notes_Array = NULL;
	Note_Node *tail = NULL;
	Note_Node *int_rep;
	for( int i = 0; i < elements; i++ ) {
		int_rep = convert_from_string(buffer[i], key, freq_table, Tempo);
		add2end(&Notes_Array, &tail, int_rep);	
	}

	Note_Node *temp = Notes_Array;
	int i = 0;
	while( temp != NULL ) {
		printf("%s converts to: %f hz for %f ms.\n", buffer[i], temp->frequency, temp->duration);
		i++;
		temp = temp->next;
	}

	free_list(&Notes_Array);
	for( int i = 0; i < elements; i++ ) { free(buffer[i]); }
	free(buffer);
	for( int i = 0; i < ROWS_IN_TABLE; i++ ) { free(freq_table[i]); }
	free(freq_table);
	free(key);
	return 0;
}
