/*
 * Lishiyu
 * _MSG_H_
 */



#pragma once

#include <string>
#include <iostream>

#define LEN_BYTE 2
#define TYPEID_BYTE 1
#define HEAD_LEN 3
#define MAX_SIZE 2048
 //如果发送的包很小，send可能处理让数据一次发送完
#define ERROR_TYPE 7
#define MAX_CLIENT_NUM 32

//请求/响应数据包类型
enum Pack_Type { CONNECT = 1, DISCONNECT, GET_TIME, GET_NAME, GET_CLIENT_LIST, SEND_MSG, EXIT};

// |TYPE 5 ID 3| LEN | LEN | CONTENT
class Message {
private:
    int len; //数据包content长度
    int type; //数据包类型
    int ID; // 数据包ID字段
    char head; //头部信息 |TYPE 5 ID 3|
    std::string content;//content
    std::string msg; // 发送的msg:包括TYPE_ID + len + content
public:
    Message() {
        content = "";
        msg = "";
        ID = 0;
        len = 0;
    }
    Message(int type, int ID, std::string content) {
        this->type = type;
        this->ID = ID;
        this->content = content;
        this->len = content.length();
        this->head = (((char)type & 0x7) << 5) | ((char)ID & 0x31);
    }
    Message(std::string msg) {
        this->msg = msg;
    }


    std::string getMsg() { return this->msg; }
    std::string getContent() { return this->content; }
    int  getCID() { return ID; }
    int  getType() { return type; }
    void setMsg(std::string s) { this->msg = s; }
    void setContent(std::string s) { this->content = s; }
    void setType(int t) { this->type = t; }
    void setCID(int id) { ID = id; }
    void setHead(char head) { this->head = head; }
    

    void ParseMsgHead(); //解析Msg头 head
    int ParseMsgLength(char l[]);//解析Msg len
    std::string ParseMsg(); //解析Msg
    std::string PackageMsg(); //封装Content
    void SetContent(std::string content) {
        this->content = content;
    }
};
