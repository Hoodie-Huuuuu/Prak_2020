#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>

#include "socket_wrap.hpp"
#include "Parser.hpp"

using namespace std;
using namespace ModelSQL;


int main(int argc, char* argv[])
{	
	try {
		// создаём серверный сокет
		UnServerSocket sock("keyfile");
		// слушаем запросы на соединение
		sock.listen(5);

		while (true) {
			//принимаем клиента
			int sock_cl = sock.accept();

			//получаем первую команду
			string command = sock.recv(sock_cl);
			while (command != "stop;"){
				try {
					//создаем парсер
					Parser p = Parser(command);
					//парсим
					p.parse(command);
					cout << endl;
					//считаем полиз
					p.calculate_poliz();
					//ответ сервера
					for (unsigned i =0; i < p.result_text.size(); ++i){
		            	sock.send(p.result_text[i].c_str(), sock_cl);
		            }
		        }
		        catch (const char * s){
					sock.send("bad", sock_cl);
					sock.send(s, sock_cl);
				}
				 catch (string & s){
					sock.send("bad", sock_cl);
					sock.send(s.c_str(), sock_cl);
				}
				catch (Errors res) {
					sock.send("bad", sock_cl);
					switch(res){
			        case OK:
			            sock.send("ok", sock_cl);
			            break;
			        case CantCreateTable:
			        	sock.send("can't create table", sock_cl);
			        	break;
			        case CantOpenTable:
			        	sock.send("can't open table" , sock_cl);
			        	break;
			        case FieldNotFound:
			        	sock.send("Field not found", sock_cl);
			        	break;
			        case BadHandle:
			        	sock.send("no such table", sock_cl);
			        	break;
			        case BadArgs:
			        	sock.send("Bad Args", sock_cl);
			        	break;
			        case CantMoveToPos:
			        	sock.send("can't move to pos", sock_cl);
			            break;
			        case CantWriteData:
			        	sock.send("can't write data", sock_cl);
			        	break;
			        case CantReadData:
			        	sock.send("can't read data", sock_cl);
			        	break;
			        case CorruptedData:
			        	sock.send("corrupted data", sock_cl);
			            break;
			        case CantCreateHandle:
			        	sock.send("can't create such table", sock_cl);
			        	break;
			        case ReadOnlyFile:
			        	sock.send( "read only file", sock_cl);
			            break;
			        case BadFileName:
			        	sock.send("such table is not exist", sock_cl);
			        	break;
			        case CantDeleteTable:
			        	sock.send("Can't delete table", sock_cl);
			        	break;
			        case CorruptedFile:
			        	sock.send("corrupted file", sock_cl);
			            break;
			        case BadPos:
			        	sock.send("Bad pos", sock_cl);
			        	break;
			        case BadFieldType:
			        	sock.send("Bad field type", sock_cl);
			        	break;
			        case BadFieldLen:
			        	sock.send("Bad fied len", sock_cl);
			        	break;
			        case NoEditing:
			        	sock.send("no editing", sock_cl);
			            break;
			        case BadPosition:
			        	sock.send("bad position", sock_cl);
			        	break;
			        case TableNotOpen:
			        	sock.send("table not opened", sock_cl);
			        	break;
			        default:
			            break;
					}
				}
				catch (int c){
					string mail;
					mail += c;
					mail += " - unexpected symbol";
					sock.send("bad", sock_cl);
					sock.send(mail.c_str(), sock_cl);
				}
	            //получаем очередную команду
	            command = sock.recv(sock_cl);
	            cout << command << endl;///////////////можно убрат
	        }
			//закончили с клиентом
			sock.send("OK", sock_cl);
		}
	}	
	
	catch (Exception & e) {
		// ошибка --- выводим текст сообщения на экран
		e.report();
	}
	return 0;
}


/*
for (unsigned i =0; i < p.poliz.size(); ++i){
	cout << p.poliz[i] << endl;
}
*/ //вывод полиза



