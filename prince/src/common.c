#include "common.h"


/**
 * Receive HTTP data from sd
 * @param sd socket descriptor
 * @param **finalbuffer pointer to
 * @return 1 if success, 0 otherwise
 */
int
_create_socket(char* host, int port)
{
  struct sockaddr_in temp;
  struct hostent *h;
  int sock;
  int errore;

  //Tipo di indirizzo
  temp.sin_family=AF_INET;
  temp.sin_port=htons(port);
  h=gethostbyname(host);
  if (h==0)
  {
    printf("Gethostbyname failed\n");
    exit(1);
  }
  bcopy(h->h_addr,&temp.sin_addr,h->h_length);
  //Creazione socket.
  sock=socket(AF_INET,SOCK_STREAM, 0);
  //Connessione del socket. Esaminare errore per compiere azioni
  //opportune in caso di errore.
  errore=connect(sock, (struct sockaddr*) &temp, sizeof(temp));
  return sock;
}


//OLD FUNCTIONS TODO: CLEAN
int
_receive_data(int sd, char **finalBuffer){
	//ALLOC finalBuffer ->> MUST FREE IT
	int i = 0;
	int amntRecvd = 0;
	int currentSize = SIZE_TO_READ;
	int oldSize = currentSize;
	char * pageContentBuffer = (char*) malloc(currentSize);
	while ((amntRecvd = recv(sd, pageContentBuffer + i, SIZE_TO_READ, 0)) > 0) {
	    i += amntRecvd;
	    oldSize = currentSize;
	    currentSize += SIZE_TO_READ;
	    char *newBuffer = (char*) malloc(currentSize);
	    memcpy(newBuffer, pageContentBuffer, oldSize);
	    free(pageContentBuffer);
	    pageContentBuffer = newBuffer;
	}
	if(i==0) return 0;

	char *body = strstr(pageContentBuffer, "\r\n\r\n");
	if(body) body+=4;
	*finalBuffer=body;

	//check if we have received the full topology
	return _check_header_clen(pageContentBuffer, body);

}
int
_check_header_clen(char *header, char *body){

		char *buffer = strstr(header, "Content-Length:");
		char *endbuf = strstr(buffer, "\r\n");
		char *len = (char*)malloc(endbuf-buffer);
		memcpy(len, buffer+15,endbuf-buffer);
		unsigned long size=atol(len);
		if(strlen(body) == size)
			return size;
		else
			return 0;

}
int
_http_receive(int sd, char **buffer){
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
	char * pageContentBuffer = (char*) malloc(size);
	int i=0;
	while(i<size){
		fgets(line, LINE_SIZE, fd);
		int line_len=strlen(line);
		memcpy(pageContentBuffer+i, line, line_len);
		i+=line_len;
	}
	*buffer = pageContentBuffer;
	return 1;

}
int
_receive_data_olsr2(int sd, char **finalBuffer){
	//ALLOC finalBuffer ->> MUST FREE IT
	int i = 0;
	int amntRecvd = 0;
	int currentSize = SIZE_TO_READ;
	int oldSize = currentSize;
	char * pageContentBuffer = (char*) malloc(currentSize);
	while ((amntRecvd = recv(sd, pageContentBuffer + i, SIZE_TO_READ, 0)) > 0) {
	    i += amntRecvd;
	    oldSize = currentSize;
	    currentSize += SIZE_TO_READ;
	    char *newBuffer = (char*) malloc(currentSize);
	    memcpy(newBuffer, pageContentBuffer, oldSize);
	    free(pageContentBuffer);
	    pageContentBuffer = newBuffer;
	}
	if(i==0) return 0;


	*finalBuffer=pageContentBuffer;
	return 1;

}

