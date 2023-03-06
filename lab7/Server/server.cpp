/*
 * Lishiyu
 * _SERVER_CPP_
 */
//this part is used to construct server

#include "server.h"
#include "Msg.h"

//收到Ctrl_C
void signalHandler(int signum){
    isQuit = 1;
}

int addClient(Client_Info new_Client){
    while (Client_num >= MAX_CLIENT_NUM - 1);//等待一个客户端释放连接
    //可以建立连接
    pthread_mutex_lock(&mutex);  //lock
    Client_num++;
    int Client_ID = -1;
    for (int i = 1; i < MAX_CLIENT_NUM; i++)
        if (hasClient[i] == 0){ //该ID位置为空
            Client_ID = i;           //给新客户端分配该ID号
            break;
        }
    Client_list[Client_ID] = new_Client;
    hasClient[Client_ID] = 1;
    pthread_mutex_unlock(&mutex);   //unlock
    return Client_ID;               //返回ID号
}

void delClient(Client_Info Client) {
    if (Client_num <= 0)
        std::cout << "no client connect!" << std::endl;;
    pthread_mutex_lock(&mutex);  //lock
    if (isInClientList(Client->ID)) {
        Client_num--;
        hasClient[Client->ID] = 0;     //该ID位设置为空
    }
    else std::cout<< "Not in the client list!" << std::endl;;
    pthread_mutex_unlock(&mutex);   //unlock
}
   
int isInClientList(int id){
    for (int i = 1; i < MAX_CLIENT_NUM; i++)
        if (hasClient[i] && Client_list[i]->ID == id)
            return 1;
    return 0;
}

std::string getTime(void){
    time_t t = time(nullptr);
    char buf[128] = { 0 };
    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return std::string(buf);
}

std::string getHostName(void) {
    char c[255];
    int size = 255;
    gethostname(c, size);
    return std::string(c);
}

std::string getClientList(void){
    std::string str("");
    for (int i = 1; i < MAX_CLIENT_NUM; i++)
        if (hasClient[i] == 1){
            std::stringstream ss;
            ss << "ip = " << Client_list[i]->IP << ", port = " << Client_list[i]->port
               << ", clientID = " << Client_list[i]->ID << "\n";
            std::string tmp = ss.str();
            str = str + tmp;
        }
    return str;
}

std::string requestProcess(std::string request, Client_Info newClient){
    Message req(request);
    req.ParseMsg();             //解析接收的数据包
    Message reply;              //创建响应数据包
    reply.setCID(newClient->ID);    //设置响应数据包ID字段
    std::cout << "client " << reply.getCID() << ": ";
    switch (req.getType()){
        //请求连接响应
        case CONNECT:
            std::cout << "asking for connection" << std::endl;
            reply.setType(CONNECT);
            reply.setContent("successfully connected!");       //内容：确定断连
            break;
        //请求断连响应
        case DISCONNECT:
            std::cout << "asking for disconnection" << std::endl;
            std::cout << "The client's connection closing..." << std::endl;
            std::cout << "ip = " << newClient->IP << ":" << newClient->port << std::endl;
            delClient(newClient);           //从列表中删除
            reply.setContent("Disconnect...");       //内容：确定断连
            reply.setType(DISCONNECT);
            break;
        //请求时间响应
        case GET_TIME:
            std::cout << "asking for time" << std::endl;
            reply.setContent(getTime());  //设置内容为时间
            reply.setType(GET_TIME);       //设置Type字段
            break;
        //请求服务端机器名称响应
        case GET_NAME:
            std::cout << "asking for name" << std::endl;
            reply.setContent(getHostName());
            reply.setType(GET_NAME);
            break;
        //请求客户端列表响应
        case GET_CLIENT_LIST:
            std::cout << "asking for client list" << std::endl;
            reply.setContent(getClientList());
            reply.setType(GET_CLIENT_LIST);
            break;
        //请求发送消息响应
        case SEND_MSG: {
            std::cout << "asking for send msg" << std::endl;
            int dst_CID = req.getCID();
            std::cout << "send message ************" << std::endl;
            if (isInClientList(dst_CID) == 0) {  //目标客户端未连接Server
                reply.setCID(SERVER_ID);
                reply.setContent("The destination client doesn't exist!"); //发送错误信息
                reply.setType(SEND_MSG);
            }
            else { //目标客户端已连接Server
                int isSend;
                Client_Info dst_Client = Client_list[dst_CID]; //获取目标客户端信息
                Message inst;   //指示数据包
                inst.setCID(newClient->ID);                    //设置ID字段为发送消息的客户端
                inst.setContent(req.getContent());             //设置发送内容
                inst.setType(SEND_MSG);
                inst.PackageMsg();                             //打包
                //将指示数据包发送给对应客户端
                isSend = send(dst_Client->client_socket, inst.getMsg().c_str(), inst.getMsg().size(), 0); //flags设置为0
                if (isSend <= 0) {    //发送错误
                    reply.setContent("Send failed!");
                    printf("send failed with error: %d\n", WSAGetLastError());
                    delClient(dst_Client);
                }
                else {
                    reply.setContent("Send successfully");
                }
                reply.setCID(SERVER_ID);
                reply.setType(SEND_MSG);
            }
            break;
        }
        default://收到错误的type值
            reply.setContent("Wrong Type!"); //发送错误信息
            reply.setType(ERROR_TYPE);
            break;
    }
    reply.PackageMsg();
    std::cout << reply.getContent() << std::endl; //显示响应信息
    if (reply.getType() != DISCONNECT) {
        int isSend = send(newClient->client_socket, reply.getMsg().c_str(), reply.getMsg().size(), 0);
        if (isSend == SOCKET_ERROR)
            std::cout << "Wrong to reply!" << std::endl; //报错
    }
    return reply.getMsg();
}


void* ThreadRun(void* arg){ //传入的参数是待建立连接的Client_Info
    pthread_detach(pthread_self()); //不阻塞主线程
    char head[HEAD_LEN];          //设置接收缓冲区------先接收头&长度
    Client_Info newClient = (Client_Info)arg;   //接收传入的参数
    int iResult;
    while (true){
        //判断主进程是否退出
        if (isQuit) {
            std::cout << "quit..." << std::endl;
            break;
        }
        std::string reply;                          //响应
        //接收请求信息
        iResult = recv(newClient->client_socket, head, HEAD_LEN, 0);//获得Msg长度
        if (iResult > 0){
            Message tmp;
            int msg_len = tmp.ParseMsgLength(head +1); //Msg长度
            char recvbuf[BUF_LEN];          //创建缓冲区
            //获得msg主体部分
            recv(newClient->client_socket, recvbuf, msg_len, 0);
            std::string request;
            request.assign(head, HEAD_LEN);
            //整体信息：head + msg主体部分
            request.append(recvbuf, msg_len);
            //处理请求信息
            reply = requestProcess(request, newClient);
            Message msg(reply);
            //解析响应数据包
            msg.ParseMsg();         
            if (msg.getType() == DISCONNECT) break;//若响应关闭，退出
        }
        else {
            //接收时网络中断
            std::cout << "The client's connection closing..." << std::endl;
            std::cout << "ip:port = " << newClient->IP << ":" << newClient->port << std::endl;
            delClient(newClient);
            break;
        }
    }
    //关闭连接
    iResult = shutdown(newClient->client_socket, SD_SEND);
    if (iResult == SOCKET_ERROR){
        printf("shutdown failed with error: %d\n", WSAGetLastError());
    }
    closesocket(newClient->client_socket);  //释放资源
    //退出线程
    pthread_exit(NULL);
    return 0;
}


//主线程
int main(void){
    WSADATA wsaData;
    //服务端SOCKET句柄
    SOCKET ListenSocket = INVALID_SOCKET;
    //服务端地址信息
    sockaddr_in * server_info = new sockaddr_in;
    //注册信号函数
    signal(SIGINT, signalHandler);
    int iResult;
    //(1) Initialize mutex & Winsock
    pthread_mutex_init(&mutex, NULL);
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    //Winsock启动错误
    if (iResult != 0){
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    //(2) Resolve the server address and port
    server_info->sin_family = AF_INET;           //IPV4
    server_info->sin_port = htons(CONNECT_PORT); //端口
    server_info->sin_addr.s_addr = INADDR_ANY;   //绑定IP
    ZeroMemory(server_info->sin_zero, 8);  //0
    //(3) Create a SOCKET for connecting to server
    ListenSocket = socket(server_info->sin_family, SOCK_STREAM, 0);
    if (ListenSocket == INVALID_SOCKET){
        printf("socket failed with error: %ld\n", WSAGetLastError());
        delete server_info;
        WSACleanup();
        return 1;
    }
    //(4) Setup the TCP listening socket
    iResult = bind(ListenSocket, (sockaddr *)server_info, sizeof(*server_info));//调用bind绑定
    if (iResult == SOCKET_ERROR){
        printf("bind failed with error: %d\n", WSAGetLastError());
        delete server_info;
        //close 前加 shutdown
        shutdown(ListenSocket, SD_BOTH);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    //(5)listening
    iResult = listen(ListenSocket, SOMAXCONN); //backlog 的值设置为 SOMAXCONN，就由系统来决定请求队列长度
    if (iResult == SOCKET_ERROR){
        printf("[ERROR] listen failed with error: %d\n", WSAGetLastError());
        delete server_info;
        shutdown(ListenSocket, SD_BOTH);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    //server启动完成
    std::cout << "Start the Server..." << std::endl;
    //(6) Accept a client socket
    while (true){
        //客户端地址信息
        sockaddr_in* client_addr = new sockaddr_in;
        int client_addr_len = sizeof(*client_addr);
        //检测退出
        if (isQuit) {
            std::cout << "quit..." << std::endl;
            break;
        }
        //客户端句柄
        SOCKET ClientSocket;
        ClientSocket = accept(ListenSocket, (sockaddr*)client_addr, &client_addr_len);
        //accept为阻塞函数
        if (ClientSocket == INVALID_SOCKET){
            printf("[ERROR] accept failed with error: %d\n", WSAGetLastError());
            delete client_addr;
            continue;
        }
        //获得句柄
        std::cout << "[Info] A client request connection..." << std::endl;
        //创建客户端信息
        Client_Info newClient = new client_info;
        newClient->client_socket = ClientSocket;
        newClient->IP = inet_ntoa(client_addr->sin_addr); 
        newClient->port = ntohs(client_addr->sin_port);
        newClient->ID = addClient(newClient);
        delete client_addr;
        std::cout << "ip:port = " << newClient->IP << ":" << newClient->port << std::endl;
        std::cout << "the client ID allocated is: " << newClient->ID << std::endl;
        Message reply;
        reply.setCID(newClient->ID);    //设置响应数据包ID字段
        reply.setType(CONNECT);
        reply.setContent("connected...");
        std::string msg = reply.PackageMsg();
        send(newClient->client_socket, msg.c_str(), msg.length(), 0);
        pthread_t *thread = new pthread_t;
        pthread_create(thread, NULL, ThreadRun, newClient);//创建子线程处理客户端请求
        //检测退出指令（如用户按退出键或者收到退出信号），检测到后即通知并等待各子线程退出
    }
    delete server_info;
    // close socket
    shutdown(ListenSocket, SD_BOTH);
    closesocket(ListenSocket);
    //close WSA
    WSACleanup();
    //等待所有子线程退出
    pthread_exit(NULL);
    return 0;
}

