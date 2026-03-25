#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>

void error_exit(const std::string& msg) {
    std::cerr << msg << " - " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
}

off_t get_file_size(int fd){
    struct stat st;
    //fstat()系统调用，通过文件描述符获取文件状态，&st接收文件信息
    if(fstat(fd,&st)==-1){
        return -1;
    }
    return st.st_size;
}

void seek_file(const char* file_path){
    int fd=open(file_path,O_RDONLY);
    if(fd==-1){
        error_exit("无法打开文件");
    }

    //获取文件大小
    std::cout<<"文件大小："<<get_file_size(fd)<<"字节"<<std::endl;

    //1.从文件开头定位SEEK_SET
    off_t pos=lseek(fd,0,SEEK_SET);
    std::cout<<"文件开头位置："<<pos<<std::endl;

    //读取第一个字符
    char ch;
    if((read(fd,&ch,1)>0)){
        std::cout<<"第一个字符："<<ch<<std::endl;
    }

    //2.从当前位置定位SEEK_CUR
    pos=lseek(fd,5,SEEK_CUR);
    std::cout<<"从当前位置后移5字节，位置："<<pos<<std::endl;

    //读取5个字符
    char buffer[16]; //创建了一个16字节的缓冲区，足够存放5个字符 + 结尾的 \0
    //从文件描述符 fd 读取最多5个字符到buffer，返回实际读取字符数
    ssize_t n=read(fd,buffer,5); 
    if(n>0){
        //在末尾添加字符串结束符
        buffer[n]='\0';
        std::cout<<"接下来5个字符："<<buffer<<std::endl;
    }

    //3.从文件末尾定位SEEK_END,读取前10字节
    pos=lseek(fd,-10,SEEK_END);
    std::cout<<"定位到文件末尾前10字节，位置："<<pos<<std::endl;

    n=read(fd,buffer,10);
    if(n>0){
        buffer[n]='\0';
        std::cout<<"最后10个字符："<<buffer<<std::endl;
    }

    //4.获取当前文件位置
    pos=lseek(fd,0,SEEK_CUR);
    std::cout<<"当前文件位置："<<pos<<std::endl;
}

int main(int argc,char* argv[]){
    if(argc<2){
        std::cout<<"用法："<<argv[0]<<" <文件名>"<<std::endl;
        return EXIT_FAILURE;
    }
    seek_file(argv[1]);

    return EXIT_FAILURE;
}