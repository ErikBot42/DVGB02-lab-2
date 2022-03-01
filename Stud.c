/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#define TRACE 1
#include "Stud.h"
#include <string.h>
#include <stdbool.h>


#define A 0
#define B 1

int iA_fully_sent = 0;
struct pkt A_prev_sent;

int iB_fully_recived = 0;

float time_wait = 2000.0;

bool A_ackWasRecived = true;

//#define PRINT_RUNNING_FUNCTIONS

// debug print
void p(char * str)
{
#ifdef PRINT_RUNNING_FUNCTIONS
    printf("%s\n", str);
#endif
}

int calcChecksum(struct pkt packet)
{
    /* just bitwise xor*/
    int acc = 0;
    acc ^= packet.seqnum ^ (packet.acknum << 5);
    
    for (int i = 0; i<sizeof(packet.payload)/sizeof(packet.payload[0]); i++)
    {
        acc ^= packet.payload[i];
        acc ^= acc << 1; // 32 > 20 
    }

    return acc;
}

struct pkt addChecksum(struct pkt packet){packet.checksum = calcChecksum(packet); return packet;}
bool testCheckSum(struct pkt packet){return packet.checksum == calcChecksum(packet);}

bool A_is_sending = false;

/* called from layer 5, passed the data to be sent to other side */
void A_output( struct msg message)
{
    if (A_is_sending) {printf("Skipping packet\n"); return;};
    A_is_sending = true;
    
    p("\nA_output()");
    struct pkt packet;
    packet.acknum = 0;
    packet.seqnum = iA_fully_sent+1;
    memcpy(packet.payload, message.data, sizeof(char)*20);

    packet = addChecksum(packet);
    
    printf("A creating and sending packet\n");
    printf("current seq: %d, (%d)\n", packet.seqnum, A_ackWasRecived);
    tolayer3(A, packet);  
    A_prev_sent = packet;
    starttimer(A, time_wait); 
    A_ackWasRecived = false;
}

void B_output(struct msg message)  /* need be completed only for extra credit */
{
    p("\nB_output()");
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    // Receive from B
    p("\nA_input()");
    if (testCheckSum(packet))
    {

        printf("A recived valid packet (%d)\n", packet.seqnum);
        if (packet.acknum == 0)
        {
            p("received positive ack");
            if (iA_fully_sent+1 == packet.seqnum)
            {
                stoptimer(A);
                A_ackWasRecived = true;
                A_is_sending = false;
                iA_fully_sent++;
            }
            else
            {
                p("Multiple ACK for same number was recived");
            }
        }
        else
        {
            p("received negative ack, resending");
            tolayer3(A, A_prev_sent);
        }
    }
    else
    {
        p("A recived corrupted packet, resending");
        tolayer3(A, A_prev_sent);
    }
}


/* called when A's timer goes off */
void A_timerinterrupt()
{
    p("\nA_timerinterrupt()");
    p("A resending packet");

    tolayer3(A, A_prev_sent);
    starttimer(A, time_wait); 
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    p("\nA_init()");
    // 1. Init A
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    // Receive from A
    p("\nB_input()");
    if (testCheckSum(packet))
    {
        printf("B recived valid packet (%d)\n", packet.seqnum);
        iB_fully_recived++;
        tolayer5(B, packet.payload);
        p("Positive ack");
        packet.acknum = 0;
        tolayer3(B, packet);
    }
    else
    {
        p("B recived corrupted packet");
        p("Negative ack");
        packet.acknum = 1;
        packet.seqnum = iB_fully_recived;
        tolayer3(B, packet);
    }
}

/* called when B's timer goes off */
void B_timerinterrupt()
{
    p("\nB_timerinterrupt()");

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    p("\nB_init()");

}
