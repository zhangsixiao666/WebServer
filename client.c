#define _GNU_SOURCE 1
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<poll.h>
#include<fcntl.h>
#define BUFFER_SIZE 64
int main(int argc,char* argv[]){
    if(argc<=2){
        printf("usage:%s ip_address port_number\n",basename(argv[0]));
        return 0;
    }//对于main函数的参数  argc表参数个数，argv[]存储每个参数,第一个为运行指令 如./client.o
    //拆分ip port 定义socket,connect
    const char* ip = argv[1];  //拆分ip
    int port = atoi(argv[2]);  //使用atoi将字符串转化为int类型
    struct sockaddr_in server_address;  //定义sockaddr_in类型结构体
    /*sockaddr_in结构体用来处理网络通信的地址
            struct sockaddr_in{
                sa_family_t sin_family;  地址族
                uint16_t  sin_port;
                struct in_addr sin_addr;
                char sin_zero[8];
            }
    */
    bzero(&server_address,sizeof server_address);//初始化 置0
    server_address.sin_family=AF_INET; //地址族
    inet_pton(AF_INET,ip,&server_address.sin_addr);//ip地址转换，点分十进制和整数之间转化
    server_address.sin_port=htons(port);//主机序转为网络序
    int sockfd=socket(PF_INET,SOCK_STREAM,0);//调用socket()创建socket，返回一个文件描述符fd
    assert(sockfd>=0);
    if(connect(sockfd,(struct sockaddr*)&server_address,sizeof server_address)<0){
        printf("connection failed\n");
        close(sockfd);
        return 1;
    }//connect()尝试连接服务器，即拿着新创建的socket去链接给定网络地址的socket，返回一个
    struct pollfd fds[2];
    fds[0].fd=0;
    fds[0].events=POLLIN;
    fds[0].revents=0;
    fds[1].fd=sockfd;
    fds[1].events=POLLIN | POLLRDHUP;
    fds[1].revents=0;
    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    int ret=pipe(pipefd);
    assert(ret!=-1);
    while(1){
        ret=poll(fds,2,-1);
        if(ret<0){
            printf("poll failure\n");
            break;
        }
        if(fds[1].revents & POLLRDHUP){
            printf("server close the connection\n");
            break;
        }
        else if(fds[1].revents & POLLIN){
            memset(read_buf,'\0',BUFFER_SIZE);
            recv(fds[1].fd,read_buf,BUFFER_SIZE-1,0);
            printf("%s\n",read_buf);
        }
        if(fds[0].revents & POLLIN){
            ret = splice(0,NULL,pipefd[1],NULL,32768,SPLICE_F_MORE | SPLICE_F_MOVE);
            ret = splice(pipefd[0],NULL,sockfd,NULL,32768,SPLICE_F_MORE | SPLICE_F_MOVE);
        }
        printf("test2\n");
        printf("%s\n",read_buf);
    }
    close(sockfd);
    return 0;

}
