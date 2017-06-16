#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#define HOST "openapi.naver.com"
#define PORT "443"

int main() {

    //
    //  Initialize the variables
    //
    BIO* bio;
    SSL* ssl;
    SSL_CTX* ctx;

    //
    //   Registers the SSL/TLS ciphers and digests.
    //
    //   Basically start the security layer.
    //
    SSL_library_init();

    //
    //  Creates a new SSL_CTX object as a framework to establish TLS/SSL
    //  or DTLS enabled connections
    //
    ctx = SSL_CTX_new(SSLv23_client_method());

    //
    //  -> Error check
    //
    if (ctx == NULL)
    {
        printf("Ctx is null\n");
    }

    //
    //   Creates a new BIO chain consisting of an SSL BIO
    //
    bio = BIO_new_ssl_connect(ctx);

    //
    //  Use the variable from the beginning of the file to create a 
    //  string that contains the URL to the site that you want to connect
    //  to while also specifying the port.
    //
    BIO_set_conn_hostname(bio, "125.209.234.165:" PORT);

    //
    //   Attempts to connect the supplied BIO
    //
    if(BIO_do_connect(bio) <= 0)
    {
        printf("Failed connection\n");
        return 1;
    }
    else
    {
        printf("Connected\n");
    }

    //
    //  The bare minimum to make a HTTP request.
    //
    
    char* write_buf = "POST /v1/voice/tts.bin HTTP/1.1\r\n"
                      "Host: " HOST "\r\n"
                      "X-Naver-Client-Id: FRGJBare95DPOfbfWggQ\r\n"
                      "X-Naver-Client-Secret: a9NYpNg5Mr\r\n"
		      "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n"
		      "Content-Length: 40\r\n"
                      "\r\n"
		      "speaker=clara&speed=0&text=Hello, world!";

    printf("response: \n%s\n", write_buf);

    //
    //   Attempts to write len bytes from buf to BIO
    //
    if(BIO_write(bio, write_buf, strlen(write_buf)) <= 0)
    {
        //
        //  Handle failed writes here
        //
        if(!BIO_should_retry(bio))
        {
            // Not worth implementing, but worth knowing.
        }

        //
        //  -> Let us know about the failed writes
        //
        printf("Failed write\n");
    }

    //
    //  Variables used to read the response from the server
    //
    int size;
    char buf[1024];

    FILE* mp3_fp = fopen("respons.mp3", "wb");

    //
    //  Read the response message
    //
    for(;;)
    {
        //
        //  Get chunks of the response 1023 at the time.
        //
        size = BIO_read(bio, buf, 1023);

	printf("size: %d\n", size);

        //
        //  If no more data, then exit the loop
        //
        if(size <= 0)
        {
            break;
        }

        //
        //  Terminate the string with a 0, to let know C when the string 
        //  ends.
        //
        buf[size] = 0;

        //
        //  ->  Print out the response
        //
        //printf("%s", buf);
	fwrite((void*)buf,sizeof(char), sizeof(buf)/sizeof(char), mp3_fp);
    }

    fclose(mp3_fp);
    //
    //  Clean after ourselves
    //
    BIO_free_all(bio);
    SSL_CTX_free(ctx);

    return 0;
}
