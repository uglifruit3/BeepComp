#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "parse.h"
#include "effects.h"
#include "frequency.h"
#include "timing.h"

unsigned short int hexchar_to_dec(char hexchar) {
	if     ( hexchar >= '0' && hexchar <= '9' )
		return hexchar - 48;
	else if( hexchar >= 'A' && hexchar <= 'F' )
		return hexchar - 55;
	else return 16;
}

void expand_arpeggio(Note_Node **start, Note_Node **tail, Effect_Package effect) {
	double dbl_total_notes = ((double)effect.param4/1000.0) * effect.param1.duration;
	int total_notes = (int)dbl_total_notes;
	double note_len = effect.param1.duration/dbl_total_notes;

	/* accounts for uneven length of note */
	double final_note_len = note_len;
	if( dbl_total_notes - total_notes > 0.005 ) {
		final_note_len = note_len * (dbl_total_notes-total_notes);
		total_notes++;
	}

	int frequencies[3] = { effect.param1.frequency, effect.param2, effect.param3 };

	for( int i = 0; i < total_notes; i++ ) {
		Note_Node *int_rep = malloc(sizeof(Note_Node));
		/* cycles through the 3 arpeggio tones */
		int_rep->frequency = frequencies[i % 3];
		int_rep->duration = note_len - (note_len-final_note_len)*(i/(total_notes-1));
		add2end(start, tail, int_rep);	
	}
}
void expand_portamento(Note_Node **start, Note_Node **tail, Effect_Package effect) {
	printf(ANSI_BOLD "Note: " ANSI_RESET "due to limitations in interacting with the pc speaker, portamento is not supported at the moment. As such, invoking this effect will do nothing.\n");

	Note_Node *int_rep = malloc(sizeof(Note_Node));
	int_rep->frequency = effect.param1.frequency;	
	int_rep->duration = effect.param1.duration;	
	add2end(start, tail, int_rep);
}
void expand_vibrato(Note_Node **start, Note_Node **tail, Effect_Package effect) {
	printf(ANSI_BOLD "Note: " ANSI_RESET "due to limitations in interacting with the pc speaker, vibrato is not supported at the moment. As such, invoking this effect will do nothing.\n");

	Note_Node *int_rep = malloc(sizeof(Note_Node));
	int_rep->frequency = effect.param1.frequency;	
	int_rep->duration = effect.param1.duration;	
	add2end(start, tail, int_rep);
}
