# linux网络编程

简单的linux网络编程，包括一对一、一对多（多进程版multi_process_socket、多线程版multi_thread_socket）



## 1 三次握手四次挥手

### 1.1 三次握手

![image-20211226180314264](linux%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B.assets/image-20211226180314264-1640852250647.png)

**在这个例子中，首先客户端主动发起连接、发送请求，然后服务器端响应请求，然后客户端主动关闭连接。**两条竖

线表示通讯的两端，从上到下表示时间的先后顺序，注意，数据从一端传到网络的另一端也需要时间，所以图中的

箭头都是斜的。双方发送的段按时间顺序编号为1-10，各段中的主要信息在箭头上标出，例如段2的箭头上标着

SYN, 8000(0), ACK1001, ，表示该段中的SYN位置1，32位序号是8000，该段不携带有效载荷（数据字节数为

0），ACK位置1，32位确认序号是1001，带有一个mss（Maximum Segment Size，最大报文长度）选项值为

1024。

建立连接（三次握手）的过程：

1 客户端发送一个带SYN标志的TCP报文到服务器。这是三次握手过程中的段1。发送完成后`客户端`进入`SYN_SEND`

状态。

> 客户端发出段1，SYN位表示连接请求。序号是1000，这个序号在网络通讯中用作临时的地址，每发一个数据
>
> 字节，这个序号要加1，这样在接收端可以根据序号排出数据包的正确顺序，也可以发现丢包的情况，另外，
>
> 规定SYN位和FIN位也要占一个序号，这次虽然没发数据，但是由于发了SYN位，因此下次再发送应该用序号
>
> 1001。mss表示最大段尺寸，如果一个段太大，封装成帧后超过了链路层的最大帧长度，就必须在IP层分
>
> 片，为了避免这种情况，客户端声明自己的最大段尺寸，建议服务器端发来的段不要超过这个长度。

2 服务器端回应客户端，是三次握手中的第2个报文段，同时带ACK标志和SYN标志。它表示对刚才客户端SYN的

回应；同时又发送SYN给客户端，询问客户端是否准备好进行数据通讯。发送完成后`服务端`进入`SYN_RCVD`状态。

> 服务器发出段2，也带有SYN位，同时置ACK位表示确认，确认序号是1001，表示“我接收到序号1000及其以
>
> 前所有的段，请你下次发送序号为1001的段”，也就是应答了客户端的连接请求，同时也给客户端发出一个连
>
> 接请求，同时声明最大尺寸为1024。

3 客户必须再次回应服务器端一个ACK报文，这是报文段3。`客户端`发送完毕后，进入`ESTABLISHED`状态，`服务端`

接收到这个包，也进入`ESTABLISHED`状态, TCP握手结束。

> 客户端发出段3，对服务器的连接请求进行应答，确认序号是8001。在这个过程中，客户端和服务器分别给对
>
> 方发了连接请求，也应答了对方的连接请求，其中服务器的请求和应答在一个段中发出，因此一共有三个段
>
> 用于建立连接，称为“三方握手（three-way-handshake）”。在建立连接的同时，双方协商了一些信息，例如
>
> 双方发送序号的初始值、最大段尺寸等。

##### 为什么是三次握手?不是两次或者四次？

从假设的角度来分析吧，假如是两次握手，会发生什么情况呢? 服务端在发出应答消息后，它根本就不能确认客户

端是否接受到消息了，那么这样意味着只有客户端可以向服务端发送数据。

假如是四次握手呢？明明已经保证了一个稳定的传输流了，为什么还要浪费性能再去发一次消息，浪费了性能。

### 1.2 四次挥手

![image-20211226181136084](linux%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B.assets/image-20211226181136084-1640852277888.png)



由于TCP连接是全双工的，因此每个方向都必须单独进行关闭。这原则是当一方完成它的数据发送任务后就能发送

一个FIN来终止这个方向的连接。收到一个 FIN只意味着这一方向上没有数据流动，一个TCP连接在收到一个FIN后

仍能发送数据。首先进行关闭的一方将执行主动关闭，而另一方执行被动关闭。

1 客户端发出段7，FIN位表示关闭连接的请求。

2 服务器发出段8，应答客户端的关闭连接请求。

3 服务器发出段9，其中也包含FIN位，向客户端发送关闭连接请求。

4 客户端发出段10，应答服务器的关闭连接请求。

**建立连接的过程是三方握手，而关闭连接通常需要4个段，服务器的应答和关闭连接请求通常不合并在一个段中，**

**因为有连接半关闭的情况，这种情况下客户端关闭连接之后就不能再发送数据给服务器了，但是服务器还可以发送**

**数据给客户端，直到服务器也关闭连接为止。**

### 1.3 滑动窗口

介绍UDP时我们描述了这样的问题：如果发送端发送的速度较快，接收端接收到数据后处理的速度较慢，而接收

缓冲区的大小是固定的，就会丢失数据。TCP协议通过“滑动窗口（Sliding Window）”机制解决这一问题。看下图

的通讯过程：

![image-20211226182950344](linux%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B.assets/image-20211226182950344-1640852285450.png)

（1）发送端发起连接，声明最大段尺寸是1460，初始序号是0，窗口大小是4K，表示“我的接收缓冲区还有4K字

节空闲，你发的数据不要超过4K”。接收端应答连接请求，声明最大段尺寸是1024，初始序号是8000，窗口大小

是6K。发送端应答，三方握手结束。

（2）发送端发出段4-9，每个段带1K的数据，发送端根据窗口大小知道接收端的缓冲区满了，因此停止发送数

据。

（3）接收端的应用程序提走2K数据，接收缓冲区又有了2K空闲，接收端发出段10，在应答已收到6K数据的同时

声明窗口大小为2K。 

（4）接收端的应用程序又提走2K数据，接收缓冲区有4K空闲，接收端发出段11，重新声明窗口大小为4K。

（5）发送端发出段12-13，每个段带2K数据，段13同时还包含FIN位。

（6）接收端应答接收到的2K数据（6145-8192），再加上FIN位占一个序号8193，因此应答序号是8194，连接处

于半关闭状态，接收端同时声明窗口大小为2K。

（7）接收端的应用程序提走2K数据，接收端重新声明窗口大小为4K。

（8）接收端的应用程序提走剩下的2K数据，接收缓冲区全空，接收端重新声明窗口大小为6K。

（9）接收端的应用程序在提走全部数据后，决定关闭连接，发出段17包含FIN位，发送端应答，连接完全关闭。

​	上图在接收端用小方块表示1K数据，实心的小方块表示已接收到的数据，虚线框表示接收缓冲区，因此套在虚

线框中的空心小方块表示窗口大小，从图中可以看出，随着应用程序提走数据，虚线框是向右滑动的，因此称为滑

动窗口。

​	从这个例子还可以看出，发送端是一K一K地发送数据，而接收端的应用程序可以两K两K地提走数据，当然也有

可能一次提走3K或6K数据，或者一次只提走几个字节的数据。也就是说，应用程序所看到的数据是一个整体，或

说是一个流（stream），在底层通讯中这些数据可能被拆成很多数据包来发送，但是一个数据包有多少字节对应

用程序是不可见的，因此TCP协议是面向流的协议。而UDP是面向消息的协议，每个UDP段都是一条消息，应用程

序必须以消息为单位提取数据，不能一次提取任意字节的数据，这一点和TCP是很不同的。

## 2 CS模型的TCP通信

TCP通信流程分析:

**server:**

```
1 socket()  创建socket
2 bind() 绑定服务器地址结构
3 listen()  设置监听上限
4 accept()  阻塞监听客户端连接
5 read(fd)  读socket获取客户端数据
6 小--大写  toupper()
7 write(fd)
8 close();
```

**client:**

```
1 socket()  创建socket
2 connect(); 与服务器建立连接
3 write() 写数据到 socket
4 read() 读转换后的数据。
5 显示读取结果
6 close()
```

### 1.1 正常的CS模型

实现server端

```c
/*************************************************************************
 > File Name: server.c
 > Author: Winter
 > Created Time: 2021年12月15日 星期三 19时20分07秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/socket.h>
#include<ctype.h>
#include<arpa/inet.h>

#define SERVER_PORT 9527

int main(int argc, char* argv[])
{
        int lfd = 0, newfd = 0;
        int res = 0;
        char buff[BUFSIZ], client_IP[BUFSIZ];

        struct sockaddr_in server_addr, client_addr;
        socklen_t client_addr_len;
        // 初始化server_addr
        server_addr.sin_family = AF_INET;                    // 使用的协议是ipv4
        server_addr.sin_port = htons(SERVER_PORT);           // 端口号
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);     // ip地址

        // 1 创建socket
        lfd = socket(AF_INET, SOCK_STREAM, 0);
        if (lfd == -1)
        {
                perror("socket error\n");
                exit(1);
        }

        // 2 绑定服务器地址结构
        res = bind(lfd, (struct sockaddr*)(&server_addr), sizeof(server_addr));
        if (res == -1)
        {
                perror("bind error\n");
                exit(1);
        }

        // 3 设置监听上限
        res = listen(lfd, 128);
        if (res == -1)
        {
                perror("listen error\n");
                exit(1);
        }

        // 4 阻塞监听客户端连接
        client_addr_len = sizeof(client_addr);
        newfd = accept(lfd,(struct sockaddr*)(&client_addr), &client_addr_len);
        if (newfd == -1)
        {
                perror("accept error\n");
                exit(1);
        }
        printf("client ip = %s, port = %d\n",
                 inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
                ntohs(client_addr.sin_port)
                );

        while(1)
        {
                // 5 read
                res = read(newfd, buff, sizeof(buff));
                if (res == -1)
                {
                        perror("read error\n");
                        exit(1);
                }
                // 输出一下
                write(STDOUT_FILENO, buff, res);

                // 6 小写转大写
                for (int i = 0; i < res; i++)
                {
                        buff[i] = toupper(buff[i]);
                }

                // 7 write
                int ret = write(newfd, buff, res);
                if (ret == -1)
                {
                        perror("write error\n");
                        exit(1);
                }
        }
        close(lfd);
        close(newfd);

        return 0;
}
```

测试一下。运行server程序，新开终端。

```bash
nc 127.0.0.1 9527
```

![image-20211226154420081](linux%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B.assets/image-20211226154420081-1640852317288.png)

程序可以正常运行

**client端**

```c
/*************************************************************************
 > File Name: client.c
 > Author: Winter
 > Created Time: 2021年12月16日 星期四 20时03分16秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define SERVER_PORT 9527

void sys_err(const char* str)
{
        perror(str);
        exit(1);
}

int main(int argc, char* argv[])
{
        int fd = 0;
        int res = 0;
        int count = 10;
        char buff[BUFSIZ];
        struct sockaddr_in server_addr;    // 服务器地址结构
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
//      inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
        // 1 创建socket
        fd = socket(AF_INET, SOCK_STREAM ,0);
        if (fd == 1)
        {
                sys_err("socket error\n");
        }

        // 2 connect
        res = connect(fd, (struct sockaddr*)(&server_addr), sizeof(server_addr));
        if (res == -1)
        {
                sys_err("connect error\n");
        }

        while(count--)
        {
                // 3 写数据到socket
                res = write(fd, "hello\n", sizeof("hello"));
                if (res == -1)
                {
                        sys_err("write error\n");
                }
                sleep(1);                             // 休眠
                // 4 读取处理的结果
                res = read(fd, buff, sizeof(buff));
                if (res == -1)
                {
                        sys_err("read error\n");
                }
                // 5 显示处理结果
                int ret = write(STDOUT_FILENO, buff, res);
                if (res == -1)
                {
                        sys_err("write error2\n");
                }
        }
        close(fd);

        return 0;
}
```

测试

![image-20211226154600042](linux%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B.assets/image-20211226154600042-1640852322967.png)

### 1.2 多进程的CS模型

这里把函数都封装了

wrap.h

```c
#ifndef __WRAP_H_
#define __WRAP_H_

void perr_exit(const char *s);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
int Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
int Close(int fd);
ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);
ssize_t my_read(int fd, char *ptr);
ssize_t Readline(int fd, void *vptr, size_t maxlen);

#endif
```

wrap.c

```c
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

// 输出错误
void perr_exit(const char *s)
{
        perror(s);
        exit(-1);
}

// 封装accept函数
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
        int n;

again:
        if ((n = accept(fd, sa, salenptr)) < 0) {
                if ((errno == ECONNABORTED) || (errno == EINTR))
                        goto again;
                else
                        perr_exit("accept error");
        }
        return n;
}

// 封装bind函数
int Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
    int n;

        if ((n = bind(fd, sa, salen)) < 0)
                perr_exit("bind error");

    return n;
}

// 封装connect函数
int Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
    int n;

        if ((n = connect(fd, sa, salen)) < 0)
                perr_exit("connect error");

    return n;
}

// 封装listen函数
int Listen(int fd, int backlog)
{
    int n;

        if ((n = listen(fd, backlog)) < 0)
                perr_exit("listen error");

    return n;
}

// 封装socket函数
int Socket(int family, int type, int protocol)
{
        int n;

        if ((n = socket(family, type, protocol)) < 0)
                perr_exit("socket error");

        return n;
}

// 读函数
ssize_t Read(int fd, void *ptr, size_t nbytes)
{
        ssize_t n;

again:
        if ( (n = read(fd, ptr, nbytes)) == -1) {
                if (errno == EINTR)
                        goto again;
                else
                        return -1;
        }
        return n;
}

// 写函数
ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
        ssize_t n;

again:
        if ( (n = write(fd, ptr, nbytes)) == -1) {
                if (errno == EINTR)
                        goto again;
                else
                        return -1;
        }
        return n;
}

// 关闭函数
int Close(int fd)
{
    int n;
        if ((n = close(fd)) == -1)
                perr_exit("close error");

    return n;
}

/*参三: 应该读取的字节数*/
ssize_t Readn(int fd, void *vptr, size_t n)
{
        size_t  nleft;              //usigned int 剩余未读取的字节数
        ssize_t nread;              //int 实际读到的字节数
        char   *ptr;

        ptr = vptr;
        nleft = n;

        while (nleft > 0) {
                if ((nread = read(fd, ptr, nleft)) < 0) {
                        if (errno == EINTR)
                                nread = 0;
                        else
                                return -1;
                } else if (nread == 0)
                        break;

                nleft -= nread;
                ptr += nread;
        }
        return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n)
{
        size_t nleft;
        ssize_t nwritten;
        const char *ptr;

        ptr = vptr;
        nleft = n;
        while (nleft > 0) {
                if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
                        if (nwritten < 0 && errno == EINTR)
                                nwritten = 0;
                        else
                                return -1;
                }

                nleft -= nwritten;
                ptr += nwritten;
        }
        return n;
}

static ssize_t my_read(int fd, char *ptr)
{
        static int read_cnt;
        static char *read_ptr;
        static char read_buf[100];

        if (read_cnt <= 0) {
again:
                if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
                        if (errno == EINTR)
                                goto again;
                        return -1;
                } else if (read_cnt == 0)
                        return 0;
                read_ptr = read_buf;
        }
        read_cnt--;
        *ptr = *read_ptr++;

        return 1;
}

ssize_t Readline(int fd, void *vptr, size_t maxlen)
{
        ssize_t n, rc;
        char    c, *ptr;

        ptr = vptr;
        for (n = 1; n < maxlen; n++) {
                if ( (rc = my_read(fd, &c)) == 1) {
                        *ptr++ = c;
                        if (c  == '\n')
                                break;
                } else if (rc == 0) {
                        *ptr = 0;
                        return n - 1;
                } else
                        return -1;
        }
        *ptr  = 0;

        return n;
}
```

实现流程

```
	1. Socket();		创建 监听套接字 lfd
	2. Bind()	绑定地址结构 Strcut scokaddr_in addr;
	3. Listen();	
	4. while (1) {
		cfd = Accpet();			接收客户端连接请求。
		pid = fork();
		if (pid == 0){			子进程 read(cfd) --- 小-》大 --- write(cfd)
			close(lfd)		关闭用于建立连接的套接字 lfd
			read()
			小--大
			write()
		} else if （pid > 0） {	
			close(cfd);		关闭用于与客户端通信的套接字 cfd	
			contiue;
		}
	  }

	5. 子进程：
		close(lfd)
		read()
		小--大
		write()	
	   父进程：
		close(cfd);
		注册信号捕捉函数：	SIGCHLD
		在回调函数中， 完成子进程回收
			while （waitpid()）;
```

实现server端

```c
/*************************************************************************
  > File Name: server.c
  > Author: Winter
  > Created Time: 2021年12月27日 星期一 10时44分09秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/socket.h>
#include<ctype.h>
#include<sys/wait.h>
#include<errno.h>
#include"wrap.h"

#define SERVER_PORT 9527

void sys_error(const char* str)
{
        perror(str);
        exit(1);
}
// 父进程回收子进程
// 回顾系统编程第六章第8、9节
void catch_child(int signum)
{
        while (waitpid(0, NULL, WNOHANG) > 0);
        return ;
}


int main(int argc, char* argv[])
{
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = 0;

        memset(&server_addr, 0, sizeof(server_addr));     // 将地址清零
        server_addr.sin_family = AF_INET;                 // IPV4协议
        server_addr.sin_port = htons(SERVER_PORT);        // 端口号
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 本地可用ip地址

        int conncet_fd = 0, accept_fd = 0;;
        int res = 0;
        char buff[BUFSIZ], client_ip[BUFSIZ];
        pid_t pid;
        // 1 创建socket套接字
        conncet_fd = Socket(AF_INET, SOCK_STREAM, 0);
        // 2 绑定服务器地址结构
        res = Bind(conncet_fd, (struct sockaddr *)(&server_addr), sizeof(server_addr));
        // 3 设置监听上限
        Listen(conncet_fd, 128);
        printf("Accept client......\n");
        while (1)
        {
                // 4 服务器阻塞监听客户端
                client_len = sizeof(client_addr);
                accept_fd = Accept(conncet_fd, (struct sockaddr *)(&client_addr), &client_len);

                // 打印客户端ip和端口
                printf("client ip = %s, port = %d\n",
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                ntohs(client_addr.sin_port));

                // 5 创建子进程
                pid = fork();
                if (pid < 0)
                {
                        sys_error("fork error\n");
                }
                else if (pid == 0)
                {
                        // 6 子进程处理客户端的请求
                        close(conncet_fd);            // 关闭用于连接的套接字
                        break;
                }
                else
                {
                        // 7 父进程继续监听客户端，并且回收子进程
                        struct sigaction act;
                        act.sa_handler = catch_child;            // 信号捕捉函数
                        sigemptyset(&act.sa_mask);               // 设置捕捉函数执行期间屏蔽字 
                        act.sa_flags = 0;                        // 设置默认属性, 本信号自动屏蔽 
                        int ret = sigaction(SIGCHLD, &act, NULL);
                        if (ret != 0)
                        {
                                sys_error("sigaction error\n");
                        }

                        close(accept_fd);             // 关闭用于通信的套接字
                        continue;
                }

        }

        // 子进程实际逻辑处理部分
        if (pid == 0)
        {
                while (1)
                {
                        // 读取客户端的信息
                        res = Read(accept_fd, buff, sizeof(buff));
                        if (res == 0)
                        {
                                printf("the client %d has been closed......\n",accept_fd);
                                close(accept_fd);
                                exit(1);
                        }
                        // 输出
                        Write(STDOUT_FILENO, buff, res);
                        // 小写转大写
                        for (int i = 0; i < res; i++)
                        {
                                buff[i] = toupper(buff[i]);
                        }
                        // 写回到客户端
                        Write(accept_fd, buff, res);
                        Write(STDOUT_FILENO, buff, res);
                }
        }
        return 0;
}
```

实现client端

```c
/*************************************************************************
 > File Name: client.c
 > Author: Winter
 > Created Time: 2021年12月25日 星期六 20时54分01秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<arpa/inet.h>
#include"wrap.h"

#define MAXLEN 8192
#define SERVER_PORT 9527

int main(int argc, char* argv[])
{
        int lfd = 0;
        char buff[MAXLEN];
        int res = 0;
        struct sockaddr_in server_addr;                            // 服务器地址结构
        memset(&server_addr, 0, sizeof(server_addr));              // 将server_addr空间置为0
        server_addr.sin_family = AF_INET;                          // IPV4
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);    // ip地址
        server_addr.sin_port = htons(SERVER_PORT);                 // 端口

        // 1 创建socket
        lfd = Socket(AF_INET, SOCK_STREAM, 0);

        // 2 与服务器建立连接
        Connect(lfd, (struct sockaddr *)(&server_addr), sizeof(server_addr));

        while (fgets(buff, MAXLEN, stdin) != NULL)
        {
                // 3 讲数据写到server端
                Write(lfd, buff, strlen(buff));
                // 4 读取转换后的数据
                res = Read(lfd, buff, MAXLEN);

                if (res == 0)
                {
                        printf("the client has been closed\n");
                        break;
                }
                else
                {
                        // 5 显示结果
                        Write(STDOUT_FILENO, buff, res);
                }
        }
        // 6 close
        Close(lfd);
        return 0;
}
```

测试

![image-20211226172717054](linux%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B.assets/image-20211226172717054-1640852331457.png)

### 1.3 多线程的CS模型

实现流程

```
	1. Socket();		创建 监听套接字 lfd
	2. Bind()		绑定地址结构 Strcut scokaddr_in addr;
	3. Listen();		
	4. while (1) {		
		cfd = Accept(lfd, );
		pthread_create(&tid, NULL, tfn, (void *)cfd);
		pthread_detach(tid);  				// pthead_join(tid, void **);  新线程---专用于回收子线程。
	  }

	5. 子线程：
		void *tfn(void *arg) 
		{
			// close(lfd)			不能关闭。 主线程要使用lfd
			read(cfd)
			小--大
			write(cfd)
			pthread_exit（(void *)10）;	
		}
```

server端实现

```c
/*************************************************************************
 > File Name: server.c
 > Author: Winter
 > Created Time: 2021年12月26日 星期日 16时09分18秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<ctype.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include"wrap.h"


#define MAXLEN 8192
#define SERVER_POER 9527

// 定义一个结构体，将地址和connect_fd绑定
struct s_info
{
        struct sockaddr_in client_addr;
        int connect_fd;
};


// 子线程
// 参考系统编程第七章第3节
void* do_work(void* arg)
{
        int n, i;
        struct s_info *ts = (struct s_info*)arg;
        char buff[MAXLEN];
        char str[INET_ADDRSTRLEN];           // #define INET_ADDRSTRLEN 16 [+d可以查看

        while (1)
        {
                // 5 read客户端
                n = Read(ts->connect_fd, buff, MAXLEN);
                if (n == 0)
                {
                        printf("the client %d closed...\n", ts->connect_fd);
                        break;
                }

                // 打印客户端信息
                printf("received from %s at port %d\n",
                        inet_ntop(AF_INET, &(*ts).client_addr, str, sizeof(str)),
                        ntohs((*ts).client_addr.sin_port));

                // 6 小写转大写
                for (int i = 0; i < n; i++)
                {
                        buff[i] = toupper(buff[i]);
                }

                // 7 将结果写到屏幕
                //Write(STDOUT_FILENO, buff, n);
                printf("after :%s\n", buff);

                // 8 将结果写回给客户端
                Write(ts->connect_fd, buff, n);
        }
        // 9 关闭
        Close(ts->connect_fd);
        return (void*)0;
}


int main(int argc, char* argv[])
{
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len;
        bzero(&server_addr, sizeof(server_addr));             // 地址清零
        server_addr.sin_family = AF_INET;                     // IPV4协议
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);      // 指定本地任意IP
        server_addr.sin_port = htons(SERVER_POER);            // 指定端口号

        int listen_fd = 0, connect_fd = 0;
        struct s_info ts[256];
        pthread_t tid;
        int i = 0;

        // 1 创建socket
        listen_fd = Socket(AF_INET, SOCK_STREAM, 0);

        // 2 绑定地址结构
        Bind(listen_fd, (struct sockaddr *)(&server_addr), sizeof(server_addr));

        // 3 设置监听上限
        Listen(listen_fd, 128);

        printf("Accepting client connect...\n");

        while (1)
        {
                client_len = sizeof(client_addr);
                // 4 阻塞监听客户端连接
                connect_fd =  Accept(listen_fd, (struct sockaddr *)(&client_addr), &client_len);

                ts[i].client_addr = client_addr;
                ts[i].connect_fd = connect_fd;

                pthread_create(&tid, NULL, do_work, (void*)(&ts[i]));    // 创建线程
                pthread_detach(tid);                                     // 设置线程分离，防止僵尸线程产生

                i++;
        }

        return 0;
}
```

这里编译server.c的时候，要在后面加-lpthread。即

```bash
gcc server.c wrap.c -o server -lpthread
```

实现client端

```c
/*************************************************************************
 > File Name: client.c
 > Author: Winter
 > Created Time: 2021年12月25日 星期六 20时54分01秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<arpa/inet.h>
#include"wrap.h"

#define MAXLEN 8192
#define SERVER_PORT 9527


int main(int argc, char* argv[])
{
        int lfd = 0;
        char buff[MAXLEN];
        int res = 0;
        struct sockaddr_in server_addr;                            // 服务器地址结构
        memset(&server_addr, 0, sizeof(server_addr));              // 将server_addr空间置为0
        server_addr.sin_family = AF_INET;                          // IPV4
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);    // ip地址
        server_addr.sin_port = htons(SERVER_PORT);                 // 端口

        // 1 创建socket
        lfd = Socket(AF_INET, SOCK_STREAM, 0);

        // 2 与服务器建立连接
        Connect(lfd, (struct sockaddr *)(&server_addr), sizeof(server_addr));

        while (fgets(buff, MAXLEN, stdin) != NULL)
        {
                // 3 讲数据写到server端
                Write(lfd, buff, strlen(buff));
                // 4 读取转换后的数据
                res = Read(lfd, buff, MAXLEN);

                if (res == 0)
                {
                        printf("the client has been closed\n");
                        break;
                }
                else
                {
                        // 5 显示结果
//                      Write(STDOUT_FILENO, buff, res);
                        printf("transform: %s\n", buff);
                }
        }
        // 6 close
        Close(lfd);
        return 0;
}
```

测试

![image-20211226173315799](linux%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B.assets/image-20211226173315799-1640852339102.png)
