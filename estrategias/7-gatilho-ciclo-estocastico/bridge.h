#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"

//CAN Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int create_can_socket(char* ifname);
int create_udp_socket(int port);
