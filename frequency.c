#include <math.h>
#include <string.h>

#include "frequency.h"
#include "parse.h"

double calc_freq(int hsteps_from_A4) {
	/* uses equation of the form:
	 * new_freq = known_freq * (a)^(half steps from known_freq) */
	return A4 * pow(CONST_A, hsteps_from_A4);
}

int round_dbl(double n) {
	int rounded;
	if( fabs(n) - abs(n) > 0.5 ) { 
		if( n > 0 ) rounded = (int)(n + 0.5);
		else        rounded = (int)(n - 0.5);
	}
	else rounded = (int)n;

	return rounded;
}

int hsteps_from_A4(int frequency) {
	double steps = log10((double)frequency/A4)/log10((double)CONST_A);
	return round_dbl(steps);
}


int** gen_freq_table(double tuning_A) {
	int no_octaves = ROWS_IN_TABLE - 1;
	int** freq_table = malloc((ROWS_IN_TABLE+1)*sizeof(int*));
	for( int octave = 0; octave <= no_octaves; octave++ ) {
		freq_table[octave] = malloc(12*sizeof(int));

		for( int i = 0; i < 12; i++ ) {
			 /* C | C#/Db | D | D#/Eb | E | F | F#/Gb | G | G#/Ab | A | A#/Bb | B
			  * (12 elements) */

			int oct_diff = octave - TUNING_A_OCTAVE;
			int hsteps = (oct_diff * 12) + (i - 9);
			freq_table[octave][i] = round_dbl(calc_freq(hsteps));
			}
		}

	/* Assembling the final extra array, for use in referencing when translating */
	char letters[12] = {'C', 0, 'D', 0, 'E', 'F', 0, 'G', 0, 'A', 0, 'B'};
	/* REMEMBER TO FREE THE FREQ TABLE FROM MEMORY IN MAIN */
	freq_table[ROWS_IN_TABLE] = malloc(12*sizeof(int));
	for( int i = 0; i < 12; i++ ) {
		freq_table[ROWS_IN_TABLE][i] = letters[i];
	}

	return freq_table;
}

int get_freq_from_string(char note[3], int** table) {
	/* using the reference row in the frequency table to find the correct note */
	if( note[0] == 'r' ) return 1;
	int note_index;
	for( int i = 0; i < 12; i++ ) {
		if( note[0] == table[ROWS_IN_TABLE][i] ) { note_index = i; break; }
	}

	/* The accidental is set to 0, -1, or 1, depending on how many half steps it alters the note by */
	int accidental = 0;
	int octave;
	if( strlen(note) == 3 ) {
		switch( note[1] ) {
			case '#':
				accidental = 1;
				break;
			case 'b':
				accidental = -1;
				break;
		}
		octave = note[2] - 48;
	} else octave = note[1] - 48;

	/* Modifying the index of the note, based upon any attached accidentals */
	note_index += accidental;

	return table[octave][note_index];
}

	
void *gen_key_sig(char *key_string) {
	/* pointer to return values */
	void *out_ptr;
	int error = ARG_ERROR;
	int empty_struct = NORMAL;

	/* tokenizing input */
	if( strlen(key_string) < 3 ) { out_ptr = &error; return out_ptr; }
	char key_string_cpy[4]; strncpy(key_string_cpy, key_string, 4);
	const char delim[] = " ";
	char *key_array[2]; 
	key_array[0] = strtok(key_string_cpy, delim);
  key_array[1] = strtok(NULL, delim);
	
	/* error checking input second argument */
	if( key_array[1] == NULL || (strcmp(key_array[1], "M") && strcmp(key_array[1], "m"))) {
		out_ptr = &error;
		return out_ptr;
	}

	/* generating key names to compare against */
	int no_alt_keys;
	int no_accidentals = -1;
	int key_index;
	char **key_names;
	struct Alt_Key *alt_keys;
	if( key_array[1][0] == 'M' ) {
		char *temp[] = {
			"C", "G", "D", "A" , "E" , "B" , "F#", "Db", "Ab", "Eb", "Bb", "F"
		}; key_names = temp;
		no_alt_keys = 3;	
		alt_keys = malloc(no_alt_keys*sizeof(struct Alt_Key));
		alt_keys[0].name = "Cb"; alt_keys[0].index = 5;
		alt_keys[1].name = "Gb"; alt_keys[1].index = 6;
		alt_keys[2].name = "C#"; alt_keys[2].index = 7;
	} else if( key_array[1][0] == 'm' ) {
		char *temp[] = {
			"A", "E", "B", "F#", "C#", "G#", "D#", "Bb", "F" , "C" , "G" , "D"
		}; key_names = temp;
		no_alt_keys = 1;
		alt_keys = malloc(no_alt_keys*sizeof(struct Alt_Key));
		alt_keys[0].name = "Eb"; alt_keys[0].index = 6;
	}
	
	for( int i = 0; i < 12; i++ ) {
		if( !strcmp(key_array[0], key_names[i]) ) { 
			no_accidentals = i - 2*((i*(i/7)) % 6);
			key_index = i;
			break; }
	}
	if( no_accidentals == -1 ) {
		for( int i = 0; i < no_alt_keys; i++ ) { 
			if( !strcmp(key_array[0], alt_keys[i].name) ) {
				int j = alt_keys[i].index;
				no_accidentals = j - 2*((j*(j/7)) % 6);
				key_index = j;
				break;
			}
		}
	}
	/* error case for key name */
	if( no_accidentals == -1 ) {
		free(alt_keys);
		out_ptr = &error;
		return out_ptr;
	}

	/* case: we are dealing with sharps */
	char **notes_in_sig;
	char acc_symbol;
	if( key_index <= 6 ) { 
		char *temp[] = { "F", "C", "G", "D", "A", "E" };
		notes_in_sig = temp;
		acc_symbol = '#';
	}
	/* other case, we are dealing with flats */
	else {
		char *temp[] = { "B", "E", "A", "D", "G", "C" };
		notes_in_sig = temp;
		acc_symbol = 'b';
	}

	Key_Map *keymap = malloc((no_accidentals + 1)*sizeof(Key_Map));
	for( int i = 0; i < no_accidentals; i++ ) {
		keymap[i].name = notes_in_sig[i];
		keymap[i].accidental = acc_symbol;
	}
	/* setting the final name to NULL signals the end of the keymap
	 * for neat iteration */
	keymap[no_accidentals].name = NULL; 

	free(alt_keys);

	out_ptr = keymap;
	return out_ptr;
}
