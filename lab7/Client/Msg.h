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
 //������͵İ���С��send���ܴ���������һ�η�����
#define ERROR_TYPE 7
#define MAX_CLIENT_NUM 32

//����/��Ӧ���ݰ�����
enum Pack_Type { CONNECT = 1, DISCONNECT, GET_TIME, GET_NAME, GET_CLIENT_LIST, SEND_MSG, EXIT};

// |TYPE 5 ID 3| LEN | LEN | CONTENT
class Message {
private:
    int len; //���ݰ�content����
    int type; //���ݰ�����
    int ID; // ���ݰ�ID�ֶ�
    char head; //ͷ����Ϣ |TYPE 5 ID 3|
    std::string content;//content
    std::string msg; // ���͵�msg:����TYPE_ID + len + content
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
    

    void ParseMsgHead(); //����Msgͷ head
    int ParseMsgLength(char l[]);//����Msg len
    std::string ParseMsg(); //����Msg
    std::string PackageMsg(); //��װContent
    void SetContent(std::string content) {
        this->content = content;
    }
};
