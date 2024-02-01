/*Autor: Mejía Velázquez José Rodrigo
**Fecha de entrega: 1 de febrero de 2024
**Descripción: Segunda parte del proyecto. Haciendo uso de intercambio de mensajes.
*/
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
#define TERMINA 90
#define FINALIZA 100
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

/*Proceso cola, donde se encuentra una cola circular de N lugares, también
**asigna turnos a los clientes que se forman en la cola y le informa a las
**ventanillas cual es el siguiente cliente a atender.
*/
void pCola(int id, int size){
    int idCola = id, idP, idCli, idV,  llenas = 0;
    int frente = 0, posterior = 0, flag = 0, nCli = 0, turno = 0;
    MPI_Status rep;
    int buffer[N];
    
    while(nCli < size - 4){
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &rep);
        MPI_Recv(&idP, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &rep);
        if(rep.MPI_TAG == FORMARSE){    //Si la etiqueta es FORMARSE es un cliente que quiere
            if(llenas < N){            //un lugar en la cola, si hay espacio en la cola sigue adelante.
                buffer[posterior] = idP;
                posterior = (posterior + 1) % N;
                llenas++;  //Al asignarle su turno al cliente se aumenta la cantidad de lugares
                turno++;    //ocupados en la fila y se le asigna un turno
                MPI_Send(&turno, 1, MPI_INT, idP, OK, MPI_COMM_WORLD);  //Se le envía su turno al
            }else{                                                      //cliente
                MPI_Send(&idP, 1, MPI_INT, idP, INTENTA, MPI_COMM_WORLD);   //Si la cola está llena se le envía   
            }                                                               //al cliente la etiqueta INTENTA para                                                  
        }else{//Si la etiqueta es SIGUIENTE es una ventana solicitando    //que intente formarse de nuevo.
            if(llenas > 0){    //al siguiente cliente, si hay alguien en la cola sigue adelante.
                idCli = buffer[frente];
                MPI_Send(&idCli, 1, MPI_INT, idP, CONFIRMA, MPI_COMM_WORLD);    //Confirmando el cliente que
                frente = (frente + 1) % N;                                      //atenderá la ventanilla.
                llenas--;    //Al confirma que la ventanilla atenderá al cliente se libera espacio en la cola.
                nCli++;       //Se incrementa el número de clientes atendidos.
            }else{
                MPI_Send(&idP, 1, MPI_INT, idP, ESPERA, MPI_COMM_WORLD);    //Si la cola esta vacía se le dice a la
            }                                                               //ventanilla que espere a que llegue un cliente
        }                                                                   //con la etiqueta ESPERA.  
    }
    
    for(int i = 1; i <= VENTANA;i++){                               //Al finalizar de atender a todos los clientes que llegaron
        MPI_Send(&nCli, 1, MPI_INT, i, FINALIZA, MPI_COMM_WORLD);   //se les informa a las ventanillas que finalicen.
    }
}

/*Proceso pCLiente, proceso que representa al cliente que llega al banco,
**solicita su turno y espera a ser atendido.
*/
void pCliente(int id, int size){
    int idCli = id, idP, turno;
    int flag = 0, idV;
    MPI_Status rep;
    
    MPI_Send(&idCli, 1, MPI_INT, COLA, FORMARSE, MPI_COMM_WORLD);   //Solicita su turno en la cola.

    while(flag == 0){
        MPI_Recv(&idV, 1, MPI_INT, COLA, MPI_ANY_TAG, MPI_COMM_WORLD, &rep);    //Recibe una confirmación o una señal
                                                                                //de volver a intentar.
        if(rep.MPI_TAG == OK){  //Si el cliente recibe una confirmación, sigue adelante.
            MPI_Recv(&idV, 1, MPI_INT, MPI_ANY_SOURCE, LLAMANDO, MPI_COMM_WORLD, &rep); //Recibe el llamado de la ventanilla.
            printf("Cliente: %d Me atiende Ventana: %d\n", idCli, idV);
            MPI_Send(&idCli, 1, MPI_INT, idV, YENDO, MPI_COMM_WORLD);   //El cliente informa que se dirige a la ventanilla.
            MPI_Recv(&idV, 1, MPI_INT, idV, TERMINA, MPI_COMM_WORLD, &rep); //Al cliente se le informa que su turno ha terminado.
            flag = 1;
        }else if(rep.MPI_TAG == INTENTA){   //Se le informa al cliente que debe intentar solicitar de nuevo su turno.
            MPI_Send(&idCli, 1, MPI_INT, COLA, FORMARSE, MPI_COMM_WORLD);   //El cliente envía nuevamente su solicitud de turno.
        }
    }
}

/*Proceso pVentana, proceso que representa a la ventanilla que llama
**al cliente con el siguiente turno.
*/
void pVentana(int id, int size){
    int idV = id, idP, nC = 0;
    int flag = 0;
    MPI_Status rep;
    
    MPI_Send(&idV, 1, MPI_INT, COLA, SIGUIENTE, MPI_COMM_WORLD);    //Le solicita a la cola el id del cliente
                                                                    //con el siguiente turno.
    while(flag == 0){
        MPI_Recv(&idP, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &rep);  //Recibe una confirmación con el id del
                                                                                        //cliente o una señal de espera.
        if(rep.MPI_TAG == CONFIRMA){    //Si la ventanilla recibe una confirmación sigue adelante.
            printf("Ventana: %d Atendiendo a Cliente: %d\n", idV, idP);
            MPI_Send(&idV, 1, MPI_INT, idP, LLAMANDO, MPI_COMM_WORLD);  //Le envía el llamado al cliente.
            MPI_Recv(&idP, 1, MPI_INT, idP, YENDO, MPI_COMM_WORLD, &rep);   //El cliente confirma que se dirige para allá.
            sleep(rand()%3);    //El cliente está siendo atendido.
            MPI_Send(&idV, 1, MPI_INT, idP, TERMINA, MPI_COMM_WORLD);   //Le informa al cliente que terminó de atenderlo.
            nC++;   //La ventana incrementa el número de clientes que ha atendido.
            MPI_Send(&idV, 1, MPI_INT, COLA, SIGUIENTE, MPI_COMM_WORLD);    //Le solicita a la cola el siguiente cliente.
        }else if(rep.MPI_TAG == ESPERA){    //Recibe de la cola la señal de que espere y envíe nuevamente una solicitud.
            MPI_Send(&idV, 1, MPI_INT, COLA, SIGUIENTE, MPI_COMM_WORLD);    //Envía nuevamente la solicitud del siguiente cliente.
        }else{  //Recibe la señal de finalización de la cola.
            flag = 1;
        }      
    }
    printf("Ventana %d atendió: %d\n", idV, nC);
}