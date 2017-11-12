#include "bridge.h"


int create_can_socket(char* ifname){

  int s; //socket descriptor
  struct sockaddr_can r_addr; //address of the can if that'll receive the frames
  struct ifreq ifr; //structure for ioctl to configure can nwif

  /*opening the socket*/
  s = socket(PF_CAN, SOCK_RAW, CAN_RAW); //or UDP? (PF_CAN, SOCK_DGRAM, CAN_BCM)

  /*error handling*/
  if(s < 0){
    perror("Error opening the can socket.");
    return -1;
  }

  strcpy(ifr.ifr_name, ifname); //passing the if's name to ifreq struct
  ioctl(s, SIOCGIFINDEX, &ifr); //retrieve te if's index into ifr.ifr_ifindex

  /*setting up sockaddr_can for binding?*/
  r_addr.can_family = AF_CAN;
  r_addr.can_ifindex = ifr.ifr_ifindex;

  printf("create_can_socket function:%s at index %d\n", ifname, ifr.ifr_ifindex); //just printing the index

  /*socket bind*/
  int bind_result;
  bind_result = bind(s, (struct sockaddr*)&r_addr, sizeof(r_addr));

  /*error handling*/
  if(bind_result < 0){
    perror("Error binding the socket");
    return -2;
  }

  return s;
}



int create_udp_socket(int port){
  /*socket descriptor*/
  int s;
  /*struct used for binding the socket to a local address*/
  struct sockaddr_in host_address;

  /*create the socket*/
  s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(s < 0){
    printf("Erro no receiver! s = %d \n", s);
    return -1;
  }

  /*init the host_address the socket os being bound to*/
  memset((void*)&host_address, 0, sizeof(host_address));

  /*set the address family*/
  host_address.sin_family = PF_INET;
  /*accept any income message*/
  host_address.sin_addr.s_addr = INADDR_ANY;
  /*the port the socket i to be bound to:*/
  host_address.sin_port=htons(port);

  /*bind it*/
  if(bind(s, (struct sockaddr*)&host_address, sizeof(host_address)) < 0){
    printf("Error binding the receiver! s = %d \n", s);
  }

  return s;
}
