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
//	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);
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

