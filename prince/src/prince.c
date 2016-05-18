/*
 * prince.c
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */
#include "prince.h"


int main(){
	int sd, len, bufferlen;
	char *buffer;
	map_id_bc_pair map;
//create socket and send command to jsoninfo plugin
	sd = create_socket("79.52.131.119", 9090);
	char *req = "/topology";
	int sent = send(sd,req,strlen(req),0);
	//receive data and parse header
	if(!receive_data(sd, &buffer)){
		return 0;
	}
	//parse the json with graph_parser
	c_graph_parser *gp = (void *) new_graph_parser(1, 0);
	graph_parser_parse_jsoninfo(&gp, buffer	);
	graph_parser_calculate_bc(&gp);
	graph_parser_compose_bc_map(&gp, &map); //TODO: free


	for(int i=0;i<map.size;i++){
		free(map.map[i].id);
	}
	free(map.map);
	free(buffer);
	delete_my_class(&gp);
	close_socket(sd);
	return 1;
}

int receive_data(int sd, char **finalBuffer){
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
	    memcpy(newBuffer,pageContentBuffer,oldSize);
	    free(pageContentBuffer);
	    pageContentBuffer = newBuffer;
	}
	if(i==0) return 0;

	char *body = strstr(pageContentBuffer, "\r\n\r\n");
	if(body) body+=4;
	*finalBuffer=body;

	//check if we have received the full topology
	return check_header_clen(pageContentBuffer, body);

}
int check_header_clen(char *header, char *body){

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

void close_socket(int sock)
{
  close(sock);
  return;
}

int create_socket(char* Destinazione, int Porta)
{
  struct sockaddr_in temp;
  struct hostent *h;
  int sock;
  int errore;

  //Tipo di indirizzo
  temp.sin_family=AF_INET;
  temp.sin_port=htons(Porta);
  h=gethostbyname(Destinazione);
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

