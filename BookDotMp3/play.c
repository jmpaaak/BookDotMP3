// play.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "play.h"

void playMp3File(char* mp3FullPathName) {
	char system_command[300];
	char mp3_player_name[300] = "mplayer";
	
	sprintf(system_command, "./%s %s", \
		mp3_player_name, mp3FullPathName);

	system(system_command);
}

