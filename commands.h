#ifndef commands_include
#define commands_include

#include "frequency.h"

void command_tempo(double new_tempo, double *tempo);

void command_time(char *new_time, char *time);

int command_key(char *new_key0, char *new_key1, char *key, Key_Map **keymap);

#endif
