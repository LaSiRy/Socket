/*
 * Lishiyu
 * _CLIENT_CPP_
 */


 //this part is used to construct client


#include "client.h"
#include "Msg.h"

bool waiting = false;

//设置等待
void setWaitState() {
    waiting = true;
}
//设置不等待
void releWaitState() {
    waiting = false;
}

void PrintOperation() {
    cout << "(1) Connect " << endl;
    if (client.isConnect()){
        cout << "(2) Disconnect " << endl;
        cout << "(3) Request server time" << endl;
        cout << "(4) Request server name" << endl;
        cout << "(5) Request client list" << endl;
        cout << "(6) Send message" << endl;
    }
    cout << "(7) Exit " << endl;
    cout << "Enter your choice: ";
}

//获取用户输入的操作
int GetOperation() {
    int op;
    string s_op;
    PrintOperation();
    cin >> s_op;
    while (true) {
        op = stoi(s_op);
        if (!(op >= CONNECT && op <= EXIT)) {
            cout << "[ERROR] Invalid operation!" << endl;
            PrintOperation();
            cin >> s_op;
        }
        else if (op > CONNECT && op < EXIT && client.isConnect() == false) {
            cout << "[ERROR] Invalid operation!" << endl;
            PrintOperation();
            cin >> s_op;
        }
        else break;
    }
    return op;
}

//子线程用于接受服务端数据
void* ThreadRun(void* arg) {//传入的参数是client
    Client* client = (Client*)arg;
    if (client->isConnect() == false) { //未连接
        return 0;
    }
    while (true) {
        char recvBuf[1024] = "\0";
        int ret;
        char head[3];
        int msg_len;
        Message reply;
        ret = recv(client->getSocket(), head, HEAD_LEN, 0);//获得Msg头+len
        if (ret == SOCKET_ERROR || ret == 0) {
            if (client->isConnect() == false) { //已经为false，说明已经断开连接了
                return 0;
            }
            client->setConnect(false);
            cout<<"\n[ERROR] No connection detected!\n";
            cout << "Enter your choice: ";
            shutdown(client->getSocket(), SD_BOTH);
            closesocket(client->getSocket());
            break;
        }
        else {
            reply.setHead(head[0]);
            msg_len = reply.ParseMsgLength(head + 1); //Msg长度
            //获得msg主体部分
            ret = recv(client->getSocket(), recvBuf, msg_len, 0);
        }
        string mstr;
        mstr.assign(head, HEAD_LEN);
        mstr.append(recvBuf, msg_len);
        reply.setMsg(mstr);
        reply.ParseMsg();
        //解析数据包
        string tString;
        int type = reply.getType();
        if (type < CONNECT || type>SEND_MSG) {
            cout << "[ERROR] Recieve incorrect reply!" << endl;
            continue;
        }
        if (type == CONNECT) {
            cout << "[INFO] Successfully connected!" << endl;            
            client->setID(reply.getCID());
            cout << "ID of this client is " << client->getID() <<endl;
            releWaitState();//代表已经接收到消息了
            continue;
        }
        else if (type == GET_TIME) {
            //获取时间
            tString = reply.getContent();
            cout << "[INFO] Get Time from server: " << endl;
            cout << "Current Server time is : " << tString << endl;
            releWaitState();//代表已经接收到消息了
            continue;
        }
        else if (type == GET_NAME) {
            //处理获取服务端名称
            tString = reply.getContent();
            cout << "[INFO] Get server name: " << endl;
            cout << "Current Server name is: " << tString << endl;
            releWaitState();
            continue;
        }
        else if (type == GET_CLIENT_LIST) {
            //处理获取客户端列表:编号、IP地址、端口
            cout << "[INFO] Get clientList from  server is: " << endl;
            cout << reply.getContent();
            releWaitState();
            continue;
        }
        else if (type == SEND_MSG) {
            //获取信息
            tString = reply.getContent();
            int sID = reply.getCID();
            if (sID > 0) {
                cout << endl << "[INFO] Recieve message " << endl;
                cout << "Receive message from Client " << sID << ", content is " << tString << endl << "<<";
            }
            else {
                cout << endl << "[INFO] Send message: " << endl;
                cout << tString << endl;
            }
            releWaitState();
            continue;
        }
        else if (type == DISCONNECT) {
            cout << "disconnect successfully!" << endl;
            releWaitState();
            continue;
        }
    }
    return 0;
}


//关闭连接
void closeClient() {
    if (client.isConnect() == false) { //未连接
        cout << "[ERROR] No connection detected!" << endl;
        return;
    }
    client.setConnect(false);
    //close的数据包
    Message close_pack = Message(DISCONNECT, client.getID(), "disconnect"); 
    //封装
    string m_str = close_pack.PackageMsg();
    //send
    send(client.getSocket(), m_str.c_str(), m_str.size(), 0);
    int res = closesocket(client.getSocket());//关闭socket
    //状态设置为未连接
    if (res != 0) {
        cout << "[ERROR] Can't close the socket!" << endl;
        return;
    }
    cout << "[INFO] Close the connection successfully!" << endl;
    
    return;
}

void sendMsg(int op) {
    if (client.isConnect() == false) { //未连接
        return;
    }
    Message send_msg;
    send_msg.setCID(client.getID());
    switch (op) {
    case CONNECT:
        send_msg.SetContent("connection");
        send_msg.setType(CONNECT);
        break;
    case GET_TIME:
        send_msg.SetContent("get time");
        send_msg.setType(GET_TIME);
        break;
    case GET_NAME:
        send_msg.SetContent("get name");
        send_msg.setType(GET_NAME);
        break;
    case GET_CLIENT_LIST:
        send_msg.SetContent("get list");
        send_msg.setType(GET_CLIENT_LIST);
        break;
    case SEND_MSG:
        {
            int desID = -1;
            string content;
            while (!(desID > 0 && desID < MAX_CLIENT_NUM)) {
                cout << "please enter the destination client ID: " << endl;
                cin >> desID;
            }
            cout << "please enter the msg sent to destination client: " ;
            cin.ignore();
            getline(cin,content);
            send_msg.SetContent(content);
            send_msg.setType(SEND_MSG);
            send_msg.setCID(desID);
            break;
        }
    }
    //封装信息
    string m_str = send_msg.PackageMsg();
    //发送给服务端
    int ret = send(client.getSocket(), m_str.c_str(), m_str.size(), 0);
    if(ret > 0){
        setWaitState();
        while (waiting);
        //接收服务端信息
        if (waiting == false) {//接收消息完毕
            cout << "[INFO] end..." << endl;
        }
    }
}

//建立连接
void con2Server() {
    char buf[1024];
    sendMsg(CONNECT);
    char head[HEAD_LEN];
    if (client.isConnect()==true) {//已建立连接
        cout << "[ERROR] Has connected to server!" << endl;
        return;
    }
    static char ip[1024];
    int port;
    cout << "please enter IP address:" << endl;
    cin >> ip;
    cout << "please enter the port:" << endl;
    cin >> port;
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "[ERROR] failed to create socket:" << WSAGetLastError() << endl;
    }
    client.setSocket(clientSocket);

    SOCKADDR_IN* addrServe = new SOCKADDR_IN;
    addrServe->sin_family = AF_INET;
    addrServe->sin_addr.s_addr = inet_addr(ip);
    addrServe->sin_port = htons(port);

    int ret;
    ret = connect(client.getSocket(), (SOCKADDR*)addrServe, sizeof(SOCKADDR));
    if (ret == INVALID_SOCKET) {
        cout << "[ERROR] connection failed " << WSAGetLastError() << endl;
        return;
    }
    client.setConnect(true);
    //send a package for connecting successfully
    //连接后应当创建子进程用于接收服务端发送的消息
    pthread_t* thread = new pthread_t;
    pthread_create(thread, NULL, ThreadRun, &client);
}

//处理SIGINT信号
void sigHandle(int sig) {
    switch (sig) {
    case SIGINT: {
        closeClient();
        break;
    }
    default:
        break;
    }
    exit(1);
}

int main(){
    WORD	wVersionRequested;
    WSADATA wsaData;//建立一个wsdata结构
    wVersionRequested = MAKEWORD(2, 2);//create 16bit data
    int	ret, iLen;
    int op;
    pthread_t rsvP;

    //(1)Load WinSock
    ret = WSAStartup(wVersionRequested, &wsaData);	//load winsock
    if (ret != 0)    {
        cout << "[ERROR] Load WinSock Failed!";
        return 0;
    }
    //查看版本信息
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)    {
        WSACleanup();
        printf("[ERROR] Invalid Winsock version!\n");
        return 0;
    }
    while (true) {
        op = GetOperation();
        if (op == CONNECT) {
            con2Server();
            Sleep(500);
        }
        else if (op == DISCONNECT) {
            closeClient();
        }
        //修改功能
        /*
        else if (op == GET_TIME) {
            for (int i = 1; i <= 100; i++) {
                cout << "get time " << i << ": " << endl;
                sendMsg(op);
            }
        }
        */
        else if (op == EXIT) {
            if(client.isConnect()==true)
             closeClient();
            break;//退出
        }
        else {
            sendMsg(op);
        }
    }
    shutdown(client.getSocket(), SD_BOTH);
    closesocket(client.getSocket());
    //close WSA
    WSACleanup();
    //等待所有子线程退出
    pthread_exit(NULL);
    return 0;
}