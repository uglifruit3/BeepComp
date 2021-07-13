#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "frequency.h"
#include "parse.h"
#include "semantics.h"
#include "timing.h"
#include "commands.h"
#include "effects.h"

char *Key_Str;
int **freq_table;
Key_Map *key;

int main(int argc, char *argv[]) {
	unsigned int exit = 0;
	/* parsing command line invocation */
	FILE *infile;
	FILE *outfile;
	char *outfile_name;
	if( parse_cmdline(argv, argc, &infile, &outfile, &outfile_name) ) return 1;
	
	/* initializing default values */
	Key_Str = malloc(5*sizeof(char)); strncpy(Key_Str, "C M", 5);
	freq_table = gen_freq_table(A4);
	key = gen_key_sig(Key_Str);

	/* parsing all input and writing to output */
	unsigned int status = parse_infile(infile, outfile, &Key_Str, freq_table, &key);
	if( status == 1 ) exit = 1;

	/* changing file permissions to executable on output file */
	if( outfile != stdout && chmod(outfile_name, S_IRWXU) ) { 
		fprintf(stderr, "Error changing outfile permissions to executable.\n");
	} else if( outfile != stdout ) printf("All notes succesfully written to file. Exiting.\n");

	/* freeing all allocated memory */
	free(Key_Str);
	if( infile != stdin ) fclose(infile);
	if( outfile != stdout ) fclose(outfile);
	for( int i = 0; i < ROWS_IN_TABLE+1; i++ ) { free(freq_table[i]); }
	free(freq_table);
	free(key);
	return exit;
}
