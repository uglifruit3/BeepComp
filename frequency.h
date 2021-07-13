#ifndef freq_table_include
#define freq_table_include
/*******************************************************************
 * Header file for all things frequency
 * related
 *******************************************************************/

#include <string.h>
#include <math.h>
#include <stdlib.h>

#define A4 440.0 /* Note A right above middle C is 440 hz */
#define TUNING_A_OCTAVE 4
#define CONST_A 1.059463094
#define ROWS_IN_TABLE 10

/* Struct for storing information about key signatures.
 * For use in the functions gen_key_sigs and get_key_info_from string */
typedef struct {
	char *name;
	char accidental;
} Key_Map;

/* Struct for handling alternative names of certain key signatures */
struct Alt_Key {
	char *name;
	int index;
};

/* Calculates frequency
 * IN: The number of half steps the note is from A4
 * IN: The tuning frequency for A4
 *     This should almost ALWAYS be A4 (440), but is left 
 *     flexible for extending to nonconventional tunings
 * OUT: The note's frequency */
double calc_freq(int hsteps_from_A4, double tuning_A);

/* Builds the frequency table for reference in translating
 * entered notes into proper frequencies.
 * IN: Tuning A frequency (by default 440 hz)
 * OUT: Completed frequency table
 *
 * Table format:
 *
 * O     chromatically
 * C        ascending notes
 * T            For example:
 * A     C, C#/Db, D, ...
 * V     C, C#/Db, D, ...
 * E     ...
 * List of note Characters as the final element
 */
int** gen_freq_table(double tuning_A);

/* Takes a notescript note and returns its frequency by
 * parsing note format and identifying index in frequency table
 * IN: human-readable note (e.g. C3 or Bb6) and frequency table
 * OUT: note's corresponding frequency*/
int get_freq_from_string(char* note, int** table);

/* Takes a key signature specified by key_string and returns a pointer to
 * a key struct for use in compilation. Void pointer will also point to error codes. */
void *gen_key_sig(char *key_string);

#endif
