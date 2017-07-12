#include "socket.h"



/**
* Initialize a socket
* @param host hostname of the server
* @param port remote port of the server
* @return socket descriptor
*/
int _create_socket(char* host, int port)
{
	struct sockaddr_in temp;
	struct hostent *h;
	int sock;
	int error;

	temp.sin_family=AF_INET;
	temp.sin_port=htons(port);
	h=gethostbyname(host);
	if (h==0)
	{
		printf("Gethostbyname failed\n");
		exit(1);
	}
	bcopy(h->h_addr,&temp.sin_addr,h->h_length);
	sock=socket(AF_INET,SOCK_STREAM, 0);
	error=connect(sock, (struct sockaddr*) &temp, sizeof(temp));
	return sock;
}

/**
* Receive plain ASCII data from sd
* @param sd socket descriptor
* @param **buffer pointer to
* @return 1 if success, 0 otherwise
*/
int _telnet_receive(int sd, char **buffer)
{
	/*ALLOC finalBuffer ->> MUST FREE IT */
	int i=0;
	int amntRecvd = 0;
	char * page = (char*) malloc(SIZE_TO_READ);
	while((amntRecvd = recv(sd, page+i, SIZE_TO_READ, 0)) >0){
		i+=amntRecvd;
		if(!(page=realloc(page, i+SIZE_TO_READ))){
			free(page);
			return false;
		}
	}
	if(!i) return false;
	page[i]='\0';
	*buffer=page;
	return true;

}
/**
* Receive HTTP data from sd reading line by line
* @param sd socket descriptor
* @param **finalbuffer pointer to
* @return 1 if success, 0 otherwise
*/
int _http_receive(int sd, char **buffer)
{
	FILE* fd = fdopen(sd,"r");
	char s_buffer[BUFFER_SIZE], line[LINE_SIZE];
	long size;
	setvbuf(fd, s_buffer, _IOLBF, BUFFER_SIZE);
	fgets(line, LINE_SIZE, fd);
	while(strcmp(line, "\r\n")!=0){
		fgets(line, LINE_SIZE, fd);
		if((strstr(line, "Content-Length:"))){
			size = atol(line+16);
		}
	}
	if(!size) return 0;
	char * page = (char*) malloc(size);
	int i=0;
	while(i<size){
		fgets(line, LINE_SIZE, fd);
		int line_len=strlen(line);
		memcpy(page+i, line, line_len);
		i+=line_len;
	}
	*buffer = page;
	return 1;
}


/**
* Receive HTTP data from sd reading with pages
* @param sd socket descriptor
* @param **finalbuffer pointer to
* @return 1 if success, 0 otherwise
*/
int _receive_data(int sd, char **buffer)
{
	/*ALLOC finalBuffer ->> MUST FREE IT */

	int i=0;
	int amntRecvd = 0;
	char * page = (char*) malloc(SIZE_TO_READ);
	while((amntRecvd = recv(sd, page+i, SIZE_TO_READ, 0)) >0){
		i+=amntRecvd;
		if(!(page=realloc(page, i+SIZE_TO_READ))){
			free(page);
			return 0;
		}
	}
	if(!i) return 0;
	*buffer=page;
	char *body = strstr(page, "\r\n\r\n");
	if(body) body+=4;

	/*check if we have received the full topology */
	int r = check_header_clen(page, body);
	if(!r) printf("Lenght of buffer don't match	\n");
	*buffer=strdup(body);
	free(page);
	return r;
}
/**
* Check if the size of the HTTP content is the same as the header
* @param *header	header buffer
* @param *body		http body buffer
* @return 1 if equal, 0 otherwise
*/
int check_header_clen(char *header, char *body)
{
	char *buffer = strstr(header, "Content-Length:");
	char *endbuf = strstr(buffer, "\r\n");
	char *len = (char*)malloc(endbuf-buffer);
	memcpy(len, buffer+15,endbuf-buffer);
	unsigned long size=atol(len);
	if(strlen(body) == size)
		return 1;
	else
		return 0;
}
