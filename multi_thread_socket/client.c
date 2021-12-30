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
	server_addr.sin_family = AF_INET; 		           // IPV4
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
//			Write(STDOUT_FILENO, buff, res);
			printf("transform: %s\n", buff);
		}
	}
	// 6 close
	Close(lfd);
	return 0;
}

