#include <string.h>

#include "semantics.h"
#include "timing.h"

double get_subdivision(char *time_mods, int index, double duration) {
	if( time_mods[index] != '^' ) return duration;
	get_subdivision(time_mods, ++index, duration * EIGTH);
}

//double get_duration_from_string(char time_mods[], double Tempo) {
double get_duration_from_string(char *time_mods, double tempo) {
	/* The duration of any note starts off as exactly one quarter note's worth.
	 * If nothing is specified, the function returns a quarter note */
	const double beat_duration = 60.0 / tempo;	
	double note_duration = beat_duration;

	/* List of modifiers:
	 * o  - whole note
	 * ,  - half note
	 * ^  - eight note
	 * ^^ - sixteenth... adding more sequentially further shortens
	 * .  - dot; similar to eigth note modifier, but progressively adds to note */
	
	int index = 0;
	while( time_mods[index] != '\0' ) {
		switch( time_mods[index] ) {
			case 'o': 
				note_duration = WHOLE * beat_duration; break;
			case ',': 
				note_duration = HALF * beat_duration; break;
			case '^': 
				//note_duration =  get_subdivision(time_mods, index, note_duration);
				note_duration *= 0.5; break;
			case '.': 
				//note_duration += get_subdivision(time_mods[index], index, note_duration);
				note_duration += 0.5 * note_duration; break;
		}
		index++;
	}

	return note_duration*1000;
}
