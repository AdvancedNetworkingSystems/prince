#include "socket.h"

#include "error.h"

/**
* Initialize a socket
* @param host hostname of the server
* @param port remote port of the server
* @return socket descriptor
*/
int _create_socket(char* hostname, int port, int ignore) {
	struct sockaddr_in temp;
	struct hostent *host;
	int sock;
	int rc;
        unsigned int timeout = DEFAULT_TIMEOUT;

	temp.sin_family = AF_INET;
	temp.sin_port = htons(port);
	host = gethostbyname(hostname);
	if (host == NULL) {
               perror("prince-socket");
		exit(EXIT_FAILURE);
	}
	bcopy(host->h_addr, &temp.sin_addr, host->h_length);
	sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
                perror("socket");
                exit(EXIT_FAILURE);
        }
        while (connect(sock, (struct sockaddr*) &temp, sizeof(temp))) {
                if (errno == errno & ignore) {
                        break;
                }
                perror("connect");
                fprintf(stderr, "Could not connect to socket, retry\n");
                fprintf(stderr, "Wait %d seconds before reconnecting\n", timeout);
                sleep(timeout);
                timeout = timeout >= MAX_TIMEOUT ? MAX_TIMEOUT : timeout * 2;
        }
        timeout = DEFAULT_TIMEOUT;
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
	int i = 0;
	int amntRecvd = 0;
	char * page = (char*) malloc(SIZE_TO_READ);
	while ((amntRecvd = recv(sd, page+i, SIZE_TO_READ, 0)) > 0) {
		i += amntRecvd;
		if (!(page=realloc(page, SIZE_TO_READ + i))) {
			free(page);
			return false;
		}
	}
	if (!i) return false;
	page[i] = '\0';
	*buffer = page;
	return true;

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
	while ((amntRecvd = recv(sd, page+i, SIZE_TO_READ, 0)) > 0) {
		i += amntRecvd;
		if (!(page=realloc(page, i+SIZE_TO_READ))) {
			free(page);
			return 0;
		}
	}
	if (!i) return 0;
	*buffer = page;
	char *body = strstr(page, "\r\n\r\n");
	if (body) body += 4;

	/*check if we have received the full topology */
	int r = check_header_clen(page, body);
	if(!r) fprintf(stderr, "Lenght of buffer don't match\n");
	*buffer = strdup(body);
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
	char *len = (char*) malloc(endbuf-buffer);
       if (len == NULL) {
               perror("prince-socket");
               exit(EXIT_FAILURE);
       }
	memcpy(len, buffer + 15, endbuf-buffer);
	unsigned long size = atol(len);
	if (strlen(body) == size)
		return 1;
	else
		return 0;
}
