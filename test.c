#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "tcp/pdu.h"

#define T_PORT 10001 //Daemon port (local)
#define R_PORT 15001 //Remote port
#define WINDOW_SIZE 20

void build_pdu(pdu* p, uint32_t* seq, uint32_t* ack, char* buf, ssize_t buflen);
int main ()
{
  struct timeval  time;
  gettimeofday(&time, NULL);
  printf("suseconds_t size = %i\n", sizeof(struct timeval));
  printf("Time of day (%i) or (%i)\n", time.tv_usec, time.tv_sec);
  uint32_t seq = 0xFFFFFFFF;
  char local_buf[1000-sizeof(header)];
  
  printf ("Size of header= %i\n", sizeof(header));
  pdu  p;
  pdu* p_ptr = &p;
  
  memset(&p, 0x00, sizeof(pdu));
  
  p.h.s_port = 1;

  build_pdu(&p,&seq, NULL , local_buf, 1000-sizeof(header));
  printf("Direct reference s_port: %i\n", p.h.s_port);
  printf("Indirect reference s_port: %i\n", p_ptr->h.s_port);
  printf("Indirect reference seq_num: %u\n", p_ptr->h.seq_num);
  printf("Indirect reference chk: %4u\n", p_ptr->h.chk);

  //p.h.flags +=ACK;
  p.h.flags +=SYN;
  printf("ACK = \t%u\n",p.h.flags);

  int ackd = p.h.flags && ACK;
  int syn = p.h.flags && SYN;

  if (ackd){
    printf("ACKD? \t%u\n", ackd);
  }
  
  if (syn){
    printf("SYN? \t%u\n", syn);
  }
  return 0;
}

void build_pdu(pdu* p, uint32_t* seq, uint32_t* ack, char* buf, ssize_t buflen)
{
  p->h. s_port = R_PORT;
  p->h. d_port = T_PORT;
  p->h.win = WINDOW_SIZE;
  if (seq ==NULL)
    {
      p->h. flags = ACK;
      p->h.ack_num = *ack;
    } 
  else if (ack == NULL)
    {
      p->h.seq_num = *seq;
      memcpy(p->data, buf, buflen);
    }
  else 
    {
      // must ACK or SEQ for this program
      perror("TCPD: error building pdu Must be ACK or SEQ.");
      exit(1);
    }
}
