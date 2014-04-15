/* Example: server.c receiving and sending datagrams on system generated port in UDP */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "tcpd.h"

// Prototypes
int setup_socket(struct sockaddr_in* addr, int* addrlen,  int port);
int set_fwd_addr(struct sockaddr_in in_addr, int in_addrlen,
		 struct sockaddr_in* f_addr, int* f_addrlen,
		 int port
		 );


/* Daemon service*/
int main(void) 
{
  unsigned int l_sockfd; // local socket
  unsigned int r_sockfd; // remote socket 
  fd_set read_fd_set;
  
  char buf[MAX_MES_LEN+CHKSUM_LEN];
  uint32_t buflen;
  
  ssize_t nbytes_recv = 0;
  ssize_t nbytes_sent = 0;
  
  struct sockaddr_in local_addr;
  int local_len;			
  
  struct sockaddr_in remote_addr;		
  int remote_len;	
  
  struct sockaddr_in recv_addr;
  int recv_len;
  
  struct sockaddr_in fwd_addr;
  int fwd_len;
  
  uint16_t checksum;
  uint16_t checksum_r;
  char *  test = "123456789";

    
  /* create local and remote sockets */
  l_sockfd = setup_socket(&local_addr, &local_len, L_PORT);
  r_sockfd = setup_socket(&remote_addr, &remote_len, R_PORT);
  
  checksum = calc_checksum(test, 9);
  printf("Checksum = %x\n",checksum);


  
  // ensure all bytes are read
  while (1){
    FD_ZERO (&read_fd_set);
    FD_SET (l_sockfd, &read_fd_set);
    FD_SET (r_sockfd, &read_fd_set);
    
    if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
      {
	perror("tcpd:\tselect");
	exit(EXIT_FAILURE);
      }
    nbytes_recv = 0;
    nbytes_sent = 0;
    // receive from local
    if (FD_ISSET(l_sockfd, &read_fd_set))
      {
	if ((nbytes_recv = recvfrom(l_sockfd, buf , MAX_MES_LEN, 0, (struct sockaddr *)&recv_addr, &recv_len))==-1){
	  perror("recv");
	  exit(1);
	}

	//determine the type of request (CONNECT/ACCEPT/SHUTDOWN/DATA/)
	
	//respond request

	// if connecting to other daemon, 
	//add to buffer
	//send available window
	

	if (set_fwd_addr(recv_addr, recv_len, &fwd_addr, &fwd_len, T_PORT) == -1){
	  perror("tcpd: set_fwd_addr");
	  exit(1);
	}
	
	checksum = calc_checksum(buf,nbytes_recv);
	memcpy(buf+nbytes_recv, &checksum, CHKSUM_LEN);
	printf("TCPD:Local sent checksum:\t%x\n",checksum);
	
	//printf("Daemon: forwarded data to TROLL.\n");
	// forward buffer to the remote
	sendto(r_sockfd, buf, MAX_MES_LEN+CHKSUM_LEN, 0, (struct sockaddr *)&fwd_addr, fwd_len);
	
      }
    // receive from remote
    if (FD_ISSET(r_sockfd, &read_fd_set))
      {
      	if ((nbytes_recv = recvfrom(r_sockfd, buf , MAX_MES_LEN+CHKSUM_LEN, 0, (struct sockaddr *)&recv_addr, (socklen_t *)&recv_len))==-1){
	  perror("recv");
	  exit(1);
	}


	//Read header flag
	
	//if ACK slide the window
	
	printf("TCPD: received from remote:\t%d\n",nbytes_recv);
	
	checksum_r = *(uint16_t*)(buf+nbytes_recv-CHKSUM_LEN);
	checksum = calc_checksum(buf, nbytes_recv);
	printf("TCPD:Remote checksum calculated:\t%x\n",checksum);
	
	if (checksum){
	   printf("Checksums did not match(rec:calc):\t%d:%d\n",checksum_r,checksum);
	   }

	if (set_fwd_addr(recv_addr, recv_len, &fwd_addr, &fwd_len, L_PORT) == -1){
	  perror("tcpd: set_fwd_addr");
	  exit(1);
	}

	//printf("Daemon: forwarded data to LOCAL.\n");
	// forward buffer to the remote
	sendto(l_sockfd, buf, MAX_MES_LEN, 0, (struct sockaddr *)&fwd_addr, fwd_len);
	
      }
    
    
  }
  
  return(1);
}


int setup_socket(struct sockaddr_in* addr, int* addrlen,  int port)
{
  
  /*create local socket*/
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) {
    perror("opening datagram socket");
    exit(1);
  }
  
  /*  lose the pesky "Address already in use" error message (from Beej's)*/
  int yes=1;
  if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  } 

  *addrlen = sizeof(struct sockaddr_in);
  
  /* create name with parameters and bind name to socket */
  memset(addr, '\0', *addrlen);
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(sockfd, (struct sockaddr *) addr, *addrlen) < 0) {
    perror("getting socket name");
    exit(2);
  }

  return sockfd;
}
 
int set_fwd_addr(struct sockaddr_in in_addr, int  in_addrlen, struct sockaddr_in*f_addr, int* f_addrlen, int port)
 {
   
   *f_addrlen = sizeof(struct sockaddr_in);
   
   memset(f_addr, '\0', *f_addrlen);
   f_addr->sin_family = AF_INET;
   f_addr->sin_port = htons(port); 
   f_addr->sin_addr.s_addr = inet_addr(S_ADDR);  
   
   return 0;
 }

