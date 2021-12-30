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
			act.sa_handler = catch_child;
			sigemptyset(&act.sa_mask);
			act.sa_flags = 0;
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

