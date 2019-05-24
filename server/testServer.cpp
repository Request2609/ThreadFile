#include"testServer.h"
#include"ReadWrite.h"

int main(int argc, char** argv)
{
    if(argc< 0){
        cout<< "use:./a.out filename" <<endl ;
        exit(1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0) ;
    if(fd < 0) {
        return 0 ;
    }
        
    struct sockaddr_in sock_addr ;
    sock_addr.sin_family = AF_INET ;
    sock_addr.sin_port = htons(PORT) ;
    sock_addr.sin_addr.s_addr = INADDR_ANY ;
    int use = 1 ;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &use, sizeof(use))< 0){
        cout << "socketoopt" <<endl ;  
        return 1 ;
    }   
    if(bind(fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr))< 0) {
        cout << "bind" <<endl ;
        return 1 ;
    }

    if(listen(fd, BACKLOG) < 0){
        cout<< "listen" <<endl ;
        return 1 ;
    }

    SendFile(argv[1], fd) ;
    return 0;
}

int SendFile(const char*fileName, int sockFd) {

    struct stat st_file ;
    if(lstat(fileName, &st_file)< 0){
        cout << "发送的文件不存在!"<<endl ;
    }
    //为线程分配任务
    long file_block ;
    proctol pp ;
    pp.size = st_file.st_size ;
    printf("文件的大小：%ld\n", pp.size) ;
    //获取文件的大小,20mb以上为大文件
    if(pp.size>=1024*1024*20) {
        file_block = 1024*1024*5  ;     
    }
    else {
        file_block = 1024*1024*2 ;            
    }
    printf("每个线程分的块大小：%ld\n", file_block) ;
    int threadNum = pp.size/file_block ;
    //获取最后一个包的长度
    int lastpacksize = (int)pp.size-threadNum*file_block ;
    //如果最后一个包的长度不问0的话,则多创建一个线程
    if(lastpacksize) {
        threadNum++ ;
    }
    int connfd = accept(sockFd, NULL, NULL) ;
    if(connfd < 0) {
        printf("接收连接accept出错!") ;
        exit(1) ;
    }
    proctol data ;
    data.size = file_block ;
    //第一个包是文件名称和线程的数量
    strcpy(data.fileName, fileName) ;
    data.sockfd = sockFd ;
    data.threadNum = threadNum ;
    int ret = writen(connfd, &data, sizeof(data)) ;
    if(ret < 0) {
        printf("发送消息失败!") ;
        exit(1) ;
    }
    close(connfd) ;
    //打开文件
    int fd = open(fileName, O_RDONLY) ;
    if(fd < 0) {
        cout << "open error" <<endl ;
        exit(1) ;
    }
    for(int i= 0; i< threadNum; i++) {
        connfd = accept(sockFd, NULL, NULL) ;
        if(connfd < 0) {
            exit(1) ;
        }
        //发送文件
        proctol data ;
        memset(data.buf, 0, sizeof(data.buf)) ;
        strcpy(data.fileName, fileName) ;
        //当线程数量不为0且最后一个包的长度不为0
        if(i == threadNum-1&&lastpacksize) {
            data.cur = i*file_block ; 
            data.size = lastpacksize ;
        }
        else {
            //计算偏移
            data.cur = i*file_block ;
            data.size = file_block ;
        }
            
        data.fd = fd ;
        data.sockfd = connfd ;
        thread t(sender, std::ref(data)) ;
        t.join() ;
    }
    close(sockFd) ;
    close(fd) ;
    return 1 ;
}   
//发送文
void sender(proctol& data) {
    
    long cur = data.cur ;
    int connfd = data.sockfd ;
    int read_count ;
    long size = data.size ;
    long read_size = 0 ;
    int filefd = data.fd ;
    printf("传送文件的长度%ld，名称：%s\n", data.size, data.fileName) ;
    while(size){
        //原子操作
        data.moved = sizeof(data.buf) ;
        if(sizeof(data.buf) > (size_t)size) {
            data.moved = size ;
        }
        read_count = pread(filefd, data.buf, data.moved, cur+read_size) ;
        if(read_count < 0&& errno == EINTR) {
            printf("被信号打断") ;
            continue ;
        } 
        else if(read_count == 0) {
            break ;
        }   

        else if(read_count < 0) {
            printf("pread 错误!") ;
            exit(1) ;
        }
        
        writen(connfd, &data, sizeof(data)) ;
        size-=read_count ;
        read_size += read_count ;
    }
    close(connfd) ;
}
