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
#define QNTJOGADORES 2

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
	char informacoes[256];
    int jogada;
    int fimDeJogo;
} Mensagem;

/* Protótipo das funções utilizadas */
void preencheMensagemBoasVindas(Mensagem* msg, Jogador* adversario);
void defineMarcador(Jogador* jogadores);
int isAllDigit(char *strDigit);
void printJogoVelha(char campo[3][3]);
void mapearCampoJogoVelha(int mapaCampo[9][2]);
int selecinarCell(char* nomeJogador, char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]);
int isCellSelecionada(int cellSelecionada, char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]);
int setNextJogador(int nextJogador);
void pularLinhas(int qntLinhas);
void atualizarCampoJogoVelha(Jogador jogador, int cellSelecionada, char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]);
int checkGanharPartida(char campoJogoVelha[3][3]);
void flushCampoJogoVelha(char campoJogoVelha[3][3]);

/* Esta função reporta erro e finaliza do programa: */
static void bail(const char *on_what) 
{
	if ( errno != 0 ) {
		fputs(strerror(errno),stderr);
		fputs(": ",stderr);
	}
	fputs(on_what,stderr);
	fputc('\n',stderr);
	exit(1);
}

/* Função principal de execução do programa */
int main(int argc,char **argv) 
{
	int z;
	char *srvr_addr = ENDERECOPADRAO;
	char *srvr_port = PORTAPADRAO;
	struct sockaddr_in adr_srvr;/* AF_INET */
	struct sockaddr_in adr_clnt;/* AF_INET */
	int len_inet; /* comprimento  */
	int s; /* Socket do servidor */
	int jog[QNTJOGADORES];
	int vencedor = FALSE;
	Mensagem msg; /* variáveis utilizada para armazernar os dados da estrutura */
    Jogador jogadores[QNTJOGADORES];
    int mapaCampoJogoVelha[9][2];
    int qntRodadas = 0;
    int cellSelecionada;
    int nextJogador = 0;
    int jogarNovamente = 0;

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

	/* Coloca o socket do servidor em estado de "escuta" com até 2 clientes 
	que no caso serão os dois que jogarão o jogo da velha */
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
        jog[0] = accept(s, (struct sockaddr *)&adr_clnt, &len_inet);
        if ( jog[0] == -1 )
		{
            bail("accept(1): Error ao conectar com o jogador 1");
        }
        printf("Jogador número 1 conectado!\n");

		// Conectando o jogador 2
        jog[1] = accept(s, (struct sockaddr *)&adr_clnt, &len_inet);
        if ( jog[1] == -1 )
		{
            bail("accept(2): Error ao conectar com o jogador 2");
        }
        printf("Jogador número 2 conectado!\n");

		// Lê as estruturas dos jogadores 1 e 2 conectados e pegam seus nome para mandar a mensagem de boas vindas
		z = read(jog[0], &msg, sizeof(Mensagem));
		if (z == -1)
		{
			bail("read(1): It's not possible to read the socket (jogador1)");
		}
        jogadores[0] = msg.jogador;

		z = read(jog[1], &msg, sizeof(Mensagem));
		if (z == -1)
		{
			bail("read(2): It's not possible to read the socket (jogador2)");
		}
        jogadores[1] = msg.jogador;

        defineMarcador(jogadores);

        msg.jogador = jogadores[1];
        msg.fimDeJogo = FALSE;
        preencheMensagemBoasVindas(&msg, &jogadores[0]);
		z = write(jog[1],(const void *) &msg,sizeof(Mensagem));
		if ( z == -1 )
		{
			bail("write(1)");		
		}

        msg.jogador = jogadores[0];
		preencheMensagemBoasVindas(&msg, &jogadores[1]); 
		z = write(jog[0], (const void *) &msg, sizeof(Mensagem));
		if ( z == -1 )
		{
			bail("write(2)");
		}

        flushCampoJogoVelha(msg.tabuleiro);
        int aux = 0;

		/* Fica em um loop até que haja um vencedor no jogo */
		while (!msg.fimDeJogo)
		{
            printf("A\n");
            strcpy(msg.informacoes, "Sua vez.");
            z = write(jog[nextJogador], (const void *) &msg, sizeof(Mensagem));
			if ( z == -1 )
			{
				bail("write(3)");
			}

			/* Esperando a jogada */
			z = read(jog[nextJogador], &msg, sizeof(Mensagem));
			if (z == -1)
			{
				bail("read(3): It's not possible to read the socket");
			}
            
            aux++;
            if (aux == 4){
                msg.fimDeJogo = TRUE;
            }
            nextJogador = setNextJogador(nextJogador);
            printf("\n\nJOGADA: %d\n", msg.jogada);
            printf("NEXT: %d\n", nextJogador);
            printf("AUX: %d\n", aux);
            printf("FIM DE JOGO: %d\n", msg.fimDeJogo);
		}

         /* Fecha a conexão com o cliente depois que o jogo acaba */
        close(jog[0]);
        close(jog[1]);
	}

   

	return 0;
}

/* Funções de modularização do código */

/* Preenche o campo de informações da mensagem com o mensangem de boas vindas de acordo com o jogador */
void preencheMensagemBoasVindas(Mensagem* msg, Jogador* adversario) 
{
	char* msgPt1 = "Bem vindo ";
    char* msgPt2 = " ao game jogo da velha!\n";
    char* msgPt3 = "Seu marcador é o ";
    char* msgPt4 = "\nSeu oponente é o ";

    strcpy(msg->informacoes, msgPt1);
    strcat(msg->informacoes, msg->jogador.nome);
    strcat(msg->informacoes, msgPt2);
    strcat(msg->informacoes, msgPt3);

    size_t cur_len = strlen(msg->informacoes);
    msg->informacoes[cur_len] = msg->jogador.markJogoVelha;
    msg->informacoes[cur_len+1] = '\0';

    strcat(msg->informacoes, msgPt4);
    strcat(msg->informacoes, adversario->nome);
    strcat(msg->informacoes, "\n");

}

// Define qual é o marcador no jogo do jogador
void defineMarcador(Jogador* jogadores) 
{
    jogadores[0].markJogoVelha = MARCADOR_X;
    jogadores[1].markJogoVelha = MARCADOR_O;   
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

// Mapear o campo do jogo da velha para sempre que a pessoa inserir o número desejado de onde quer jogar acessá-lo rapidamente
void mapearCampoJogoVelha(int mapaCampo[9][2])
{
    int lin = 0;
    int col = 0;

    for (int i = 0; i < 9; i++) 
    {
        // Insere os valores de linha e coluna do campo do jogo da velha
        mapaCampo[i][0] = lin;
        mapaCampo[i][1] = col;

        // Quando chega na última coluna do campo do jogo da velha reseta o valor da coluna e vai para próxima linha 
        if (col == 2) 
        {
            col = 0;
            lin++;
            continue;
        }

        col++;
    }
}

// O jogador escolhe a célula campo que deseja jogar
int selecinarCell(char* nomeJogador, char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]) 
{
    char strCell[10];
    int cellSelecionada;
    int digitarNovamente;

    printf("%s escolha o número da célula correspondente ao local que deseja jogar: ", nomeJogador);
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

// Pegando a posição do próximo jogador
int setNextJogador(int jogadorCorrente) 
{
    if (jogadorCorrente == 1) 
    {
        return 0;
    }

    return 1;
}

// Para pular linhas e deixar o layout mais "legal"
void pularLinhas(int qntLinhas) 
{
    for (int i = 0; i < qntLinhas; i++) 
    {
        printf("\n");
    }
}

// Faz a atualização da matriz que representa o campo do jogo da velha após a jogada do jogador
void atualizarCampoJogoVelha(Jogador jogador, int cellSelecionada, char campoJogoVelha[3][3], int mapaCampoJogoVelha[9][2]) 
{
    int lin, col;

    // Pega a linha e coluna da célula selecionada
    lin = mapaCampoJogoVelha[cellSelecionada-1][0];
    col = mapaCampoJogoVelha[cellSelecionada-1][1];

    // Insere o marcador do jogador passado como argumento na respectiva célula do campo do jogo da velha
    campoJogoVelha[lin][col] = jogador.markJogoVelha;
}

// Checar se algum jogador ganhou a partida
int checkGanharPartida(char campoJogoVelha[3][3]) 
{
    // Checando diagonal principal
    if (campoJogoVelha[0][0] == campoJogoVelha[1][1] && campoJogoVelha[1][1] == campoJogoVelha[2][2]) 
    {
        return TRUE;
    }

    // Checando a diagonal secundária
    if (campoJogoVelha[0][2] == campoJogoVelha[1][1] && campoJogoVelha[1][1] == campoJogoVelha[2][0]) 
    {
        return TRUE;
    }

    // Checar as linhas horizontais e verticais ao mesmo tempo visando desempenho
    for (int i = 0; i < 3; i++) 
    {
        // Checar as linhas horizontais
        if (campoJogoVelha[i][0] == campoJogoVelha[i][1] && campoJogoVelha[i][1] == campoJogoVelha[i][2]) 
        {
            return TRUE;
        }

        // Checar as linhas verticais
        if (campoJogoVelha[0][i] == campoJogoVelha[1][i] && campoJogoVelha[1][i] == campoJogoVelha[2][i]) 
        {
            return TRUE;
        }
    }

    return FALSE;
}

// Limpa o campo jogo da velha que tem o risco de começar com caracteres correspondente ao jogo
void flushCampoJogoVelha(char campoJogoVelha[3][3]) 
{   
    char caracter = 0;

    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++) 
        {
            campoJogoVelha[i][j] = caracter++;
        }
    }
}
