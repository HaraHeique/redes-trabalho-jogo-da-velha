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

/* Estrutura dos jogadores */
typedef struct jogador
{
	char nome[50];
} Jogador;

/* Protótipo das funções utilizadas */
void criarJogadores(Jogador* jogadores, int qntJogadores);
int isAllDigit(char *strDigit);
void printJogoVelha(char campo[3][3]);

/* Função de chamada principal */
int main(int argc, char *argv[])
{
    Jogador jogadores[QNTJOGADORES];
    char campoJogoVelha[3][3];

    criarJogadores(jogadores, QNTJOGADORES);
    printJogoVelha(campoJogoVelha);

    return 0;
}

/* Funções de modularização do código */

// Função responsável por criar os jogadores perguntando suas informações para eles
void criarJogadores(Jogador* jogadores, int qntJogadores) 
{
    for (int i = 0; i < qntJogadores; i++) 
    {
        char nomeJogador[100];

        printf("Digite o nome do jogador %d: ", i+1);
	    scanf("%s", nomeJogador);

        /* Valida o nome do usuário */
        while (strlen(nomeJogador) == 0 || isAllDigit(nomeJogador)) 
        {
            printf("Favor digite nome de usuário válido.\n");
            printf("Digite o nome do usuário: ");
            scanf("%s", nomeJogador);
        }

        strcpy(jogadores[i].nome, nomeJogador);
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
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++) 
        {
            printf("%c", campo[i][j]);     
        }        

        printf("\n");
    }
}
