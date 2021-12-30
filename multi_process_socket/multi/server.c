/*************************************************************************
  > File Name: server.c
  > Author: Winter
  > Created Time: 2021年12月25日 星期六 10时14分51秒
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
	exit(-1);
}

// 回收子进程
void catch_child(int signum)
{
	while (waitpid(0, NULL, WNOHANG) > 0);
	return;
}


int main(int argc, char* argv[])
{
	struct sockaddr_in server_addr, sockaddr_client;
	socklen_t client_addr_len;
	memset(&server_addr, 0 ,sizeof(server_addr));             // 将地址清零
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


	// 1 创建socket
	int lfd = 0, newfd = 0;
	int res = 0;
	char buff[BUFSIZ];
	pid_t pid;
	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	// 2 绑定地址结构 bind
	Bind(lfd, (struct sockaddr*)(&server_addr), sizeof(server_addr));

	// 3 设置监听上限
	Listen(lfd, 128);
	client_addr_len  = sizeof(sockaddr_client);
	while (1)
	{
		// 4 接受客户端连接请求
		newfd = accept(lfd, (struct sockaddr *)(&sockaddr_client), &client_addr_len);

		// 5 fork创建子进程
		pid = fork();	
		if (pid < 0)          // 错误处理
		{
			sys_error("fork error\n");
		}
		else if (pid == 0)   // 子进程
		{
			close(lfd);  // 关闭用于建立连接的套接字
			break;
		}
		else                // 父进程
		{
			// 父进程要回收子进程，需要信号捕捉函数
			struct sigaction act;
			act.sa_handler = catch_child;
			sigemptyset(&act.sa_mask);
			act.sa_flags = 0;
			int ret = sigaction(SIGCHLD, &act, NULL);
			if (ret != 0)
			{
				sys_error("sigaction error\n");
			}
			

			close(newfd);  // 关闭与客户端建立连接的套接字
			continue;
		}

	}
	// 完成业务，即服务器的读写流程
	if (pid == 0)
	{
		while (1)
		{
			res = Read(newfd, buff, sizeof(buff));         // 读取客户端的信息
			if (res == 0)
			{
				close(newfd);
				exit(1);
			}

			for (int i = 0; i <res; i++)
			{
				buff[i] = toupper(buff[i]);            // 小写转大写
			}
		 	Write(newfd, buff, res);
			Write(STDOUT_FILENO, buff, res);
		}
	}

	return 0;
}

