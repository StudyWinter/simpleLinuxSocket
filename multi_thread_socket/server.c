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
		
		pthread_create(&tid, NULL, do_work, (void*)(&ts[i]));	 // 创建线程
		pthread_detach(tid);                                     // 设置线程分离，防止僵尸线程产生

		i++;
	}

	return 0;
}

