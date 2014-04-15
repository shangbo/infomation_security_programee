#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/netfilter.h>
#include <libnfnetlink/libnfnetlink.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <linux/kernel.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>

#define MATCH 1//indicates port mach IP addr
#define NMATCH 0//indicates port not match IP addr
//user's,filter rule information
int enable_flag=1;//firewall's filter,1(on),2(off)
unsigned int controlled_protocol=0;//the type of protocol controlled:1-ICMP,6-TCP,17-UDP
unsigned short controlled_srcport=0;//source port,0-control all the source port of message,only for TCP&UDP
unsigned short controlled_dstport=0;//destination port,only for TCP&UDP
unsigned int controlled_saddr=0;//source addr
unsigned int controlled_daddr=0;//destination addr
//store the IP message information
struct iphdr * piphdr;//process IP's head addr
//Libnetfilter_queue function base
int fd;//file description ver
struct nfq_handle *h;
struct nfq_q_handle *qh;
struct nfnl_handle *nh;
//main

static int callback(struct nfq_q_handle * qh, struct nfgenmsg * nfmsg, struct nfq_data * nfa, void *data);

int main(int argc, char **argv)
{
    char buf[1600];//get message data buffer area from IP level, length > max IP_length(1512)
    int length;//save the length of the message data
    if(argc==1)
        enable_flag=0;//close firewall's function of message filtering
    else{
        getpara(argc,argv);//controlling configeration information-->overall ver
        printf("input info:p=%d, x=%d y=%d m=%d n=%d \n",controlled_protocol,controlled_saddr,controlled_daddr,controlled_srcport,controlled_dstport);//show controlling information    
    }
    h=nfq_open();//open Lbnetfilter_queue function base
    if (!h)
    {
        fprintf(stderr, "Error during nfq_open()\n");
        exit(1);
    }
    if(nfq_unbind_pf(h,AF_INET)<0){//unbind nf_queue
        fprintf(stderr,"already nfq_unbind_pf()\n");
        exit(1);
    }
    if(nfq_bind_pf(h,AF_INET)<0){//rebind nf_queue
        fprintf(stderr,"Error during nfq_bind_pf()\n");
        exit(1);
    }
    qh=nfq_create_queue(h,0,&callback,NULL);
    //select queue(queue_0),set callback()
    if(!qh){
        fprintf(stderr, "Error during nfq_create_queue()\n");
        exit(1);
    }
    if(nfq_set_mode(qh,NFQNL_COPY_PACKET,0xffff)<0){
        fprintf(stderr, "can't set packet_copy mode\n");
        exit(1);
    }
    //date structure transition
    // nh=nfq_nfnlh(h);
    // fd=nfnl_fd(nh);
    nh = nfq_nfnlh(h);
    fd = nfnl_fd(nh);
    while(1){
        printf("%s\n", "test" );
        length=recv(fd,buf,1600,0);//receive data package
        printf("%s\n", "test" );
        nfq_handle_packet(h,buf,length);//function-->call the recalling function of nvq_create_queue()-->send package
    }
    nfq_destroy_queue(qh);//close queue processing
    nqf_close(h);//close base
    exit(0);
}

//show opions of command line
void display_usage(char * commandname){
    printf("Usage 1:%s\n", commandname);//no reference,close message filter function
    printf("Usage 2:%s -x saddr -y daddr -m srcport -n dstport\n",commandname);
}

//opition analysis
int getpara(int argc, char *argv[]){
    int optret;//save option charactor
    unsigned short tmpport;//save temperary ver
    optret=getopt(argc,argv,"pxymnh");//get the first option
    while(optret!=-1){
        printf("frist in getpara:%s\n",argv[optind] );
        switch(optret){//the option type is protocol
            case 'p'://protocol type
                if(strncmp(argv[optind],"ping",4)==0)
                    controlled_protocol=1;//attempt to controll ping
                else
                    if (strncmp(argv[optind],"tcp",3)==0)
                        controlled_protocol=6;//attempt to controll TCP message
                    else
                        if(strncmp(argv[optind],"udp",3)==0)
                            controlled_protocol=17;//attempt to controll DUP message
                        else{//-p+-ping/-tcp/-udp,the protocol type which is not supported
                            printf("Unknown protocol!Please check and try again!\n");
                            exit(1);
                        }
                break;  
            case 'x'://source IP addr
                if(inet_aton(argv[optind],(struct in_addr *)&controlled_saddr)==0){
                    //IP addr"192.168.47.1"-->32bit int IP addr
                    printf("Invalid source ip address! Please check and try again!\n");
                    exit(1);
                }
                break;
            case 'y'://destination IP addr
                if (inet_aton(argv[optind],(struct in_addr *)&controlled_daddr)==0)
                {
                    printf("Invalid destination ip address! Please check and try again!\n");
                    exit(1);
                }
                break;
            case 'm'://source port
                tmpport=atoi(argv[optind]);//char type-->digital type
                if (tmpport==0)
                {
                    printf("Invalid source port! Please check and try again!\n");
                    exit(1);
                }
                controlled_srcport=htons(tmpport);//-->network byte sequence
                break;
            case 'n'://destination port
                tmpport=atoi(argv[optind]);
                if(tmpport==0){
                    printf("Invalid source port! Please check and try again!\n");
                    exit(1);
                }
                controlled_dstport=htons(tmpport);
                break;
            case 'h':
                display_usage(argv[0]);//show option format
                exit(1);
            }   
            optret=getopt(argc,argv,"pxymnh");//process next option
    }
}

//match the port
int port_check(unsigned short srcport, unsigned short dstport){
    //accordding to the source port and destination port==0 or not, 4 situations:
    if((controlled_srcport==0)&&(controlled_dstport==0)){//source port=0&&destination port=0-->control all the port
        return MATCH;
    }
    if((controlled_srcport!=0)&&(controlled_dstport==0)){//control all destination port
        if (controlled_srcport==srcport)//only compare source port
        {
            return MATCH;
        }
        else
            return NMATCH;
    }
    if((controlled_srcport==0)&&(controlled_dstport!=0)){//control all source port
        if(controlled_dstport==dstport)//only compare destination port
            return MATCH;
        else
             return NMATCH;
    }
    if((controlled_srcport!=0)&&(controlled_dstport!=0)){//compare both the source and destination port
        if((controlled_srcport==srcport)&&(controlled_dstport==dstport))
            return MATCH;
        else
            return NMATCH;
    }
    return NMATCH;//not normal situation
}
//match ip adddr
int ipaddr_check(unsigned int saddr, unsigned int daddr){
    //source ip addr=0&&destination ip addr=0,4 situations:
    if((controlled_saddr==0)&&(controlled_daddr==0)){//controll all source ip addr and destination ip addr
        return MATCH;
    }
    if((controlled_saddr!=0)&&(controlled_daddr==0)){//controll destination ip addr
        if(controlled_saddr==saddr)//only compare source ip addr
            return MATCH;
        else
            return NMATCH;
    }
    if((controlled_saddr==0)&&(controlled_daddr!=0)){//controll source ip addr
        if(controlled_daddr==daddr)//only compare destination ip addr
            return MATCH;
        else
            return NMATCH;
    }
    if((controlled_saddr!=0) && (controlled_daddr!=0)){//compare both the source and destination addr
        if((controlled_saddr==saddr)&&(controlled_daddr==daddr))
            return MATCH;
        else
            return NMATCH;
    }
    return NMATCH;//not normal situation
}
//ICMP message filter
int icmp_check(void){
    struct icmphdr *picmphdr;
    picmphdr = (struct icmphdr *)((char *)piphdr+(piphdr->ihl*4));//get ICMP message head
    if (picmphdr->type == 8)//user-->ping-->telnet server
    {
        if (ipaddr_check(piphdr->saddr,piphdr->daddr)==MATCH)
        {
            printf("An ICMP packet is denied!\n");
            return NF_DROP;
        }
    }
    if (picmphdr->type==0)
    {
        if (ipaddr_check(piphdr->daddr,piphdr->saddr)==MATCH)
        {
            printf("An ICMP packet is denied!\n");
            return NF_DROP;
        }
    }
    return NF_ACCEPT;
}

int tcp_check(void)
{
    struct tcphdr * ptcphdr;
    ptcphdr = (struct tcphdr *)((char *)piphdr + (piphdr-> ihl * 4));
    if((ipaddr_check(piphdr->saddr,piphdr->daddr)==MATCH) && (port_check(ptcphdr->source,ptcphdr->dest) == MATCH)){
        printf("A TCP packet is denied!\n");
        return NF_DROP;
    }
    else
        return NF_ACCEPT;
}

int udp_check(void){
    struct udphdr *pudphdr;
    pudphdr = (struct udphdr *)((char *)piphdr + (piphdr->ihl*4));
    if((ipaddr_check(piphdr->saddr,piphdr->daddr) == MATCH) && (port_check(pudphdr->source,pudphdr->dest) == MATCH)){
        printf("A UDP packet is denied!\n");
        return NF_DROP;
    }
    else return NF_ACCEPT;
}

static int callback(struct nfq_q_handle * qh, struct nfgenmsg * nfmsg, struct nfq_data * nfa, void *data){
    int id = 0;
    struct nfqnl_msg_packet_hdr * ph;
    unsigned char * pdata = NULL;
    int pdata_len;
    int dealmethod = NF_DROP;
    char srcstr[32],deststr[32];

    ph = nfq_get_msg_packet_hdr(nfa);
    if(ph == NULL)
        return 1;
    id = ntohl(ph->packet_id);
    if(enable_flag == 0)
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
    pdata_len = nfq_get_payload(nfa, (char**)&data);
    if(pdata != NULL)
        piphdr = (struct iphdr *) pdata;
    else
        return 1;
    inet_ntop(AF_INET, &(piphdr->saddr), srcstr, 32);
    inet_ntop(AF_INET, &(piphdr->daddr), deststr, 32);

    printf("get a packet: %s -> %s", srcstr, deststr);
    if(piphdr->protocol == controlled_protocol)
        if(piphdr->protocol == 1)
            dealmethod = icmp_check();
        else
            if (piphdr->protocol == 6)
                dealmethod = tcp_check();
            else 
                if (piphdr->protocol == 17)
                    dealmethod = udp_check();
                else{
                    printf("Unkonwn type's packet! \n");
                    dealmethod = NF_ACCEPT;
                }
    else 
        dealmethod = NF_ACCEPT;
    return nfq_set_verdict(qh, id, dealmethod, 0, NULL);
}