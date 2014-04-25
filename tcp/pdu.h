#include <sys/types.h>
#include "header.h"

#ifndef PDU_H
#define PDU_H

#define PAYLOAD 968
typedef struct pdu
{
    header h;
    char data[1000-sizeof(header)];
}pdu;

#endif
