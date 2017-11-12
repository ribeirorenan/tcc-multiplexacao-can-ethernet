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
#define BUFFER_LENGTH 1500
#define TIMER_LENGTH 93
#define MAX_ID 0x900
#define PORCENT_MAX_ON_QUEUE 5
#define FRAMES_PER_BUFFER 93
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
/*-----------------------------------------------------------
                For Buffer Approach
 -----------------------------------------------------------*/

 char eth_buffer[BUFFER_LENGTH];
 char* eth_p;
 int frame_count = 0;
 int timer = TIMER_LENGTH;
 struct can_frame frame; //structure to store the received can frame

 /*-----------------------------------------------------------
                 statistic
  -----------------------------------------------------------*/
  struct timeval tvals_before[FRAMES_PER_BUFFER];

  struct timeval tvals_last_send[MAX_ID + 1];
  struct timeval tvals_not_cheating_cycle[MAX_ID + 1];

  struct timeval tval_after;
  struct timeval tval_result;

  struct timeval cycle;

  int first_times[MAX_ID + 1];
  int first_times_cycle[MAX_ID + 1];

  struct timeval tvals_last_arrived[MAX_ID + 1];
  long int arrived_cycles[MAX_ID + 1];


  //
  struct timeval tval_time_on_queue;
  long int time_on_queue;
  long int time_cyle;

  struct can_frame first_from_buffer;
  while(1){
      /*reading the can frame*/
      nbytes = read(s_can, &frame, sizeof(struct can_frame));

      /*save the receive time for each can frame'*/
      gettimeofday(&tvals_before[frame_count], NULL);

      timer--;
      
      /*-----------------------------------------------------------
               Write the frames on the ethernet Payload
       (Skip the first byte, that is reserved to the frame_count)
       -----------------------------------------------------------*/
      eth_p = memcpy(eth_buffer + ((frame_count * sizeof(struct can_frame)) + 1), &frame, sizeof(struct can_frame));
      frame_count++;

      /*-----------------------------------------------------------
                      Verify Time On Queue
       -----------------------------------------------------------*/
       memcpy(&first_from_buffer, eth_buffer + (0 * sizeof(struct can_frame)) + 1, sizeof(struct can_frame));
       if(first_times[first_from_buffer.can_id] == 1){
         timersub(&tvals_before[frame_count-1], &tvals_before[0], &tval_time_on_queue);
         time_on_queue = (long int)tval_time_on_queue.tv_sec + (long int)tval_time_on_queue.tv_usec;
        long int delta = arrived_cycles[first_from_buffer.can_id] * PORCENT_MAX_ON_QUEUE / 100;
        if(time_on_queue >= delta){
          timer = 0;
        }
       }
       else{
         timer = 0;
       }

      if(timer <= 0){
        eth_buffer[0] = frame_count;
        sendto(s_eth, eth_buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&target_host_address, sizeof(struct sockaddr));
        gettimeofday(&tval_after, NULL);
        //get the time for each message
        for(int i = 0; i < frame_count; i++){
          timersub(&tval_after, &tvals_before[i], &tval_result);
          struct can_frame frame_test;
          memcpy(&frame_test, eth_buffer + (i * sizeof(struct can_frame)) + 1, sizeof(struct can_frame));

          if(first_times[frame_test.can_id] == 0){
            first_times[frame_test.can_id] = 1;
            tvals_last_send[frame_test.can_id] = tval_after;
            tvals_last_arrived[frame_test.can_id] = tvals_before[i];
          }
          else{
            timersub(&tvals_before[i], &tvals_last_arrived[frame_test.can_id], &cycle);
            tvals_last_arrived[frame_test.can_id] = tvals_before[i];
            arrived_cycles[frame_test.can_id] = (long int) cycle.tv_sec + (long int) cycle.tv_usec;
            if(first_times_cycle[frame_test.can_id] == 0){
              timersub(&tval_after, &tvals_last_send[frame_test.can_id], &cycle);
              printf("%d\t%ld.%06ld\t%ld.%06ld\t%d\n", frame_test.can_id, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, (long int)cycle.tv_sec, (long int)cycle.tv_usec, frame_count);
              first_times_cycle[frame_test.can_id] = 1;
              tvals_not_cheating_cycle[frame_test.can_id] = cycle;

            }
            else{
              printf("%d\t%ld.%06ld\t%ld.%06ld\t%d\n", frame_test.can_id, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, (long int)tvals_not_cheating_cycle[frame_test.can_id].tv_sec, (long int)tvals_not_cheating_cycle[frame_test.can_id].tv_usec, frame_count);
            }
            fflush(stdout);
          }
          tvals_last_send[frame_test.can_id] = tval_after;
        }
        for (size_t i = 0; i < MAX_ID; i++) {
          first_times_cycle[i] = 0;
        }
        eth_buffer[0] = '\0';
        frame_count = 0;
        timer = TIMER_LENGTH;
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

  /*-----------------------------------------------------------
                  Buffer Approach
   -----------------------------------------------------------*/
   char eth_buffer[BUFFER_LENGTH];
   char* eth_p;
   int frame_count = 0;

   /*lenght if the income packet*/
   int length = 0;

   while (1) {

     /*wait for incoming message*/
     length = recvfrom(s_eth, eth_buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&host_address, &hst_addr_size);

     if(length > 0){
       frame_count = eth_buffer[0];
       eth_p = eth_buffer;
       for(int i = 0; i < frame_count; i++){
         struct can_frame frame_test;

         memcpy(&frame_test, eth_p + (i * sizeof(struct can_frame)) + 1, sizeof(struct can_frame));

         printf("Unpacked ETH->Frame: ID=0x%X DLC=%d data[0]=0x%X data[1]=0x%X\n", frame_test.can_id, frame_test.can_dlc, frame_test.data[0], frame_test.data[1]);

         nbytes = write(s_can, &frame_test, sizeof(struct can_frame));
      }

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
