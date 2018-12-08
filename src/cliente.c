/* Cliente */
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

/*Constantes*/
#define FALSE 0
#define TRUE 1
#define ENDERECOPADRAO "127.0.0.1"
#define PORTAPADRAO "4242"
 
typedef struct mensagem{
	char nome[128]; 
    char jogador;
}Message;
	
/*	
* Esta função reporta erro e finaliza do programa
*/	
static void	bail(const char *on_what) {	
    if ( errno != 0 ) {	
        fputs(strerror(errno),stderr);	
        fputs("	 ",stderr);
    }	
    fputs(on_what,stderr);	
    fputc('\n',stderr);	
    exit(1);	
}	

int main(int argc,char *argv[]) {	
    int z;
    char *srvr_addr = ENDERECOPADRAO;
    char *srvr_port = PORTAPADRAO;
    struct sockaddr_in adr_srvr;/* AF_INET */
    int len_inet; /* comprimento */
    int s; /* Socket */
    int vencedor = FALSE;
    Message msg; /* variagem utilizada para armazernar os dados da estrutura */
                
        
    /*		
    * Cria um socket do tipo TCP		
    */		
    s = socket(PF_INET,SOCK_STREAM,0);		
    if ( s == -1 ){
        bail("socket()");
    }
    /*
    * Preenche a estrutura do socket com a porta e endereço do servidor
    */
    memset(&adr_srvr,0,sizeof adr_srvr);
    adr_srvr.sin_family = AF_INET;
    adr_srvr.sin_port = htons(atoi(srvr_port));
    adr_srvr.sin_addr.s_addr =
    inet_addr(srvr_addr);
    if ( adr_srvr.sin_addr.s_addr == INADDR_NONE ){
        bail("bad address.");
    }

    /*
    * Conecta com o servidor
    */
    len_inet = sizeof adr_srvr;

    z = connect(s,(struct sockaddr *) &adr_srvr,len_inet);
    if ( z == -1 )
        bail("connect(2)");

    int x = 1;
    char resposta[128];

    printf("Seu nome.\n");
    scanf("%s", resposta);
    strcpy(msg.nome, resposta);

    /* Escreve a mensagem para o socket do servidor */
    z = write(s, (const void *)&msg, sizeof(Message));
    if (z == -1)
    {
        bail("write(2): It's not possible to write on socket.");
    }

    while(vencedor == FALSE){

        /* Escreve a mensagem para o socket do servidor */
        z = write(s, (const void *)&msg, sizeof(Message));
        if (z == -1)
        {
            bail("write(2): It's not possible to write on socket.");
        }

        /*
        * Le as informações 
        */
        z = read(s,&msg,sizeof(Message));
        if ( z == -1 ){
            bail("read(2)");
        }
        printf("%s\n",msg.nome);
        
        printf("Digite 1 pra sair.\n");
        scanf("%d", &vencedor);

    }

    /*	
    * Fecha o socket	
    */	
    close(s);	

    return 0;	
}	