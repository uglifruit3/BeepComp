#include <string.h>
#include <math.h>

#include "timing.h"

double get_duration_from_string(char *time_mods, double tempo) {
	/* The duration of any note starts off as exactly one quarter note's worth.
	 * If nothing is specified, the function returns a quarter note */
	const double beat_duration = 60.0 / tempo;	
	double note_duration = beat_duration;

	int no_dots = 1;
	double orig_note_duration = note_duration;

	int index = 0;
	while( time_mods[index] != '\0' ) {
		switch( time_mods[index] ) {
			case 'o': 
				note_duration = WHOLE * beat_duration; break;
			case ',': 
				note_duration = HALF * beat_duration; break;
			case '^': 
				note_duration *= 0.5; break;
			case '.': 
				if( no_dots == 1 ) orig_note_duration = note_duration;
				note_duration += powf(0.5,(float)no_dots) * orig_note_duration; 
				no_dots++;
				break;
		}
		index++;
	}

	return note_duration*1000;
}
