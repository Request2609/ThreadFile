#pragma once
#include<iostream>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<thread>
#include<vector>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<fcntl.h>
#define LEN 4096
struct proctol{
    
    int fd ;
    int sockfd ;
    int threadNum ;
    long cur ;
    long  moved ;
    //文件长度
    long size ;
    //缓冲区大小
    char fileName[1024] ;
    char buf[LEN] ;
} ;

void receive(proctol& args) ;
