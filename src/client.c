/* Client-side code */

#include <stdio.h>	
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>	
#include <string.h>	
#include <sys/types.h>	
#include <sys/socket.h>	
#include <netinet/in.h>	
#include <arpa/inet.h>	
#include <netdb.h>	
#include <time.h>
#include <ctype.h>

/* Constantes */
#define ENDERECOPADRAO "127.0.0.1"
#define PORTAPADRAO "4242"
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
typedef struct mensagem {
	Jogador jogador;
    char informacoes[100];
} Message;

/* Protótipo das funções utilizadas */
void criaJogador(Jogador* jogador);
int isAllDigit(char *strDigit);
void pularLinhas(int qntLinhas);

/* Esta função reporta erro e finaliza do programa */	
static void	bail(const char *on_what) {	
    if ( errno != 0 ) {	
        fputs(strerror(errno),stderr);	
        fputs("	 ",stderr);
    }	
    fputs(on_what,stderr);	
    fputc('\n',stderr);	
    exit(1);	
}	

/* Função principal de execução do programa */
int main(int argc,char *argv[]) {	
    int z;
    char *srvr_addr = ENDERECOPADRAO;
    char *srvr_port = PORTAPADRAO;
    struct sockaddr_in adr_srvr;/* AF_INET */
    int len_inet; /* comprimento */
    int s; /* Socket */
    int vencedor = FALSE;
    Message msg; /* variavel utilizada para armazernar os dados da estrutura */        
        
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
    adr_srvr.sin_addr.s_addr =
    inet_addr(srvr_addr);
    if ( adr_srvr.sin_addr.s_addr == INADDR_NONE )
    {
        bail("bad address.");
    }

    // Passa o jogador como argumento para pegar seu nome
    criaJogador(&msg.jogador);

    /* Conecta com o servidor */
    len_inet = sizeof adr_srvr;

    z = connect(s, (struct sockaddr *) &adr_srvr, len_inet);
    if ( z == -1 ) 
    {
        bail("\nNão é possível conectar com o servidor. Provavelmente ele está offline.");
    }

    /* Escreve na mensagem inicial que é mandando seu nome para o socket do servidor */
    z = write(s, (const void *)&msg, sizeof(Message));
    if (z == -1)
    {
        bail("write(2): It's not possible to write on socket.");
    }

    /* Após cliente se conectar com o server lê as informações com a mensagem de 
    boas vindas proveniente do servidor */
    z = read(s, &msg, sizeof(Message));
    if ( z == -1 )
    {
        bail("read(2)");
    }

    // Mensagem de boas-vindas proveniente do cliente
    pularLinhas(1);
    puts(msg.informacoes);
    pularLinhas(2);

    exit(1);

    while (vencedor == FALSE) 
    {
        /* Escreve a mensagem para o socket do servidor */
        z = write(s, (const void *)&msg, sizeof(Message));
        if (z == -1)
        {
            bail("write(2): It's not possible to write on socket.");
        }

        /* Lê as informações */
        z = read(s, &msg, sizeof(Message));
        if ( z == -1 ){
            bail("read(2)");
        }
        // printf("%s\n", msg.jogador.nome);
        
        // printf("Digite 1 pra sair.\n");
        // scanf("%d", &vencedor);
    }

    /* Fecha o socket */	
    close(s);	

    return 0;	
}

/* Funções de modularização do código */

// Função responsável por criar o jogador perguntando suas informações para eles
void criaJogador(Jogador* jogador) 
{
    char nomeJogador[100];
    
    printf("Digite seu nome jogador: ");
    scanf("%[^\n]%*c", nomeJogador);

    /* Valida o nome do usuário */
    while (nomeJogador == NULL || strlen(nomeJogador) <= 1 || isAllDigit(nomeJogador)) 
    {
        printf("Favor digite um nome válido.\n");
        printf("Digite o nome do jogador: ");
        scanf("%[^\n]%*c", nomeJogador); 
    }

    // Coloca o nome do jogador dentro da estrutura passada como referência
    strcpy(jogador->nome, nomeJogador);
}

// Checa se a string passada é de somente dígitos numéricos e positivos
int isAllDigit(char *strDigit)
{
	for (int i = 0; i < strlen(strDigit); i++)
	{
		if (!isdigit(strDigit[i]))
		{
			return FALSE;
		}
	}
	return TRUE;
}

// Para pular linhas e deixar o layout mais "legal"
void pularLinhas(int qntLinhas) 
{
    for (int i = 0; i < qntLinhas; i++) 
    {
        printf("\n");
    }
}