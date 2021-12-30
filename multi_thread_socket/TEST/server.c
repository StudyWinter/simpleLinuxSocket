/*************************************************************************
 > File Name: server.c
 > Author: Winter
 > Created Time: 2021年12月28日 星期二 10时23分47秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include"wrap.h"
#include<arpa/inet.h>
#include<ctype.h>
#include<signal.h>
#include<fcntl.h>

#define SERVER_PORT 9527

// 将客户端地址 和 connect_fd绑定
struct s_info
{
	struct sockaddr_in client_addr;
	int connect_fd;
}; 


void* do_work(void* arg)
{
	int n, i;
	struct s_info* ts = (struct s_info*)(arg);

	char buff[BUFSIZ];
	char str[INET_ADDRSTRLEN];            // define INET_ADDRSTRLEN 16 [+d查看

	while (1)
	{
		// read客户端
		n = Read(ts->connect_fd, buff, sizeof(buff));
		if (n == 0)
		{
			printf("the client %d has been closed.....\n", ts->connect_fd);
			break;
		}

		// 输出客户端信息
		printf("client ip = %s, port = %d\n",
			inet_ntop(AF_INET, &(*ts).client_addr, str, sizeof(str)),
			ntohs((*ts).client_addr.sin_port));

		// 小写转大写
		for (int i = 0; i < n; i++)
		{
			buff[i] = toupper(buff[i]);
		}

		printf("after:%s\n", buff);

		Write(ts->connect_fd, buff, n);

	}
	Close(ts->connect_fd);
	return (void*)0;

}

int main(int argc, char* argv[])
{
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len;
	bzero(&server_addr, sizeof(server_addr));                // 地址清零
	server_addr.sin_family = AF_INET;           		 // IPV4协议
	server_addr.sin_port = htons(SERVER_PORT);  		 // 端口号
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	 // 本地任意可用的ip地址
	
	int connect_fd = 0, accept_fd = 0;
	pthread_t tid;
	char client_ip[BUFSIZ];
	struct s_info ts[256];
	int i = 0;

	// 1 创建套接字
	connect_fd = Socket(AF_INET, SOCK_STREAM, 0);	
	// 2 绑定服务器地址结构
	Bind(connect_fd, (struct sockaddr *)(&server_addr), sizeof(server_addr));

	// 3 设置监听上限
	Listen(connect_fd, 128);

	printf("Accept client......\n");
	while (1)
	{
		client_len = sizeof(client_addr);
		// 4 服务器阻塞客户端
		accept_fd =  Accept(connect_fd, (struct sockaddr*)(&client_addr), &client_len);	

		ts[i].client_addr = client_addr;
		ts[i].connect_fd = accept_fd;


		
		// 5 创建子线程
		pthread_create(&tid, NULL, do_work, (void*)(&ts[i]));       // 创建线程
		pthread_detach(tid);                                        // 设置线程分离
		
		i++;
	}


	// 6 主线程用于继续监听客户端

	// 7 子线程处理逻辑

	// 8 关闭套接字
	return 0;
}

