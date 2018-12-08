/* server.c:
*
* Exemplo de servidor de data e hora:
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*constantes*/
#define FALSE 0
#define TRUE 1
#define ENDERECOPADRAO "127.0.0.1"
#define PORTAPADRAO "4242"

typedef struct mensagem{
	char nome[128]; /* nome do servidor */
}Message;

/*
* Esta função reporta erro e finaliza do programa:
*/
static void
bail(const char *on_what) {
	if ( errno != 0 ) {
		fputs(strerror(errno),stderr);
		fputs(": ",stderr);
	}
	fputs(on_what,stderr);
	fputc('\n',stderr);
	exit(1);
}

int main(int argc,char **argv) {
	int z;
	char *srvr_addr = ENDERECOPADRAO;
	char *srvr_port = PORTAPADRAO;
	struct sockaddr_in adr_srvr;/* AF_INET */
	struct sockaddr_in adr_clnt;/* AF_INET */
	int len_inet; /* comprimento  */
	int s; /* Socket do servidor */
	int j1, j2;
	int vencedor = FALSE;
	Message msgJ1, msgJ2; /* variagem utilizada para armazernar os dados da estrutura */

/*
* Cria um socket do tipo TCP
*/
	s = socket(PF_INET,SOCK_STREAM,0);
	if ( s == -1 )
		bail("socket()");

/*
 * Preenche a estrutura do socket com a porta e endereço do servidor
 */
	memset(&adr_srvr,0,sizeof adr_srvr);
	adr_srvr.sin_family = AF_INET;
	adr_srvr.sin_port = htons(atoi(srvr_port));
	if ( strcmp(srvr_addr,"*") != 0 ) {
		/* Endereço normal */
		adr_srvr.sin_addr.s_addr = inet_addr(srvr_addr);
		if ( adr_srvr.sin_addr.s_addr == INADDR_NONE )
			bail("bad address."); 
	} else {
		/* Qualquer endereço */
		adr_srvr.sin_addr.s_addr = INADDR_ANY;
	}

/*
* Liga (bind) o socket com o endereço/porta
*/
	len_inet = sizeof adr_srvr;
	z = bind(s,(struct sockaddr *)&adr_srvr, len_inet);
	if ( z == -1 )
		bail("bind(2)");

/*
* Coloca o socket do servidor em estado de "escuta"
*/
	z = listen(s,2);
	if ( z == -1 )
		bail("listen(2)");

/*
* Inicia o loop do servido:
*/
	for (;;) {
		/*
		* Aguardando os dois jogadores conectarem:
		*/
        len_inet = sizeof adr_clnt;
        j1 = accept(s,(struct sockaddr *)&adr_clnt,&len_inet);
        if ( j1 == -1 ){
            bail("accept(2)");
        }
        printf("Jogador numero 1 conectado.\n");

        j2 = accept(s,(struct sockaddr *)&adr_clnt,&len_inet);
        if ( j2 == -1 ){
            bail("accept(2)");
        }
        printf("Jogador numero 2 conectado.\n");

		while (vencedor == FALSE){
			/* Esperando a jogada do jogador 1*/
			z = read(j1, &msgJ1, sizeof(Message));
			if (z == -1)
			{
				bail("read(2): It's not possible to read the socket");
			}


			z = write(j1,(const void *) &msgJ1,sizeof(Message));
			if ( z == -1 )
			{
				bail("write(2)");
			}

			/* Esperando a jogada do jogador 2*/
			z = read(j2, &msgJ2, sizeof(Message));
			if (z == -1)
			{
				bail("read(2): It's not possible to read the socket");
			}


			z = write(j2,(const void *) &msgJ2,sizeof(Message));
			if ( z == -1 ){
				bail("write(2)");		
			}
		}
        
	}

    /*
    * Fecha a conexão com o cliente
    */
    close(j1);
    close(j2);

	return 0;
}