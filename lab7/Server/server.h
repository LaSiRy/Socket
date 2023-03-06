/*
 * Lishiyu
 * _INCLUDE_H_
 */


#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <signal.h>
#include <mutex>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <pthread.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "pthreadVC2.lib")


 //连接端口
#define CONNECT_PORT 2893 
 //连接客户端最大数目 - 32
#define MAX_CLIENT_NUM 32
#define Server_ID 0
#define BUF_LEN 1024

//使用thread库时，不要使用namespace std，因为std::bind()有冲突；


 //存储当前连接的客户端信息
typedef struct client_info {
	int ID;         //客户端ID
	std::string IP; //客户端IP地址
    int port;       //客户端连接端口号
    SOCKET client_socket;   //客户端套接字
}*Client_Info;

Client_Info Client_list[MAX_CLIENT_NUM];	//当前已连接的客户端
int hasClient[MAX_CLIENT_NUM] = {0};		//建立客户端到ID的映射
int Client_num = 0;							//连接客户端的数量
int isQuit = 0;								//是否退出主进程
pthread_mutex_t mutex;


int addClient(Client_Info);		//该函数用于添加一个客户端并返回客户端ID
void delClient(Client_Info);	//该函数用于删除一个客户端
int isInClientList(int);        //该函数用于判断客户端是否在Server的Client链表中
std::string getTime(void);		//该函数用于返回当前时间
std::string getHostName(void);	//该函数用于返回主机名称
std::string getClientList(void);	//该函数用于返回当前连接的客户端列表
std::string requestProcess(std::string, Client_Info); //响应客户端请求并生成响应数据包
void* ThreadRun(void*);								//定义子线程执行函数