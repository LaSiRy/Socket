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


 //���Ӷ˿�
#define CONNECT_PORT 2893 
 //���ӿͻ��������Ŀ - 32
#define MAX_CLIENT_NUM 32
#define Server_ID 0
#define BUF_LEN 1024

//ʹ��thread��ʱ����Ҫʹ��namespace std����Ϊstd::bind()�г�ͻ��


 //�洢��ǰ���ӵĿͻ�����Ϣ
typedef struct client_info {
	int ID;         //�ͻ���ID
	std::string IP; //�ͻ���IP��ַ
    int port;       //�ͻ������Ӷ˿ں�
    SOCKET client_socket;   //�ͻ����׽���
}*Client_Info;

Client_Info Client_list[MAX_CLIENT_NUM];	//��ǰ�����ӵĿͻ���
int hasClient[MAX_CLIENT_NUM] = {0};		//�����ͻ��˵�ID��ӳ��
int Client_num = 0;							//���ӿͻ��˵�����
int isQuit = 0;								//�Ƿ��˳�������
pthread_mutex_t mutex;


int addClient(Client_Info);		//�ú����������һ���ͻ��˲����ؿͻ���ID
void delClient(Client_Info);	//�ú�������ɾ��һ���ͻ���
int isInClientList(int);        //�ú��������жϿͻ����Ƿ���Server��Client������
std::string getTime(void);		//�ú������ڷ��ص�ǰʱ��
std::string getHostName(void);	//�ú������ڷ�����������
std::string getClientList(void);	//�ú������ڷ��ص�ǰ���ӵĿͻ����б�
std::string requestProcess(std::string, Client_Info); //��Ӧ�ͻ�������������Ӧ���ݰ�
void* ThreadRun(void*);								//�������߳�ִ�к���