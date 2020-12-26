/* TFTP Client */
#include "Client.h"
using namespace std;

int main(void) {
    char host_name[20];
    char file_name[50];
    char port4addr[20]="69";
    int type;
    tftp_c *tc = NULL;
//    printf("%d %d\n",sizeof(uint16_t),sizeof(ushort));
    log_fp = fopen("log.txt","w+");
//    fclose(log_fp);
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
    tc = tftp_connect(host_name, port4addr,(char *)MODE_NETASCII,type,file_name); 
    if (!tc) {
        now_time();
        cout<<"ERROR:fail to connect to server"<<endl;
        fprintf(log_fp,"ERROR:fail to connect to server\n");
        return -1;
    }

    /* Download or Upload */
    if (!type) { // 0 -> Download
        tftp_recv(tc);
    }
    else { // 1 -> Upload
        tftp_put(tc);
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
        now_time();
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
        now_time();
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
    int re,bp_re;
    uint16_t blocknum = 1;
    /* request head */
    snd->opcode = htons(OPCODE_RRQ);
    sprintf(snd->req, "%s%c%s%c", tc->file_name, 0, tc->mode, 0);

    /* send request 2 server */
    re = sendto(tc->sockfd, snd, TFTP_RRQ_LEN(tc->file_name,tc->mode), 0, ((sockaddr *)&tc->addr_server), tc->addr_len);
    if (re == -1) {
        now_time();
        cout<<"ERROR:fail to send request to server"<<endl;
        fprintf(log_fp,"ERROR:fail to send request to server\n");
    }
    /* ========================DEBUG================================= */
   /* cout<<"========================DEBUG================================="<<endl;
    cout<<"snd.opcode = "<<ntohs(snd->opcode)<<"\nreq.head = "<<snd->req<<endl;
    cout<<"re = "<<re<<endl;
    cout<<"========================DEBUG================================="<<endl;*/
    now_time();
    cout<<"Download Start"<<endl;
    fprintf(log_fp,"Download Start");
    FILE *fp = fopen(tc->file_name, "w+");
 //   fprintf(fp, "%s", "test");

    /* begin recv data */
    start_c = clock();
    size_all = 0;
    while (1) {
        memset(recv.data, 0, sizeof(recv.data));
        re = recvfrom(tc->sockfd, &recv, sizeof(tftp_recv_pack), 0, ((sockaddr *)&tc->addr_server), &tc->addr_len);
        
        if (re == -1) {
            now_time();
            cout<<"ERROR:fail to recv from server"<<endl;
            fprintf(log_fp,"ERROR:fail to recv from server\n");
            break;
        }
        
        /* recv success & send ACK */
        //TODO:Throughput
        if (recv.opcode == htons(OPCODE_DATA) && recv.bnum_ecode == htons(blocknum)) {
        //    now_time();
        //    cout<<"DATA: BlockNum = "<<ntohs(recv.bnum_ecode)<<", DataSize = "<<(re - 4)<<endl;
        //    fprintf(log_fp,"DATA: BlockNum = %d, DataSize = %d\n", ntohs(recv.bnum_ecode), (re-4));
            fprintf(fp,"%s",recv.data); 

            /* record re */
            size_all += re;
            /* Send ACK */
            ack.opcode = htons(OPCODE_ACK);
            ack.blocknum = recv.bnum_ecode;
            sendto(tc->sockfd, &ack, sizeof(tftp_ack), 0 , ((sockaddr *)&tc->addr_server), tc->addr_len);
        }
        /* ========================DEBUG================================= */
        /*cout<<"========================DEBUG================================="<<endl;
        cout<<"re = "<<re<<endl;
        cout<<"recv.opcode = "<<ntohs(recv.opcode)<<endl;
        cout<<"recv.bnum_ecode = "<<ntohs(recv.bnum_ecode)<<endl;
        cout<<"blocknum = "<<blocknum<<endl;
        cout<<"========================DEBUG=z================================"<<endl;*/


        /* the last block size < 512, end recv */
        if (re < DATA_SIZE+4) {
            /* end of recv */
            end_c = clock();
            time_all = (double)(end_c - start_c) / CLOCKS_PER_SEC;
            now_time();
            cout<<"Download Finish"<<endl;
            fprintf(log_fp,"Download Finsish");

            /* statistic this download */
            fprintf(log_fp,"\n======================================\n");
        //    fprintf(log_fp,"file size = %lf kB\n", (size_all)/1024);
        //    fprintf(log_fp,"download time = %lf s\n", time_all);
            fprintf(log_fp,"this download average throughout  = %.2lf kB/s", size_all/(1024 * time_all));
            fprintf(log_fp,"\n======================================\n");

            break;
        }
        blocknum++;
    }
    fclose(fp);
    return 1;
}

int tftp_put(tftp_c *tc) {
    tftp_wrq *snd = (tftp_wrq *)malloc(TFTP_WRQ_LEN(tc->file_name,tc->mode));
    tftp_recv_pack recv;
    tftp_data snd_data;
    int re = 0;
    /* WRQ msg */
    snd->opcode = htons(OPCODE_WRQ);
    sprintf(snd->req, "%s%c%s%c", tc->file_name, 0, tc->mode, 0);
    /* send WRQ 2 server */
    re = sendto(tc->sockfd, snd, TFTP_WRQ_LEN(tc->file_name, tc->mode), 0, ((sockaddr *)&tc->addr_server), tc->addr_len);
    if (re == -1) {
        now_time();
        cout<<"ERROR:fail to send request to server"<<endl;
        fprintf(log_fp,"ERROR:fail to send request to server");
    }
    re = recvfrom(tc->sockfd, &recv, sizeof(tftp_recv_pack), 0, ((sockaddr *)&tc->addr_server), &tc->addr_len);
    if (re == -1) {
            now_time();
            cout<<"ERROR:fail to recv from server"<<endl;
            fprintf(log_fp,"ERROR:fail to recv from server\n");
            return 0;

    }/* recv blocknum=0 => start transfer */
  /*  cout<<"========================DEBUG================================="<<endl;
                cout<<"re = "<<re<<endl;
                cout<<"recv.opcode = "<<ntohs(recv.opcode)<<endl;
                cout<<"recv.bnum_ecode = "<<ntohs(recv.bnum_ecode)<<endl;
         //       cout<<"blocknum = "<<blocknum<<endl;
                cout<<"========================DEBUG=z================================"<<endl;*/
    if (recv.opcode == htons(OPCODE_ACK) && recv.bnum_ecode == htons(0)) {
        now_time();
        printf("Upload Start\n");
        fprintf(log_fp, "Upload Start\n");
        FILE *fp = fopen(tc->file_name, "r");
        size_all = 0;
        start_c = clock();
    //    printf("%s\n",tc->file_name);
        if (fp == NULL) {
            now_time();
            cout<<"ERROR: non-exist file"<<endl;
            fprintf(log_fp, "ERROR: non-exist file\n");
            return 0;
        }
        int blocknum = 1;
        int size_t;
        snd_data.opcode = htons(OPCODE_DATA);
        
        while (1) {
            /* init data packet */
            memset(snd_data.data,0 ,sizeof(snd_data.data));
            snd_data.blocknum = htons(blocknum);
            size_t = fread(snd_data.data, 1, DATA_SIZE, fp);
        //    printf("%s\n", snd_data.data);  
            /* send data 2 server */
            re = sendto(tc->sockfd, &snd_data, size_t+4, 0, ((sockaddr *)&tc->addr_server), tc->addr_len);
            if (re == -1) {
                now_time();
                cout<<"ERROR: fail to send data to server"<<endl;
                fprintf(log_fp,"ERROR: fail to send data to server\n");
                return 0;
            }
            size_all += re;
            /* recv from server (ACK|ERROR) */
            re = recvfrom(tc->sockfd, &recv, sizeof(tftp_recv_pack), 0, ((sockaddr *)&tc->addr_server), &tc->addr_len);
            if (re == -1) {
                now_time();
                cout<<"ERROR: fail to recv ACK from server"<<endl;
                fprintf(log_fp, "ERROR: fail to recv ACK from server\n");
                return 0;
            }
            /* ack yes */
            if (recv.opcode == htons(OPCODE_ACK) && recv.bnum_ecode == htons(blocknum)) {
                size_all += re;
                blocknum++;
            }
                /* ========================DEBUG================================= */
            /* cout<<"========================DEBUG================================="<<endl;
            cout<<"re = "<<re<<endl;
            cout<<"recv.opcode = "<<ntohs(recv.opcode)<<endl;
            cout<<"recv.bnum_ecode = "<<ntohs(recv.bnum_ecode)<<endl;
            cout<<"blocknum = "<<blocknum<<endl;
            cout<<"========================DEBUG================================"<<endl; */
            if (size_t != DATA_SIZE) {
                /* end of recv */
                end_c = clock();
                time_all = (double)(end_c - start_c) / CLOCKS_PER_SEC;

                now_time();
                printf("Upload Finish\n");
                fprintf(log_fp, "Upload Finish\n");

                /* statistic this upload */
                fprintf(log_fp,"\n======================================\n");
            //    fprintf(log_fp,"file size = %lf kB\n", (size_all)/1024);
            //    fprintf(log_fp,"upload time = %lf s\n", time_all);
                fprintf(log_fp,"this upload average throughout  = %.2lf kB/s", size_all/(1024 * time_all));
                fprintf(log_fp,"\n======================================\n");   
                break;
            }
        }
        fclose(fp);
    }
    return 1;
}

void now_time(void) {
    n_time = time(NULL);
    l_time = localtime(&n_time);
    printf("\n%s    ", asctime(l_time));
    fprintf(log_fp, "\n%s    ", asctime(l_time));
    return;
}