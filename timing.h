#ifndef timing_include
#define timing_include

/********************************************************************
 * Header file for all things timing related in making note 
 * translations
 ********************************************************************/

/* Constants for use in calculating the length of a note */
#define WHOLE 4.0
#define HALF  2.0
#define EIGTH 0.5

/* Recursively finds the duration of a note that includes any subdivisions beneath
 * the quarter note.
 * IN: The string of time modifiers, #the index therein, and the note's current length
 * OUT: The note's new duration in seconds */
double get_subdivision(char *time_mods, int index, double duration);

/* Finds the duration in seconds that a note should be played
 * IN: string form of human-readable time modifications, e.g.
 *     ",.", "^^", etc. and current tempo
 * OUT: duration in seconds that time modification gives */
double get_duration_from_string(char *time_mods, double tempo);

#endif
