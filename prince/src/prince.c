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
	id_bc_pair *map;
	sd = create_socket("150.217.18.250", 9090);

	buffer=receive_data(sd);

	c_graph_parser *gp = (void *) new_graph_parser(1, 0);
	graph_parser_parse_jsoninfo(&gp, buffer);
	graph_parser_calculate_bc(&gp);
	graph_parser_compose_bc_map(&gp, map);

	free(buffer);
	free(map);
	delete_my_class(&gp);
	close_socket(sd);
}

char * receive_data(int sd){
    char *buffer = ( char * ) malloc ( sizeof ( char ) * SIZE_TO_READ );
    int n = read (sd, buffer, SIZE_TO_READ );
    while(n > 0){
        buffer = ( char * ) realloc ( buffer, n + SIZE_TO_READ );
        n = read (sd, buffer, SIZE_TO_READ );
    }

    return buffer;
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
  sock=socket(AF_INET,SOCK_STREAM |  SOCK_NONBLOCK, 0);
  //Connessione del socket. Esaminare errore per compiere azioni
  //opportune in caso di errore.
  errore=connect(sock, (struct sockaddr*) &temp, sizeof(temp));
  return sock;
}

