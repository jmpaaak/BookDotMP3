#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

void callAPI(char *imgContent) {

	char shellBuffer[300];

	/*** call google cloud vision api ***/
	FILE * fpReqJson = fopen("request-target.json", "w+b");       
	FILE * fpTempJson = fopen("request.json", "r+b");       

	int i;
	for(i=0; i < 5; i++) {
		fgets(shellBuffer, 300, fpTempJson); 
		fprintf(fpReqJson, "%s", shellBuffer); // write lines
	}

	fprintf(fpReqJson, "\"%s\"", imgContent); // write content 

	for(i=0; i < 9; i++) {
		fgets(shellBuffer, 300, fpTempJson); 
		fprintf(fpReqJson, "%s", shellBuffer); // write lines
	}

	rewind(fpReqJson); // move pointer to first of FILE

//	while(fgets(shellBuffer, 300, fpReqJson)) {
//		system(shellBuffer);
//	}

	fclose(fpReqJson);

	system("./vision.sh");  
	
	/*** parsing http response JSON ***/
	FILE * fpResJson = fopen("response.json", "rb");       
	char * resultTextRaw;
	char resultText[1000];
	char resultTextBuffer[1000]; 

	for(i=0; i < 7; i++) {
		fgets(resultTextBuffer, 1000, fpResJson); 	// skipping 6 lines and get next
	}

	resultTextRaw = strstr(resultTextBuffer, ":");
	resultTextRaw = strstr(resultTextRaw, "\"");
	strcpy(resultText, (&resultTextRaw[0])+1);		// skipping start "
	int textWithAtosLen = strlen(resultText);
	resultText[textWithAtosLen-3] = '\n';	// skipping end "
	resultText[textWithAtosLen-2] = '\0';	// skipping end "

	int indexResolved;
	for(i=0, indexResolved=0; i < strlen(resultText); i++) {
		if(resultText[i] != '\\' && resultText[i+1] != 'n') {
			resultText[indexResolved] = resultText[i];
		}
		else {
			resultText[indexResolved] = ' ';
			i++; // it cause i = i+2
		}
		indexResolved++;
	}
	resultText[indexResolved] = '\n';
	resultText[indexResolved+1] = '\0';

	printf("\n%s", resultText); // res texts

	fclose(fpTempJson);
	fclose(fpResJson);
}



static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table() {

	decoding_table = malloc(256);

	int i;
	for (i = 0; i < 64; i++)
		decoding_table[(unsigned char) encoding_table[i]] = i;
}

char *base64_encode(const unsigned char *data,
		size_t input_length,
		size_t *output_length) {

	*output_length = 4 * ((input_length + 2) / 3);

	char *encoded_data = malloc(*output_length);
	if (encoded_data == NULL) return NULL;

	int i, j;
	for (i = 0, j = 0; i < input_length;) {

		uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
		uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
		uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
			
	}

	for (i = 0; i < mod_table[input_length % 3]; i++)
		encoded_data[*output_length - 1 - i] = '=';

	return encoded_data;
}


void callGoogleVision(char *imageUrl) {
	char inputUrl[100];
    strcpy(inputUrl, imageUrl); // "TST001.png";

	size_t fileLen;

	FILE *file;
	unsigned char *buffer;
	char *base64Content;

	//Open file
	char locUrl[100];
	strcpy(locUrl, "./");
	strcat(locUrl, inputUrl);
	file = fopen(locUrl, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", inputUrl);
		return;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer = (char*) malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
		fclose(file);
		return;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	size_t output_length = 0; // note *NOT* a pointer
	base64Content = base64_encode(buffer, fileLen, &output_length); 

	fclose(file);
	free(buffer);

	callAPI(base64Content);

	return;
}
