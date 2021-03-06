/* Example: server.c receiving and sending datagrams on system generated port in UDP */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tcp/tcpd.h"

/* server program called with port number to use */
int main(void) 
{
  unsigned int sockfd, file_len, buflen;
  char cli_buf[PAYLOAD];
  char srv_buf[PAYLOAD];
  size_t nbytes_written = 0;
  size_t total_written = 0;
  ssize_t nbytes_recv = 0;
  ssize_t nbytes_sent = 0;
  int fidr = 0;
  unsigned int connect_s;
  struct sockaddr_in addr; //Connection socket descriptor
  socklen_t addrlen;
  struct sockaddr_in client_addr; //Client Internet address
  struct in_addr client_ip_addr; //Client IP address
  socklen_t client_addr_len; // Internet address length
 
  
  int header_len = FILE_NAME_LEN + DATA_LEN;
  char file_name[FILE_NAME_LEN];
  char new_path[FILE_NAME_LEN+sizeof("new/")] = "new/";
  
  
  
  /* NO CHECK ARGS
     if(argc < 2) { 
     printf("usage: svr port_number\n");
     exit(1);
     }
  */
  
  /*create socket*/
  sockfd = SOCKET(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    perror("opening datagram socket");
    exit(1);
  }
  
  /*  lose the pesky "Address already in use" error message (taken from Beej's)*/
  int yes=1;
  if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  } 
  
  /* create name with parameters and bind name to socket */
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(L_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(BIND(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("getting socket name");
    exit(2);
  }
  addrlen=sizeof(struct sockaddr_in);
  client_addr_len = sizeof(client_addr);
 


 /* TODO:  Add Listen / Connection functionality
  //listen(sockfd, 1);
  // Accept a connection. The accept() will block and then return with
  
  // connect_s assigned and client_addr filled-in.
  
  
  if ((connect_s = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len))==-1){
  perror("connect");
  exit(1);
  }
  
  // memcpy(&client_ip_addr, &client_addr.sin_addr.s_addr, 4);
  //printf("Accept completed!!! IP address of client = %s port = %d \n",inet_ntoa(client_ip_addr), ntohs(client_addr.sin_port));
 */
  printf("Waiting for connection...\n");
  //ensure full header is received  
  while ((nbytes_recv += RECV(sockfd, srv_buf+nbytes_recv,  PAYLOAD-nbytes_recv, 0, (struct sockaddr *)&client_addr, (socklen_t *) &client_addr_len)) < header_len);
  if (nbytes_recv < 0){
    perror("recv");
    exit(1);
  }
  printf("Connected.");
  printf("Server: nbytes_recv:\t%d\n",nbytes_recv);
  
  //read in total file length
  memcpy(&file_len, srv_buf, DATA_LEN);
  file_len -= FILE_NAME_LEN;
  printf("Server: file length\t%d\n", file_len);
  
  //read in file name
  memcpy(file_name, srv_buf+DATA_LEN,FILE_NAME_LEN);
  printf("Server: file name \t%s\n", file_name);
  
  //creating new file path and directory
  strcat(new_path, file_name);
  mkdir("new",S_IRWXU);
  printf("Server: new path\t%s\n", new_path);  
  
  //creating new file
  if ((fidr = open(new_path, O_CREAT|O_TRUNC|O_WRONLY|O_APPEND,  S_IRWXU)) <0){
    perror("server :file creat");
    exit(1);
  }
  
  nbytes_written = write(fidr, srv_buf + header_len, nbytes_recv - header_len);	
  total_written += nbytes_written;
  
  
  //ensure all bytes are read
  while (total_written < file_len){
    
    //try to get a full buffer
    if ((nbytes_recv = RECV(sockfd, srv_buf , PAYLOAD, 0, (struct sockaddr *)&addr, (socklen_t *)&addrlen)) == -1){
      perror("recv");
      exit(1);
    }
    if (nbytes_recv < (file_len - total_written)) 
      {
	nbytes_written = write(fidr, srv_buf , nbytes_recv);		
	total_written += nbytes_written;
      }
    else 
      {
	nbytes_written = write(fidr, srv_buf , (file_len - total_written));		
	total_written += nbytes_written;
      }
	
  }
  
  
  printf("Server: bytes written\t%d\n", total_written);
  close(fidr);
  
  // CLOSE CONNECTIONS
  close(sockfd);
  //close(connect_s);
  
  return(0);
}
