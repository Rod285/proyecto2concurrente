#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#define COLA 0
#define VENTANA 3
#define CLIENTE 5
#define FORMARSE 10
#define SIGUIENTE 20
#define OK 30
#define INTENTA 40
#define LLAMANDO 50
#define YENDO 60
#define ESPERA 70
#define CONFIRMA 80
#define LISTA 90
#define TERMINA 100
#define FINALIZA 110
#define N 8

void pCola(int id, int size);
void pCliente(int id, int size);
void pVentana(int id, int size);

int main(int argc,char ** argv){
    int id, size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    if(id == COLA){
        pCola(id, size);
    }else if(id > COLA && id <= VENTANA){
        pVentana(id,size);
    }else{
        pCliente(id, size);
    }
    MPI_Finalize();
return 0;
}

void pCola(int id, int size){
    int idCola = id, idP, idCli, idV,  vLlenas = 0;
    int frente = 0, posterior = 0, flag = 0, nCli = 0, turno = 0;
    MPI_Status rep;
    int buffer[N];
    
    while(nCli < size - 4){
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &rep);
        MPI_Recv(&idP, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &rep);
        if(rep.MPI_TAG == FORMARSE){
            if(vLlenas < N){
                buffer[posterior] = idP;
                posterior = (posterior + 1) % N;
                vLlenas++;
                turno++;
                MPI_Send(&turno, 1, MPI_INT, idP, OK, MPI_COMM_WORLD);
            }else{
                MPI_Send(&idP, 1, MPI_INT, idP, INTENTA, MPI_COMM_WORLD);
            }
        }else{
            if(vLlenas > 0){
                idCli = buffer[frente];
                MPI_Send(&idCli, 1, MPI_INT, idP, CONFIRMA, MPI_COMM_WORLD);
                frente = (frente + 1) % N;
                vLlenas--;
                nCli++;
            }else{
                MPI_Send(&idP, 1, MPI_INT, idP, ESPERA, MPI_COMM_WORLD);
            }
        }  
    }
    
    for(int i = 0; i < size - 1;i++){
        MPI_Send(&nCli, 1, MPI_INT, i, FINALIZA, MPI_COMM_WORLD);
    }
}

void pCliente(int id, int size){
    int idCli = id, idP, turno;
    int flag = 0, idV;
    MPI_Status rep;
    
    MPI_Send(&idCli, 1, MPI_INT, COLA, FORMARSE, MPI_COMM_WORLD);

    while(flag == 0){
        MPI_Recv(&idV, 1, MPI_INT, COLA, MPI_ANY_TAG, MPI_COMM_WORLD, &rep);
        
        if(rep.MPI_TAG == OK){
            MPI_Recv(&idV, 1, MPI_INT, MPI_ANY_SOURCE, LLAMANDO, MPI_COMM_WORLD, &rep);
            printf("Cliente: %d Me atiende Ventana: %d\n", idCli, idV);
            MPI_Send(&idCli, 1, MPI_INT, idV, YENDO, MPI_COMM_WORLD);
            MPI_Recv(&idV, 1, MPI_INT, idV, TERMINA, MPI_COMM_WORLD, &rep);
            flag = 1;
        }else if(rep.MPI_TAG == INTENTA){ 
            MPI_Send(&idCli, 1, MPI_INT, COLA, FORMARSE, MPI_COMM_WORLD);
        }
    }
}

void pVentana(int id, int size){
    int idV = id, idP, nC = 0;
    int flag = 0;
    MPI_Status rep;
    
    MPI_Send(&idV, 1, MPI_INT, COLA, SIGUIENTE, MPI_COMM_WORLD);

    while(flag == 0){
        MPI_Recv(&idP, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &rep);
        
        if(rep.MPI_TAG == CONFIRMA){
            printf("Ventana: %d Atendiendo a Cliente: %d\n", idV, idP);
            MPI_Send(&idV, 1, MPI_INT, idP, LLAMANDO, MPI_COMM_WORLD);
            MPI_Recv(&idP, 1, MPI_INT, idP, YENDO, MPI_COMM_WORLD, &rep);
            sleep(rand()%3);
            MPI_Send(&idV, 1, MPI_INT, idP, TERMINA, MPI_COMM_WORLD);
            nC++;
            MPI_Send(&idV, 1, MPI_INT, COLA, SIGUIENTE, MPI_COMM_WORLD);
        }else if(rep.MPI_TAG == ESPERA){
            MPI_Send(&idV, 1, MPI_INT, COLA, SIGUIENTE, MPI_COMM_WORLD);
        }else{
            flag = 1;
        }      
    }
    printf("Ventana %d atendió: %d\n", idV, nC);
}