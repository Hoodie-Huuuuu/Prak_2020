#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <iostream>

#include "socket_wrap.hpp"


#define CHECK(func, errcode) if ((func) == -1) throw Exception(errcode)
namespace {
    enum{
        SIZE_BUF = 1024
    };  
}
using namespace ModelSQL;

//================================== Exception ==================================
//конструктор исключений
ModelSQL::Exception::Exception(Ex_type errcode)
{
    switch(errcode){
    case EX_CREATE_SOCK:
        msg = "error create socket";
        break;
    case EX_CONN_SOCK:
        msg = "error connect socket";
        break;
    case EX_LISTEN_SOCK:
        msg = "error listen socket";
        break;
    case EX_ACCEPT_SOCK:
        msg = "error accept socket";
        break;
    case EX_BIND_SOCK:
        msg = "error bind socket";
        break;
    case EX_SEND_SOCK:
        msg = "error send socket";
        break;
    case EX_RECV_SOCK:
        msg = "error recv socket";
        break;
    case EX_SHUTDOWN_SOCK:
        msg = "error shutdown socket";
        break;
    case EX_CLOSE_SOCK:
        msg = "error close socket";
        break;
    default:
        msg = "error";
        break;
    }
}


//вывести сообщение об ошибке
void
ModelSQL::Exception::report() const
{
    std::cout << msg << endl;
    return;
}


//================================ BaseUnSocket ================================
//конструктор базового сокета
BaseUnSocket::BaseUnSocket()
{
    //создать сокет
    CHECK(my_sock = socket(AF_UNIX, SOCK_STREAM, 0), Exception::EX_CREATE_SOCK);
}


BaseUnSocket::BaseUnSocket(int unix_sock)
{
    my_sock = unix_sock;
}


//отправить сообщение
void
BaseUnSocket::send(const char *msg, int sock) const
{
    //с учетом '\n'
    if (sock == 0) sock = my_sock;
    int len = strlen(msg) + 1;

    //отправить длину сообщения
    CHECK(write(sock, &len, sizeof(int)), Exception::EX_SEND_SOCK);

    //отправить сообщение
    CHECK(write(sock, msg, len), Exception::EX_SEND_SOCK);

    return;
}


//прочитать сообщение
string
BaseUnSocket::recv(int sock) const
{
    if (sock == 0) sock = my_sock;
    int len = 0;
    //читаем длину
    CHECK(read(sock, &len, sizeof(int)), Exception::EX_RECV_SOCK);

    string res;
    char buf[SIZE_BUF];
    int num_of_cells = len / SIZE_BUF + 1;
    int len_for_read = SIZE_BUF;

    for (int i = 0; i < num_of_cells; ++i){
        if (len < SIZE_BUF){
            len_for_read = len;
        }
        //читаем сообщение
        CHECK(read(sock, buf, len_for_read), Exception::EX_RECV_SOCK);
        //добавить считанную порцию
        res += buf;
        len -= SIZE_BUF;
    }
    return res; 
}


//закрыть соккет
void
BaseUnSocket::close()
{
    shutdown(my_sock, 2);
    ::close(my_sock);
    return;
}


BaseUnSocket::~BaseUnSocket()
{
    close();
}


//=============================== UnClientSocket ===============================
//сокет для клиентов

//конструктор
UnClientSocket::UnClientSocket(const char *server_adr)
{
    adr_server.sun_family = AF_UNIX;
    strcpy(adr_server.sun_path, server_adr);
}


//соединиться с сервером  
void
UnClientSocket::connect() const
{
    CHECK(::connect(my_sock, (const struct sockaddr*)&adr_server, sizeof(adr_server)),
            Exception::EX_CONN_SOCK);
    return;
}


//вернуть адрес сервера
string
UnClientSocket::get_server_adr() const
{
    return adr_server.sun_path;
}


//=============================== UnServerSocket ===============================
//сокет для сервера

//конструктор
UnServerSocket::UnServerSocket(const char *adress)
{
    my_adr.sun_family = AF_UNIX;
    strcpy(my_adr.sun_path, adress);
    unlink(adress);
    CHECK(bind(my_sock, (struct sockaddr *)&my_adr, sizeof(my_adr)), Exception::EX_BIND_SOCK);
}


//перейти в состояние listen
void
UnServerSocket::listen(const int max_size_queue) const
{
    CHECK(::listen(my_sock, max_size_queue), Exception::EX_LISTEN_SOCK);
}


//принять запрос клиента и вернуть сокет
int
UnServerSocket::accept()  const
{
    int sock_cl = 0;
    CHECK(sock_cl = ::accept(my_sock, NULL, NULL), Exception::EX_ACCEPT_SOCK);
    return sock_cl;
}


UnServerSocket::~UnServerSocket()
{
    unlink(my_adr.sun_path);
}


#undef CHECK






