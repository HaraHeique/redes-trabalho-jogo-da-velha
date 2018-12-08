/* Server-side code */

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
#include <ctype.h>

/* Constantes */
#define ENDERECOPADRAO "127.0.0.1"
#define PORTAPADRAO "4242"
#define MARCADOR_X 'X'
#define MARCADOR_O 'O'
#define FALSE 0
#define TRUE 1

/* Estruturas do programa */

// Estrutura do jogador que joga o game
typedef struct jogador
{
	char nome[50];
    char markJogoVelha;
} Jogador;

// Estrutura que é trocada entre cliente e servidor
typedef struct mensagem 
{
	Jogador jogador;
	char informacoes[100];
} Message;

/* Protótipo das funções utilizadas */
void preencheMensagemBoasVindas(Message* msgJogador);

/* Esta função reporta erro e finaliza do programa: */
static void bail(const char *on_what) {
	if ( errno != 0 ) {
		fputs(strerror(errno),stderr);
		fputs(": ",stderr);
	}
	fputs(on_what,stderr);
	fputc('\n',stderr);
	exit(1);
}

/* Função principal de execução do programa */
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
	Message msgJ1, msgJ2; /* variáveis utilizada para armazernar os dados da estrutura */

	/* Cria um socket do tipo TCP */
	s = socket(PF_INET,SOCK_STREAM,0);
	if ( s == -1 ) 
	{
		bail("socket()");
	}

	/* Preenche a estrutura do socket com a porta e endereço do servidor */
	memset(&adr_srvr,0,sizeof adr_srvr);
	adr_srvr.sin_family = AF_INET;
	adr_srvr.sin_port = htons(atoi(srvr_port));
	if ( strcmp(srvr_addr,"*") != 0 ) 
	{
		/* Endereço normal */
		adr_srvr.sin_addr.s_addr = inet_addr(srvr_addr);
		if ( adr_srvr.sin_addr.s_addr == INADDR_NONE ) 
		{
			bail("bad address.");
		} 
	} 
	else 
	{
		/* Qualquer endereço */
		adr_srvr.sin_addr.s_addr = INADDR_ANY;
	}

	/* Liga (bind) o socket com o endereço/porta */
	len_inet = sizeof adr_srvr;
	z = bind(s,(struct sockaddr *)&adr_srvr, len_inet);
	if ( z == -1 ) 
	{
		bail("bind(2)");
	}

	/* Coloca o socket do servidor em estado de "escuta" com até 2 clientes que no
	caso serão os dois que jogarão o jogo da velha */
	z = listen(s, 2);
	if ( z == -1 ) 
	{
		bail("listen(2)");
	}

	printf("Servidor do jogo da velha online!\n");

	/* Inicia o loop do servidor: */
	for (;;) 
	{
		/* Aguardando as solicitações de conexões dos dois jogadores */
        len_inet = sizeof adr_clnt;

		// Conectando o jogador 1
        j1 = accept(s, (struct sockaddr *)&adr_clnt, &len_inet);
        if ( j1 == -1 )
		{
            bail("accept(2)");
        }
        printf("Jogador número 1 conectado!\n");

		// Conectando o jogador 2
        j2 = accept(s, (struct sockaddr *)&adr_clnt, &len_inet);
        if ( j2 == -1 )
		{
            bail("accept(2)");
        }
        printf("Jogador número 2 conectado!\n");

		// Lê as estruturas dos jogadores 1 e 2 conectados e pegam seus nome para mandar a mensagem de boas vindas
		z = read(j1, &msgJ1, sizeof(Message));
		if (z == -1)
		{
			bail("read(2): It's not possible to read the socket");
		}

		z = read(j2, &msgJ2, sizeof(Message));
		if (z == -1)
		{
			bail("read(2): It's not possible to read the socket");
		}

		// Escreve nos jogadores 1 e 2 as mensagens de boas vindas
		preencheMensagemBoasVindas(&msgJ1); 
		preencheMensagemBoasVindas(&msgJ2);

		z = write(j1, (const void *) &msgJ1, sizeof(Message));
		if ( z == -1 )
		{
			bail("write(2)");
		}

		z = write(j2,(const void *) &msgJ2,sizeof(Message));
		if ( z == -1 )
		{
			bail("write(2)");		
		}

		/* Fica em um loop até que haja um vencedor no jogo */
		while (vencedor == FALSE)
		{

			/* Esperando a jogada do jogador 1 */
			z = read(j1, &msgJ1, sizeof(Message));
			if (z == -1)
			{
				bail("read(2): It's not possible to read the socket");
			}

			z = write(j1, (const void *) &msgJ1, sizeof(Message));
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

    /* Fecha a conexão com o cliente depois que o jogo acaba */
    close(j1);
    close(j2);

	return 0;
}

/* Preenche o campo de informações da mensagem com o mensangem de boas vindas de acordo com o jogador */
void preencheMensagemBoasVindas(Message* msgJogador) 
{
	char* msgPt1 = "Bem vindo ";
	char* msgPt2 = " ao game jogo da velha!\n";

	strcat(msgJogador->informacoes, msgPt1);
	strcat(msgJogador->informacoes, msgJogador->jogador.nome);
	strcat(msgJogador->informacoes, msgPt2);
}
