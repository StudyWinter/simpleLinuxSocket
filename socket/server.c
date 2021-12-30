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
	server_addr.sin_port = htons(SERVER_PORT); 	     // 端口号
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

