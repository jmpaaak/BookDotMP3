// speech.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "speech.h"

void callSpeechSynthesis(char* text, char* mp3FullPathName, \
		int page_number, int speaker_gender, int speed) {
	int i=0;
	char url[200] = "https://125.209.234.165/v1/voice/tts.bin";
	char client_id[200] = "bwViWdJYYtYi8bcWTyfT'";
	char client_secret[200] = "AiJe1PCeEb'";
	char content_type[200] =\
	"application/x-www-form-urlencoded; charset=UTF-8'";	

	char wget_prefix[200] = "./busybox wget";
	char header_1[200] = " --header 'X-Naver-Client-Id: ";
	char header_2[200] = " --header 'X-Naver-Client-Secret: ";
	char header_3[200] = " --header 'Content-Type: ";
	char headers[3][200];
	int header_count = 3;
	sprintf(headers[0], "%s%s", header_1, client_id);
	sprintf(headers[1], "%s%s", header_2, client_secret);
	sprintf(headers[2], "%s%s", header_3, content_type);

	char params[300];
	char speaker[50];
	char s_speed[10];

	if(speaker_gender == 0) {
		sprintf(speaker, "%s", "mijin");
	}
	else {
		sprintf(speaker, "%s", "jinho");
	}

	sprintf(s_speed, "%d", 5 - (speed));

	sprintf(params, " --post-data 'speaker=%s&speed=%s&text=", \
			speaker, s_speed);
	
	char data[3000];
	sprintf(data, "%s%s", \
			params, text);

	char response_type[400];
	sprintf(response_type, " -O %s",mp3FullPathName);

	char system_command[2000];
	sprintf(system_command, "%s%s%s%s%s%s' %s", \
		wget_prefix, headers[0], headers[1], headers[2],\
		response_type, data, url);

	printf("%s\n", system_command);
	system(system_command);
}
