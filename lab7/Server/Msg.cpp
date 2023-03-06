/*
 * Lishiyu
 * _MSG_CPP_
 */
 // this part is used to define the protocol of application layer

#include "Msg.h"

int Message::ParseMsgLength(char l[]) {
    int len = (((int)l[0] << 8) & 0xFF00) + (l[1] & 0xFF);
    this->len = len;
    return len;
}

void Message::ParseMsgHead() {
    type = ((head & 0xE0) >> 5);
    ID = head & 0x1F;
}

std::string Message::ParseMsg() {
    this->head = this->msg[0];
    ParseMsgHead();
    this->len = (((int)this->msg[1] << 8) & 0xFF00) | (this->msg[2] & 0xFF);
    this->content = this->msg.substr(LEN_BYTE + TYPEID_BYTE);
    return this->content;
}

//����msg��len�ֶ�
std::string Message::PackageMsg() {
    if (type >= CONNECT && type <= SEND_MSG) { //type��1~6��Χ��
        head = (char)type;
    }
    else { //�Ƿ�type
        head = ERROR_TYPE;
        this->content = "";
    }
    head = (head & 0x7) << 5;
    if (this->ID >= 0 && this->ID <= 31) { //ID�Ϸ�
        head = (head + (char)this->ID);
    }
    this->len = this->content.length();
    if (this->len > MAX_SIZE) {
        std::cout << "MSG too long!" << std::endl;
    }
    this->msg.resize(3);
    this->msg[0] = head; //type+ID
    this->msg[1] = (this->len >> 8) & 0xFF;//len-���ֽ�
    this->msg[2] = this->len & 0xFF; //len-���ֽ�
    this->msg.append(this->content,0,this->len); //��ȷ����
    return this->msg;
}
