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
