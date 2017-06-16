// speech.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void callSpeech(char* text, char* mp3FullPathName, int page_number) {
	int i=0;

	//char url[200] = "https://openapi.naver.com/v1/voice/tts.bin";
	char url[200] = "https://125.209.234.165/v1/voice/tts.bin";
	char client_id[200] = "FRGJBare95DPOfbfWggQ\"";
	char client_secret[200] = "a9NYpNg5Mr\"";
	char content_type[200] =\
	"application/x-www-form-urlencoded; charset=UTF-8\"";	

	char wget_prefix[200] = "./wget_static";
	char header_1[200] = " --header=\"X-Naver-Client-Id: ";
	char header_2[200] = " --header=\"'X-Naver-Client-Secret: ";
	char header_3[200] = " --header=\"Content-Type: ";
	char headers[3][200];
	int header_count = 3;
	sprintf(headers[0], "%s%s", header_1, client_id);
	sprintf(headers[1], "%s%s", header_2, client_secret);
	sprintf(headers[2], "%s%s", header_3, content_type);

	char params[100] =" --post-data=\"speaker=clara&speed=0&text=";
	char data[3000];
	sprintf(data, "%s%s", \
			params, text);

	char response_type[400];
	sprintf(response_type, " -O %s",mp3FullPathName);

	char system_command[2000];
	sprintf(system_command, "%s%s%s%s%s%s\" %s", \
		wget_prefix, headers[0], headers[1], headers[2],\
		response_type, data, url);

	printf("%s\n",system_command);
	system(system_command);
}


int main(void) {
	char* text = "Hello! Book Dot, MP3";

	char* mp3FullPathName = "./exam2.mp3";

	int page_number = 1;

	callSpeech(text, mp3FullPathName, page_number);

	system("mplayer exam2.mp3");

	return 0;
}

