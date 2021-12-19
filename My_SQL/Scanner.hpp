#ifndef SCANER_MODULE
#define SCANER_MODULE


#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

//все типы лексем
enum Type_of_lex {
	LEX_NULL = 0,
	LEX_SELECT,
	LEX_FROM,
	LEX_INSERT,
	LEX_INTO,
	LEX_UPDATE,
	LEX_SET,
	LEX_DELETE,
	LEX_CREATE,
	LEX_TABLE_COM,//команда
	LEX_TABLE,	  //идентификатор - имя таблицы
	LEX_TEXT,
	LEX_LONG,
	LEX_STRING,
	LEX_NUM,
	LEX_BOOL,
	LEX_DROP,
	LEX_WHERE,
	LEX_LIKE,
	LEX_NOT,
	LEX_IN,
	LEX_ALL,
	LEX_OR,
	LEX_AND,
	LEX_ID,
	LEX_FIN,  //конец команды
	LEX_COMMA,//разделители 
	LEX_STAR,
	LEX_LBRACKET,
	LEX_RBRACKET,
	LEX_APOSTR,
	LEX_PLUS,
	LEX_MINUS,
	LEX_SLASH,
	LEX_PERCENT,
	LEX_EQ,
	LEX_MORE,
	LEX_LESS,
	LEX_MORE_EQ,
	LEX_LESS_EQ,
	LEX_NEQ
};

//================================= Lex ===================================
class Lex {
	long value;
	Type_of_lex type;
public:
	Lex(Type_of_lex type_lex = LEX_NULL, long val = 0); //конструктор
	Lex(const Lex & l);								   //конструктор копирования
	Type_of_lex get_type() const;					  //вернуть тип лексемы
	long get_value() const;							 //вернуть значение лексемы
};


//================================ Ident ===================================
class Ident {
	string name;//имя
public:
	Ident(const string & name_ident);//конструктор
	Ident(const Ident & id);	    //конструктор копирвоания
    
	string get_name() const;
};


//================================ Table_ident ==============================
class Table_ident {
	vector<Ident> idents;//вектор идентификаторов
public:
	size_t get_size() const;
	long put(const string & name_id);//добавить идентификатор или вернуть его номер в таблице
	Ident operator[](long i) const;
};


//================================== Scanner ================================
class Scanner {
	enum 					state { H, IDENT, NUMB, STRING, ALE, DELIM };//состояния лексического анализатора
	static Type_of_lex 		lex_words[]; //таблица типов лексем служебных слов
	static Type_of_lex 		lex_delim[];//таблица типов лексем разделителей
	state 					curr_state;//текущее состояние анализатора
	string                  str_in;   //анализируемая строка
	int  					c;		 //текущий символ
	string 					buf;	//буфер

	void clear();                       //почистить буфер
	void add();                        //дбавить символ в буфер
	long put_str(const string & text);//положить строку в таблицу строковых констант
	long look(const string & word, const char **list);
	
public:
	static const char 		*table_words[];  //таблица служебных слов
	static const char 		*table_delim[]; //таблица разделителей
	Table_ident 			table_ident;   //таблица идентификаторов
	vector<string>			table_strings;//массив строковых констант

	void gc();    //получить следующий символ
	Lex get_lex();//получить лексему
	Scanner (const string &command);//констурктор
	Scanner (const Scanner &);	   //конструктор копирования
	Scanner &operator=(const Scanner &);
};

#endif