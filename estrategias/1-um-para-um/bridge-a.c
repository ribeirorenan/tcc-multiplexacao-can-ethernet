/*
* Autor: Renan Airton Batista Ribeiro
* Trabalho de Conclusão de Curso: Multiplexação de pacotes CAN em Quadros Ethernet.
*/
#include "bridge.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h> //timer


//ports for UDP
#define LOCAL_PORT 5000
#define REMOTE_PORT 5001
#define MAX_ID 0x7FF
#define CAN_IF "vcan0"

/*-----------------------------------------------------------
                CAN Socket
 -----------------------------------------------------------*/
//the can if that'll receive the frame
char* can_ifname = CAN_IF;
int s_can; //socket descriptor of the can if
int nbytes; //number of read bytes from the received can frame

/*-----------------------------------------------------------
                ETH Socket
 -----------------------------------------------------------*/
int s_eth; //udp socket descriptor
struct sockaddr_in target_host_address; // addr that will receive the packet
unsigned char* target_address_holder; //same

/*Threads for listen and write*/
pthread_t t1_id;
pthread_t t2_id;


/*-----------------------------------------------------------
|                                                           |
|                Listen CAN and send ETH                    |
|                                                           |
 -----------------------------------------------------------*/
void *sender_eth(){
  struct can_frame frame; //structure to store the received can frame
  /*-----------------------------------------------------------
                  statistic
   -----------------------------------------------------------*/

  struct timeval tvals_last_send[MAX_ID + 1];

  struct timeval tval_after;
  struct timeval tval_before;
  struct timeval tval_result;

  struct timeval cycle;

  int first_times[MAX_ID + 1];



  while(1){
      /*reading the can frame*/
      nbytes = read(s_can, &frame, sizeof(struct can_frame));
      gettimeofday(&tval_before, NULL);


      sendto(s_eth, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&target_host_address, sizeof(struct sockaddr));
      /*
       *getting time
       */
      gettimeofday(&tval_after, NULL);

      timersub(&tval_after, &tval_before, &tval_result);

      if(first_times[frame.can_id] == 0){
        tvals_last_send[frame.can_id] = tval_after;
        first_times[frame.can_id] = 1;
      }
      else{
        timersub(&tval_after, &tvals_last_send[frame.can_id], &cycle);
        tvals_last_send[frame.can_id] = tval_after;
        printf("%d\t%ld.%06ld\t%ld.%06ld\t1\n", frame.can_id, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, (long int)cycle.tv_sec, (long int)cycle.tv_usec );
        fflush(stdout);
      }
  }

}


/*-----------------------------------------------------------
|                                                           |
|                Listen ETH and send CAN                    |
|                                                           |
 -----------------------------------------------------------*/
 void *receiver_eth(){
  /*For the eth socket*/
  struct sockaddr_in host_address;
  int hst_addr_size = sizeof(host_address);
  //Just to be able to cast the frame
  struct can_frame* pointer_frame;
  struct can_frame frame;

  /*buffer for the can frame that'll be received*/
  char buffer[sizeof(struct can_frame)];

   int length = 0;
   while (1) {

     /*wait for incoming message*/
     length = recvfrom(s_eth, buffer, sizeof(struct can_frame), 0, (struct sockaddr*)&host_address, &hst_addr_size);

     pointer_frame = (struct can_frame*)buffer;

     frame = *pointer_frame;

     if(length > 0){
      printf("Received ETH frame: \n");
      printf("ID=0x%X DLC=%d data[0]=0x%X data[1]=0x%X\n", frame.can_id, frame.can_dlc, frame.data[0], frame.data[1]);
      write(s_can, &frame, sizeof(struct can_frame));
    }

  }
}

int main(){
  /*-----------------------------------------------------------
        Create a CAN socket to send One can frame per packet
   -----------------------------------------------------------*/
  s_can = create_can_socket(can_ifname);
  /*-----------------------------------------------------------
        Create a UDP socket to send One can frame per packet
   -----------------------------------------------------------*/
  s_eth = create_udp_socket(LOCAL_PORT); //opening udp local socket

  //error handling
  if(s_eth == -1 ){
    perror("error opening the udp socket");
    return -1;
  }

  /*init target address structure*/
  /*using loopback on my own computer*/
  target_host_address.sin_family = PF_INET;
  target_host_address.sin_port = htons(REMOTE_PORT);
  target_address_holder = (unsigned char*)&target_host_address.sin_addr.s_addr;
  target_address_holder[0] = 127;
  target_address_holder[1] = 0;
  target_address_holder[2] = 0;
  target_address_holder[3] = 1;

   int t_err1;
   int t_err2;
   t_err1 = pthread_create(&t1_id, NULL, &sender_eth, NULL);
   t_err2 = pthread_create(&t2_id, NULL, &receiver_eth, NULL);

   pthread_join(t1_id, NULL);
   pthread_join(t2_id, NULL);

  return 0;
}
