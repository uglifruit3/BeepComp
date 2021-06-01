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
	char *Key_Str = malloc(5*sizeof(char)); strncpy(Key_Str, "C M", 5);
	int **freq_table = gen_freq_table(A4);
	Key_Map *key = gen_key_sig(Key_Str);

	/* parsing all input and writing to output */
	unsigned int status = parse_infile(infile, outfile, &Tempo, &Key_Str, freq_table, &key);
	if( status == 1 ) return 1;

	/* changing file permissions to executable on output file */
	if( outfile != stdout && chmod(outfile_name, S_IRWXU) ) { 
		printf("Error changing outfile permissions to executable.\n");
	} else if( outfile != stdout ) printf("All notes succesfully written to file. Exiting.\n");

	/* freeing all allocated memory */
	free(Key_Str);
	if( infile != stdin ) fclose(infile);
	if( outfile != stdout ) fclose(outfile);
	for( int i = 0; i < ROWS_IN_TABLE+1; i++ ) { free(freq_table[i]); }
	free(freq_table);
	free(key);
	return 0;
}
