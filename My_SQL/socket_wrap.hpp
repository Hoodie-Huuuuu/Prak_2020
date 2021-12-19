#ifndef SOCK_WRAP_MODULE
#define SOCK_WRAP_MODULE

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>

using namespace std;

//================================ BaseUnSocket ================================
//базовый класс сокетов на UNIX
class BaseUnSocket {
protected:
    int my_sock;
public:
    BaseUnSocket();               //конструктор
    BaseUnSocket(int unix_sock); //за корректность сокета отвечает пользователь
    ~BaseUnSocket();            //деструктор
    void send(const char *msg, int sock = 0) const;//отправить сообщение
    string recv(int sock = 0) const;              //получить сообщение
    void close();                                //закрыть сокет
};


//=============================== UnClientSocket ===============================
//сокет для клиентов
class UnClientSocket: public BaseUnSocket {
    struct sockaddr_un adr_server; //адрес сервера
public:
    UnClientSocket(const char *server_adr); //конструктор
    void connect() const;                  //подключиться к серверу
    string get_server_adr() const;        //узнать адрес сервера
};


//=============================== UnServerSocket ===============================
//сокет для сервера
class UnServerSocket: public BaseUnSocket {
    struct sockaddr_un my_adr; //адрес сервера
public:
    UnServerSocket(const char *adress); //конструктор
    ~UnServerSocket();                 //деструктор  
    void listen(int size_queue) const;//состояние listen
    int accept() const;              //взять клиента из очереди
};


//================================== Exception ==================================
namespace ModelSQL
{
    //исключения
    class Exception
    {
        string msg;

    public:
        enum Ex_type
        {
            EX_CREATE_SOCK,
            EX_CONN_SOCK,
            EX_LISTEN_SOCK,
            EX_ACCEPT_SOCK,
            EX_BIND_SOCK,
            EX_SEND_SOCK,
            EX_RECV_SOCK,
            EX_SHUTDOWN_SOCK,
            EX_CLOSE_SOCK
        };
        void report() const;
        Exception(Ex_type errcode);
    };
}

#endif
