#include"testClient.h"
#include"ReadWrite.h"
struct sockaddr_in addr ;

int main(int argc, char** argv)
{

    if(argc != 3) {
        printf("usage:./a.out ip port\n") ;
        exit(1) ;
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0) ;
    if(sockfd < 0) {
        printf("创建套接字失败!");
        exit(1) ;
    }
    
    const char* ip = argv[1] ;
    int port = atoi(argv[2]) ;
    
    addr.sin_family = AF_INET ;
    addr.sin_port = htons(port) ;
    inet_pton(AF_INET, ip, &addr.sin_addr) ;
    
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("connect连接失败!") ;
        exit(1) ;
    }
    int threadNum = 0 ;
    proctol data ;
    int ret = readn(sockfd, &data, sizeof(data)) ;
    if(ret < 0) {
        printf("recv 接收消息失败!") ;
        exit(1) ;
    }
    threadNum = data.threadNum ;    
    printf("创建了%d个线程！\n", data.threadNum) ;
    if(threadNum > 20) {
        printf("线程数量太多!\n") ;
        exit(1) ;
    }

    printf("接收到文件名称：%s\n", data.fileName) ;
    int fd = open(data.fileName, O_CREAT|O_WRONLY, 0777) ;
    if(fd < 0) {
        printf("open创建文件出错!") ;   
        exit(1) ;
    }
    //创建线程
    for(int i = 0; i< threadNum; i++) {
        proctol data ;
        memset(&data, 0, sizeof(data)) ;
        data.fd = fd ;
        std::thread t(receive ,std::ref(data)) ;
        t.join() ;
    }

    close(fd) ;
    close(sockfd) ;
    printf("接收完成!") ;
    return 0;
}

void receive(proctol& data) {
    
    int sockfd = socket(AF_INET,  SOCK_STREAM, 0) ;
    if(sockfd < 0) {
        printf("创建套接字失败!") ;
        exit(1) ;
    }

    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("连接服务器失败!") ;
        exit(1) ;
    }
    //接收到数据
    proctol datas ;
    int ret ;
    if((ret = readn(sockfd, &datas, sizeof(datas))) < 0) {
        printf("接收数据错误！");
        exit(1) ;
    }
    long size = datas.size ;
    long cur = datas.cur ;
    if(pwrite(data.fd, datas.buf, datas.moved, cur)<0) {
        printf("写文件出错！") ;
        exit(1) ;
    }
    printf("文件指针移动的长度：%ld\n", datas.moved) ;
    size -= datas.moved;
    cur += datas.moved ;
    //接收消息
    printf("当前需要读取%ld字节\n", size) ;
    while(size) {
        ret = readn(sockfd, &datas, sizeof(datas)) ;
        if(ret <  0 && errno == EINTR) {
            printf("读取文件失败！\n") ;
            continue ;
        }
        else if (ret == 0) {
            break ;
        }
        else if(ret < 0) {
            printf("读取文件出错！") ;
            exit(1) ;
        }
        //将数据写入到文件中
        if(pwrite(data.fd, datas.buf, datas.moved, cur) < 0) {
            printf("写文件出错！")  ;  
            exit(1) ;
        }
        cur += datas.moved ;
        size -= datas.moved ;
    }
    close(sockfd) ;
}
