#include <iostream>
#include "socket_wrap.hpp"
using namespace std;
using namespace ModelSQL;


int main(int argc, char* argv[])
{
	try {
		// создаём сокет
		UnClientSocket sock("keyfile");
		// устанавливаем соединение
		sock.connect();

		cout << endl;
		cout << "===============================================================================" << endl;
		cout << "========================== ||||||||||||||||||||||| ============================" << endl;
		cout << "========================== || THE BEST SQL EVER || ============================" << endl;
		cout << "========================== ||||||||||||||||||||||| ============================" << endl;
		cout << "===============================================================================" << endl;
		cout << endl;

		string command = "hello";
		while (command != "stop;"){
			cout << endl;
			cout << "Enter the command" << endl;
			getline(cin, command);
			//пропускаем пустые строчки
			while (command.size() == 0) {
				getline(cin, command);
			}
			command += ';';
			// отправляем серверу команду
			sock.send(command.c_str());
			// печатаем на экран ответ от сервера 
			string res = sock.recv();
			if (res == "bad") {
				cout << sock.recv() << endl;
				continue;
			}
			while (res != "OK"){
				cout << res << endl;
				res = sock.recv();
			}
		}
		cout << "===============================================================================" << endl;
		cout << "================== ||||||||||||||||||||||||||||||||||||||| ====================" << endl;
		cout << "================== || you are leaving the very best SQL || ====================" << endl;
		cout << "================== ||||||||||||||||||||||||||||||||||||||| ====================" << endl;
		cout << "===============================================================================" << endl;
		

	} catch (Exception & e) {
		// ошибка --- выводим текст сообщения на экран
		e.report();
	}
	return 0;
}