#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include "bitmap.h"
#include "vision.h"

#define HOST "vision.googleapis.com"
#define HOST_IP "172.217.24.238"
#define PATH "/v1/images:annotate?key="
#define API_KEY "AIzaSyCct00PWxWPoXzilFo8BrgeAKawR9OiRZQ"
#define PORT "443"

char return_text[2000];

void callAPI(char* imgContent) {
	// Initialize the variables
	BIO* bio;
	SSL* ssl;
	SSL_CTX* ctx;

	// Registers the SSL/TLS ciphers and digests
	// Basically start the security layer
	SSL_library_init();

	// Create a new SSL_CTS object as a framework to establish TTL/SSL
	// or DTLS enabled connections
	ctx = SSL_CTX_new(SSLv23_client_method());

	// Error check
	if(ctx == NULL) 
	{
		printf("Ctx is null\n");
	}

	// Creates a new BIO chain consisting of an SSL BIO
	bio = BIO_new_ssl_connect(ctx);

	// Use the variable from the beginning of the file to create a
	// string that contains the URL to the site that you want to connect
	// th while also specifying the port

	BIO_set_conn_hostname(bio, HOST_IP ":" PORT);
	
	// Attempts to connect the supplied BIO
	if(BIO_do_connect(bio) <= 0)
	{
		printf("Failed connection\n");
		return;
	}
	else
	{
		printf("Connected\n");
	}

	// The bare minimum to make a HTTP request
	char prefix[300]   = "POST " PATH API_KEY " HTTP/1.1\r\n";
	char host[300]     = "HOST: " HOST "\r\n";
	char header_1[300] = "Content-Type: application/json\r\n";
	
	
	int base_data_length = strlen(imgContent);
	char* data = (char*) malloc(base_data_length + 500);
	
	strcpy(data,       "{\n"
			   "  \"requests\": [\n"
			   "    {\n"
			   "      \"image\": {\n"
			   "        \"content\": \"");

	strcat(data, imgContent);
	strcat(data, "\"\n");
	strcat(data,       "      },\n"
			   "      \"features\": [\n"
			   "        {\n"
			   "          \"type\": \"TEXT_DETECTION\"\n"
			   "        }\n"
			   "      ]\n"
			   "    }\n"
			   "  ]\n"
			   "}");

	int content_length = strlen(data);

	char header_2[200];
	sprintf(header_2, "Content-Length: %d\r\n", content_length);

	char* write_buf = (char*) malloc(base_data_length + 500 \
			+ strlen(prefix) + strlen(host) + \
			strlen(header_1) + strlen(header_2));
	sprintf(write_buf, "%s%s%s%s\r\n%s", \
			prefix, \
			host, \
			header_1, \
			header_2, \
			data);

	printf("request: \n%s%s%s%s\r\n", prefix, host, 
			header_1, header_2);

	// free data
	free(data);

	// Attemps to write len bytes from buf to BIO
	if(BIO_write(bio, write_buf, strlen(write_buf)) <= 0)
	{
		// Handle failed writes here
		if(!BIO_should_retry(bio))
		{
			// Not worth implementing, but worth knowing
		}
		// Let us know about the failed writes
		printf("Failed write\n");
	}

	// free write_buf
	free(write_buf);

	// Variables used to read the response from the server
	int size = 2000;
	char buf[2000];
	char temp_text[2000];
	strcpy(temp_text, "");

	int end_flag = 0;
	int first_flag = 1;
	int chunk_flag = 0;
	int temp_flag = 0;
	int error_flag = 0;

	int read_count = 0; 
	int read_max = 10;

	// Read the response message
	for(read_count=0;read_count<read_max;read_count++)
	{
		char* temp = NULL;
		char* temp2 = NULL;
		char* temp3 = NULL;
		char* temp4 = NULL;
		char* temp5 = NULL;
		char* temp6 = NULL;

		printf("size :%d\n", size);

		if(size < 1000)
		{
			error_flag =1;
			break;
		}

		// Get chunks of the response 1023 at the time
		size = BIO_read(bio, buf, 2000);

		printf("size :%d\n", size);

		if(first_flag==1)
		{
			temp = strstr(buf, "description");
			if(temp != NULL)
			{
				temp2 = strstr(temp, "boundingPoly");
				if(temp2 != NULL)
				{
					temp3 = strstr(temp, ":");
					temp3 = strstr(temp3, "\"");
					strncpy(temp_text, temp3+1, strlen(buf)-strlen(temp2)-1);
					break;
				}
				else 
				{
					temp3 = strstr(temp, ":");
					temp3 = strstr(temp3, "\"");
					strcpy(temp_text, temp3+1);
					if(first_flag==1) first_flag=0;
				}
			}
		}
		else 
		{
			temp4 = strstr(buf, "boundingPoly");
			if(temp4 != NULL)
			{
				strncat(temp_text, buf, strlen(buf)-strlen(temp4)-1);
				break;
			}
			else if(temp4 == NULL)
			{
				strcat(temp_text, buf);
			}

			temp5 = strstr(buf, "chunked");
			if(temp5 != NULL)
			{
				temp5 = strstr(temp5, "description");
				temp5 = strstr(temp5, ":");
				temp5 = strstr(temp5, "\"");
				temp5 = temp5 + 1;

				strcpy(temp_text, "");
				strcpy(temp_text, temp5);
			}
		}
	}

	if(error_flag == 1)
	{
		strcpy(return_text, "no response");
		return;
	}

	// filter if text is chunked
	char* chunked_temp = strstr(temp_text, "chunked");

	if(chunked_temp != NULL)
	{
		chunked_temp = strstr(chunked_temp, "description");
		chunked_temp = strstr(chunked_temp, ":");
		chunked_temp = strstr(chunked_temp, "\"");
		chunked_temp += 1;
		
		strcpy(return_text, chunked_temp);
	}
	else
	{
		strcpy(return_text, temp_text);
	}

	strcpy(temp_text, "");
	char temp2_text[2000];
	char* bound_temp = strstr(return_text, "boundingPoly");
	if(bound_temp != NULL)
	{
		bound_temp = bound_temp -2;
		strncpy(temp2_text, return_text, \
			strlen(return_text)-strlen(bound_temp));

		strcpy(return_text, "");
		strcpy(return_text, temp2_text);
	}

	// Clean after ourselves
	BIO_free_all(bio);
	SSL_CTX_free(ctx);

	// parsing result_text
	
	// remove page number
	strcpy(temp_text, "");
	strcpy(temp2_text, "");
	char *temp1 = NULL;
	char *temp2 = NULL;
	char *temp3 = NULL;
	char *temp4 = NULL;

	temp1 = return_text;
	temp2 = strstr(temp1, "\\n");
	while(temp2 != NULL)
	{
		temp3 = temp1;
		temp1 = temp2;
		temp2 = strstr(temp1+2, "\\n");
	}

	int temp_length = strlen(temp3+2) - strlen(temp1);
	int no_page_flag = 0;
	int j=0;
	for(j=0;j<temp_length;j++)
	{
		if((temp3+2)[j]<48 || \
		   (temp3+2)[j]>57)
		{
			no_page_flag=1;
		}
	}

	if(no_page_flag==0)
	{
		strncpy(temp2_text, return_text, \
			strlen(return_text)-strlen(temp3));
		strcat(temp2_text, temp1);
		strcpy(return_text, "");
		strcpy(return_text, temp2_text);
	}

	// remove '\n'
	strcpy(temp_text, "");
	strcpy(temp2_text, "");
	temp1 = NULL;
	temp2 = NULL;

	temp1 = return_text;
	temp2 = strstr(temp1, "\\n");
	while(temp2 != NULL)
	{
		strncat(temp_text, temp1, \
			strlen(temp1)-strlen(temp2));
		temp1 = temp2 + 2;
		temp2 = strstr(temp1, "\\n");
	}

	// remove last '",'
	temp2 = strstr(temp1, "\",");
	if(temp2 != NULL)
	{
		strncat(temp_text, temp1, \
			strlen(temp1)-strlen(temp2));
	}
	else
	{
		strcat(temp_text, temp1);
	}

	// remove POST~HTTP/1.1 in the text
	strcpy(return_text, "");
	temp1 = NULL;
	temp2 = NULL;

	temp1 = temp_text;
	temp2 = strstr(temp1, API_KEY);
	while(temp2 != NULL)
	{
		strncat(return_text, temp1, \
			strlen(temp1)-strlen(temp2)-30);
		temp2 = strstr(temp2, "HTTP/1.1");
		temp1 = temp2 + 10;
		temp2 = strstr(temp1, API_KEY);
	}
	strcat(return_text, temp1);

	// remove '('
	strcpy(temp_text, "");
	temp1 = NULL;
	temp2 = NULL;

	temp1 = return_text;
	temp2 = strstr(temp1, "(");
	while(temp2 != NULL)
	{
		strncat(temp_text, temp1, \
			strlen(temp1)-strlen(temp2));
		temp1 = temp2 + 1;
		temp2 = strstr(temp1, "(");
	}
	strcat(temp_text, temp1);

	strcpy(return_text, "");
	strcpy(return_text, temp_text);

	// remove ')'
	strcpy(temp_text, "");
	temp1 = NULL;
	temp2 = NULL;

	temp1 = return_text;
	temp2 = strstr(temp1, ")");
	while(temp2 != NULL)
	{
		strncat(temp_text, temp1, \
			strlen(temp1)-strlen(temp2));
		temp1 = temp2 + 1;
		temp2 = strstr(temp1, ")");
	}
	strcat(temp_text, temp1);

	strcpy(return_text, "");
	strcpy(return_text, temp_text);

	// remove '''
	strcpy(temp_text, "");
	temp1 = NULL;
	temp2 = NULL;

	temp1 = return_text;
	temp2 = strstr(temp1, "'");
	while(temp2 != NULL)
	{
		strncat(temp_text, temp1, \
			strlen(temp1)-strlen(temp2));
		temp1 = temp2 + 1;
		temp2 = strstr(temp1, "'");
	}
	strcat(temp_text, temp1);

	strcpy(return_text, "");
	strcpy(return_text, temp_text);

	// remove '\'
	strcpy(temp_text, "");
	temp1 = NULL;
	temp2 = NULL;

	temp1 = return_text;
	temp2 = strstr(temp1, "\\");
	while(temp2 != NULL)
	{
		strncat(temp_text, temp1, \
			strlen(temp1)-strlen(temp2));
		temp1 = temp2 + 1;
		temp2 = strstr(temp1, "\\");
	}
	strcat(temp_text, temp1);

	strcpy(return_text, "");
	strcpy(return_text, temp_text);

	return;
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


char* callGoogleVision(char *imageUrl) {
	char inputUrl[100];
    	strcpy(inputUrl, imageUrl);

	size_t fileLen;

	FILE *file;
	unsigned char *buffer;
	char *base64Content;

	//Open file
	char locUrl[100];
	strcpy(locUrl, inputUrl);
	printf("%s\n", locUrl);
	file = fopen(locUrl, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", inputUrl);
		return;
	}

	// Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate memory
	buffer = (char*) malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
		fclose(file);
		return;
	}

	// Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	size_t output_length = 0; // note *NOT* a pointer
	base64Content = base64_encode(buffer, fileLen, &output_length);

	fclose(file);
	free(buffer);

	callAPI(base64Content);

	return return_text;
}
