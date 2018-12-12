/* Cabeçalho */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Constantes */
#define FALSE 0
#define TRUE 1
#define QNTJOGADORES 2
#define MARCADOR_X 'X'
#define MARCADOR_O 'O'

/* Estrutura dos jogadores */
typedef struct jogador
{
	char nome[50];
    char markJogoVelha;
} Jogador;

/* Protótipo das funções utilizadas */
void criarJogadores(Jogador* jogadores, int qntJogadores);
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

/* Função de chamada principal */
int main(int argc, char *argv[])
{
    Jogador jogadores[QNTJOGADORES];
    char campoJogoVelha[3][3];
    int mapaCampoJogoVelha[9][2];
    int ganhou = 0;
    int deuVelha = 0;
    int qntRodadas = 0;
    int cellSelecionada;
    int nextJogador;
    int jogarNovamente = 0;

    flushCampoJogoVelha(campoJogoVelha);
    mapearCampoJogoVelha(mapaCampoJogoVelha);
    criarJogadores(jogadores, QNTJOGADORES);
    pularLinhas(1);
    
    nextJogador = 0;
    
    while (!ganhou && !deuVelha) 
    {
        printJogoVelha(campoJogoVelha);
        pularLinhas(1);
        cellSelecionada = selecinarCell(jogadores[nextJogador].nome, campoJogoVelha, mapaCampoJogoVelha);
        pularLinhas(2);

        // Atualiza o campo do jogo da velha com a partir da célula selecionada pelo jogador corrente
        atualizarCampoJogoVelha(jogadores[nextJogador], cellSelecionada, campoJogoVelha, mapaCampoJogoVelha);

        // Caso alguém tenha ganhado a partida mostra quem foi o vencedor
        if (checkGanharPartida(campoJogoVelha)) 
        {
            ganhou = 1;

            printJogoVelha(campoJogoVelha);
            pularLinhas(1);
            printf("%s GANHOU A PARTIDA com o marcador %c. Parabains!", jogadores[nextJogador].nome, jogadores[nextJogador].markJogoVelha);
            pularLinhas(2);
            continue;
        }

        // Caso número de rodadas cheguem a 9 é porque ninguém ganha. Logo deu velha
        if (++qntRodadas == 9) 
        {
            deuVelha = 1;

            printJogoVelha(campoJogoVelha);
            pularLinhas(1);
            printf("DEU VELHA. Ninguém ganhou o jogo.");
            pularLinhas(2);
            continue;
        }

        // Define a posição do próximo jogador
        nextJogador = setNextJogador(nextJogador);
    }

    return 0;
}

/* Funções de modularização do código */

// Função responsável por criar os jogadores perguntando suas informações para eles
void criarJogadores(Jogador* jogadores, int qntJogadores) 
{
    char nomeJogador[100];

    for (int i = 0; i < qntJogadores; i++) 
    {
        printf("Digite o nome do jogador %d: ", i+1);
	    scanf("%[^\n]%*c", nomeJogador);

        /* Valida o nome do usuário */
        while (strlen(nomeJogador) == 0 || isAllDigit(nomeJogador)) 
        {
            printf("Favor digite nome de usuário válido.\n");
            printf("Digite o nome do usuário: ");
            scanf("%[^\n]%*c", nomeJogador); 
        }

        strcpy(jogadores[i].nome, nomeJogador);

        // Define qual é o marcador no jogo do jogador
        if (i % 2 == 0) 
        {
            jogadores[i].markJogoVelha = MARCADOR_X;
        }
        else 
        {
            jogadores[i].markJogoVelha = MARCADOR_O;
        }
    }
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

    return jogadorCorrente + 1;
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