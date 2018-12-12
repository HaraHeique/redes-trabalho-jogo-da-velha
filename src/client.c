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
#define MARCADOR_X 'X'
#define MARCADOR_O 'O'

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
    char tabuleiro[3][3];
    int mapaCampoJogoVelha[9][2];
    char informacoes[256];
    int jogada;
    int fimDeJogo;
    char adversario[50];
} Mensagem;

/* Protótipo das funções utilizadas */
void criaJogador(Jogador* jogador);
int isAllDigit(char *strDigit);
void pularLinhas(int qntLinhas);
void printJogoVelha(char campo[3][3]);
int selecinarCell(char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]);
int isCellSelecionada(int cellSelecionada, char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]);

/* Esta função reporta erro e finaliza do programa */	
static void	bail(const char *on_what) 
{	
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
    Mensagem msg; /* variavel utilizada para armazernar os dados da estrutura */        

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

    /* Conecta com o servidor */
    len_inet = sizeof adr_srvr;

    z = connect(s, (struct sockaddr *) &adr_srvr, len_inet);
    if ( z == -1 ) 
    {
        bail("\nNão é possível conectar com o servidor. Provavelmente ele está offline.");
    }

    // Passa o jogador como argumento para pegar seu nome
    criaJogador(&msg.jogador);

    /* Escreve na mensagem inicial que é mandando seu nome para o socket do servidor */
    z = write(s, (const void *)&msg, sizeof(Mensagem));
    if (z == -1)
    {
        bail("write(2): It's not possible to write on socket.");
    }

    /* Após cliente se conectar com o server lê as informações com a mensagem de 
    boas vindas proveniente do servidor */
    z = read(s, &msg, sizeof(Mensagem));
    if ( z == -1 )
    {
        bail("read(2)");
    }

    printf("-----------INICIO DA PARTIDA-----------\n");

    // Mensagem de boas-vindas proveniente do servidor
    pularLinhas(1);
    puts(msg.informacoes);
    pularLinhas(2);
    
    do {
        z = read(s, &msg, sizeof(Mensagem));
        if ( z == -1 )
        {
            bail("read(2)");
        }

        puts(msg.informacoes);
        printJogoVelha(msg.tabuleiro);
        pularLinhas(1);

        if(!msg.fimDeJogo){
            msg.jogada = selecinarCell(msg.tabuleiro, msg.mapaCampoJogoVelha);
            pularLinhas(2);
            z = write(s, (const void *)&msg, sizeof(Mensagem));
            if (z == -1)
            {
                bail("write(2): It's not possible to write on socket.");
            }
        }else{
            printf("FIM DE JOGO\n");
        }
        
    }while (!msg.fimDeJogo);

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

// Printar a matriz com um layout legal
void printJogoVelha(char campo[3][3]) 
{
    int numCell = 1;

    for (int i = 0; i < 3; i++) 
    {
        printf("|");

        for (int j = 0; j < 3; j++) 
        {
            // Caso a célula do campo esteja nulo é porque não foi jogado pelo jogador ainda, logo coloca sua enumeração
            if (campo[i][j] != MARCADOR_X && campo[i][j] != MARCADOR_O)
            {
                printf("%d", numCell);
            }
            else 
            {
                printf("%c", campo[i][j]);
            }
            printf("|");
            numCell++;
        }
        printf("\n");
    }
}

// O jogador escolhe a célula campo que deseja jogar
int selecinarCell(char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]) 
{
    char strCell[10];
    int cellSelecionada;
    int digitarNovamente;

    printf("Escolha o número da célula correspondente ao local que deseja jogar: ");
    scanf("%s", strCell);

    do {
        digitarNovamente = 0;

        // Caso o jogador não digite número ou que não esteja no range de 1 a 9
        while (strlen(strCell) > 1 || strCell[0] == '0' || !isdigit(strCell[0]))
        {
            printf("Favor digite somente dígitos numéricos de 1 a 9 que representam as células.\n");
            printf("Escolha o número da célula novamente: ");
            scanf("%s", strCell);
        }

        // Caso o jogador digite uma célula já selecionada
        cellSelecionada = atoi(strCell);

        if (isCellSelecionada(cellSelecionada, campoJogoVelha, mapaCampoJogoVelha)) 
        {
            printf("Célula já selecionada. Favor escolha as dispiníveis que são os dígitos numéricos.\n");
            printf("Escolha o número da célula novamente: ");
            scanf("%s", strCell);
            digitarNovamente = 1;
        }

    } while(digitarNovamente);
	
    return cellSelecionada;
}

// Checa se a célula selecionada já foi previamente selecionada em jogadas anteriores
int isCellSelecionada(int cellSelecionada, char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]) 
{
    int lin = mapaCampoJogoVelha[cellSelecionada-1][0];
    int col = mapaCampoJogoVelha[cellSelecionada-1][1];

    if (campoJogoVelha[lin][col] == MARCADOR_X || campoJogoVelha[lin][col] == MARCADOR_O) 
    {
        return TRUE;
    }

    return FALSE;
}