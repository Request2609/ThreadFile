#pragma once
#include <iostream>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<unistd.h>
#include<vector>
#include<thread>
#include<string.h>
#define LEN  4096
#define PORT 2345
#define BACKLOG 50
using namespace std ;

struct proctol{

    int fd ;
    //服务器端套接字
    int sockfd ;
    int threadNum ;
    long cur ;
    //文件指针向右移动的长度
    long moved ;
    //文件长度
    long size ;
    //缓冲区大小
    char fileName[1024] ;
    char buf[LEN] ;
} ;

int SendFile(const char*fileName, int sockFd) ;
void sender(proctol& args) ;
