/* TFTP Server */

/* TODO:ERROR Code 4 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <malloc.h>
#include <stdlib.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include "Client.h"

using namespace std;

tftp_c *tftp_connect(char *host_name, char *port4addr,char *mode, int type, char *file_name);
int tftp_recv(tftp_c *tc);


int main(void) {
    char host_name[20];
    char file_name[50];
    char port4addr[20]="69";
    int type;
    tftp_c *tc = NULL;

    log_fp = fopen("log.txt","w+");

    cout<<"\nplease enter TFTP-Server host:";
    cin>>host_name;
    cout<<"\ndownload or upload(download -> 0 || upload -> 1)?:";
    cin>>type;
    cout<<"\nplease enter file name:";
    cin>>file_name;

    /* ========================DEBUG================================= */
    /*cout<<"========================DEBUG=================================\n"<<endl;
    cout<<"hostname = "<<host_name<<"\nfilename = "<<file_name<<"\ntype ="<<type<<endl;
    cout<<"========================DEBUG=================================\n"<<endl;*/


    /* Connect to TFTP Server */ 
    tc = tftp_connect(host_name, port4addr,MODE_OCTET,type,file_name); 
    if (!tc) {
        cout<<"ERROR:fail to connect to server"<<endl;
        fprintf(log_fp,"ERROR:fail to connect to server\n");
        return -1;
    }
    cout<<"success to connect to server"<<endl;
    fprintf(log_fp,"success to connect to server\n");

    /* Download or Upload */
    if (!type) {
        tftp_recv(tc);
    }
    return 0;
}

tftp_c *tftp_connect(char *host_name, char *port4addr,char *mode, int type, char *file_name) {
    tftp_c *tc = NULL;
    addrinfo hints;
    addrinfo *result = NULL;
    tc = (tftp_c *)malloc(sizeof(tftp_c));

    /* 
        Create a socket: 
        AF_INET:IPV4
        SOCK_DGREAM UDP
        0:auto choose protocal for type
    */
    if ((tc->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout<<"ERROR:fail to create socket"<<endl;
        fprintf(log_fp,"ERROR:fail to create socket\n");
        free(tc);
        return NULL;
    }

    /* settings 4 getaddrinfo() */
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    /* get server addr 4 sendto() */
    if (getaddrinfo(host_name, port4addr, &hints, &result) != 0) {
        cout<<"ERROR:fail to get server address"<<endl;
        fprintf(log_fp,"ERROR:fail to get server address\n");
        free(tc);
        return NULL;
    }
    memcpy(&tc->addr_server, result->ai_addr, result->ai_addrlen);
    tc->addr_len = sizeof(sockaddr_in);
    tc->mode = mode;
    tc->file_name = file_name;
    tc->type = type;

    /* ========================DEBUG================================= */
    /*cout<<"========================DEBUG=================================\n"<<endl;
    cout<<"tc->mode = "<<tc->mode<<"\ntc->filename = "<<tc->file_name<<endl;
    cout<<"========================DEBUG=================================\n"<<endl;*/ 

    return tc;
}
/* download from server */
int tftp_recv(tftp_c *tc) {
    tftp_rrq *snd = (tftp_rrq *)malloc(TFTP_RRQ_LEN(tc->file_name,tc->mode));
    tftp_recv_pack recv;
    tftp_ack ack;
    int re;
    uint16_t blocknum = 1;

    /* request head */
    snd->opcode = htons(OPCODE_RRQ);
    sprintf(snd->req, "%s%d%s%d", tc->file_name, 0, tc->mode, 0);

    /* send request 2 server */
    re = sendto(tc->sockfd, snd, TFTP_RRQ_LEN(tc->file_name,tc->mode), 0, ((sockaddr *)&tc->addr_server), tc->addr_len);
    if (re == -1) {
        cout<<"ERROR:fail to sent request to server"<<endl;
        fprintf(log_fp,"ERROR:fail to sent request to server\n");
    }
    /* ========================DEBUG================================= */
    cout<<"========================DEBUG================================="<<endl;
    cout<<"snd.opcode = "<<ntohs(snd->opcode)<<"\nreq.head = "<<snd->req<<endl;
    cout<<"re = "<<re<<endl;
    cout<<"========================DEBUG================================="<<endl;

    FILE *fp = fopen(tc->file_name, "w+");

    /* begin recv data */
    while (1) {
        cout<<"11111"<<endl;
        re = recvfrom(tc->sockfd, &recv, sizeof(tftp_recv_pack), 0, ((sockaddr *)&tc->addr_server), &tc->addr_len);
        cout<<"22222"<<endl;
        
        if (re == -1) {
            cout<<"ERROR:fail to recv from server"<<endl;
            fprintf(log_fp,"ERROR:fail to recv from server\n");
            break;
        }
        
        /* recv success & send ACK */
        //TODO:Throughput
        if (recv.opcode == htons(OPCODE_DATA) && recv.bnum_ecode == blocknum) {
            cout<<"DATA: BlockNum = "<<ntohs(recv.bnum_ecode)<<", DataSize = "<<(re - 4)<<endl;
            fprintf(log_fp,"DATA: BlockNum = %d, DataSize = %d\n", ntohs(recv.bnum_ecode), (re-4));
            /* Send ACK */
            ack.opcode = htons(OPCODE_ACK);
            ack.blocknum = blocknum;
            re = sendto(tc->sockfd, &ack, sizeof(tftp_ack), 0 , ((sockaddr *)&tc->addr_server), tc->addr_len);
            fprintf(fp,"%s",recv.data);
        }
        /* ========================DEBUG================================= */
        cout<<"========================DEBUG================================="<<endl;
        cout<<"re = "<<re<<endl;
        cout<<"recv.opcode = "<<ntohs(recv.opcode)<<endl;
        cout<<"recv.bnum_ecode = "<<ntohs(recv.bnum_ecode)<<endl;
        cout<<"recv.data = "<<recv.data<<endl;
        cout<<"========================DEBUG================================="<<endl;      


        /* the last block size < 512, end recv */
        if (re < DATA_SIZE+4) {
            cout<<"download finish"<<endl;
            fprintf(log_fp,"download finsish");
            break;
        }
        blocknum++;
    }
    fclose(fp);
    return 1;
}