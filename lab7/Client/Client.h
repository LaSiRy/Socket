/*
 * Lishiyu
 * _CLIENT_H_
 */



#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#include <string>
#include <vector>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "pthreadVC2.lib")
using namespace std;

class Client {
private:
	bool isconnect = false;//�Ƿ����ӷ����
	string IP; //�ͻ���IP
	int port; //�ͻ��˶˿�
	int ID;//�ͻ���ID
	SOCKET client_socket;//�ͻ����׽���
	
public:
	void PrintOperation(Client) {	}
	int getID() { return ID; }
	int getPort() { return port; }
	string getIP() { return IP; }
	SOCKET getSocket() { return client_socket; }
	bool isConnect() { return isconnect; }
	void setConnect(bool t) { isconnect = t; }
	void setSocket(SOCKET s) { client_socket = s; }
	void setID(int ID) { this->ID = ID; }
};

Client client;