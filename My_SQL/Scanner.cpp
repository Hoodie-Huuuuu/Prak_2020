#include <string>
#include <vector>
#include <fstream>

#include "Scanner.hpp"


using std::string;

//================================= Lex ===================================
//конструктор
Lex::Lex(Type_of_lex type_lex, long val)
{
	type = type_lex;
	value = val;
}


//констурктор копирования
Lex::Lex(const Lex & l)
{
	type = l.type;
	value = l.value;
}


//получить тип лексемы
Type_of_lex
Lex::get_type() const
{
	return type;
}


//получить значение лексемы(номер в таблице лексем)
long
Lex::get_value() const
{
	return value;
}


//================================= Ident ===================================
//конструктор
Ident::Ident(const string & name_ident)
{
	name = name_ident;
}


//конструктор копирования
Ident::Ident(const Ident & id)
{
	name = id.name;
}


//вернуть имя
string
Ident::get_name() const
{
	return name;
}


//============================== Table_ident ============================
//добавить идентификатор или вернуть его номер в таблице
long
Table_ident::put(const string & name_id)
{
	long j;
	for (j = 0; j < (long)idents.size(); ++j)
		if (name_id == idents[j].get_name()) {
			return j;
		}
	idents.push_back(Ident(name_id));
	return j;
}


size_t
Table_ident::get_size() const
{
	return idents.size();
}


Ident
Table_ident::operator[](long i) const
{
	return idents[i];
}

//================================= Scanner ===============================
//конструктор преобразования
Scanner::Scanner(const string &command)
{
	str_in = command;
	curr_state = H;
	c = ' ';
	clear();
}


//конструктор копирования
Scanner::Scanner(const Scanner &s)
{
	curr_state = s.curr_state;
	str_in = s.str_in;
	c = s.c;
	buf = s.buf;
}


//операция присваивания
Scanner &Scanner::operator=(const Scanner &s)
{
	curr_state = s.curr_state;
	str_in = s.str_in;
	c = s.c;
	buf = s.buf;
	return *this;
}


//почистить буфер
void
Scanner::clear()
{
	buf.clear();
}


//добавить символ в буфер
void
Scanner::add()
{
	buf += c;
}


//функция поиска слова в таблице разделителей или таблице служебных слов
long
Scanner::look(const string & word, const char **list){
	long i = 0;
	while (list[i]){
		if (word == list[i]) {
			return i;
		}
		++i;
	}
	return 0;
}


//положить строку в таблицу строковых констант
long
Scanner::put_str(const string & text)
{
	long i = 0;
	for (i = 0; i < (long)table_strings.size(); ++i){
		if (text == table_strings[i]){
			return i;
		}
	}
	table_strings.push_back(text);
	return i;
}


//взять символ из строки
void
Scanner::gc()
{
	c = str_in[0];
	str_in.erase(0, 1);
}


//таблица служебных слов
const char * Scanner::table_words[] = {
	"",		  //0
	"SELECT", //1 
	"FROM",   //2 
	"INSERT", //3 
	"INTO",   //4 
	"UPDATE", //5 
	"SET",    //6 
	"DELETE", //7 
	"CREATE", //8 
	"TABLE",  //9
	"TEXT",   //10 
	"LONG",   //11 
	"DROP",   //12 
	"WHERE",  //13 
	"LIKE",   //14
	"NOT",    //15
	"IN",     //16
	"ALL",    //17 
	"OR",     //18
	"AND",    //19
	NULL
};


//таблица разделителей
const char * Scanner::table_delim[] = {
	"",	 //0
	",", //1 
	"*", //2 
	"(", //3 
	")", //4
	"'", //5 
	"+", //6 
	"-", //7 
	"/", //8 
	"%", //9 
	"=", //10 
	">", //11 
	"<", //12 
	">=",//13 
	"<=",//14 
	"!=",//15
	NULL
};


//таблица типов служебных слов
Type_of_lex Scanner::lex_words[] = {
	LEX_NULL,	  //0
	LEX_SELECT,   //1
	LEX_FROM,     //2
	LEX_INSERT,   //3
	LEX_INTO,     //4
	LEX_UPDATE,   //5
	LEX_SET,      //6
	LEX_DELETE,   //7
	LEX_CREATE,   //8
	LEX_TABLE_COM,//9
	LEX_TEXT,     //10
	LEX_LONG,	  //11
	LEX_DROP,	  //12
	LEX_WHERE,	  //13
	LEX_LIKE,	  //14
	LEX_NOT,	  //15
	LEX_IN,       //16
	LEX_ALL,	  //17
	LEX_OR,	      //18
	LEX_AND,	  //19
	LEX_NULL	  //20
};


//таблица типов разделителей
Type_of_lex Scanner::lex_delim[] = {
	LEX_NULL,	 //0	
	LEX_COMMA,   //1
	LEX_STAR,    //2
	LEX_LBRACKET,//3
	LEX_RBRACKET,//4
	LEX_APOSTR,  //5
	LEX_PLUS,	 //6
	LEX_MINUS,   //7
	LEX_SLASH,   //8
	LEX_PERCENT, //9
	LEX_EQ,		 //10
	LEX_MORE,	 //11
	LEX_LESS,	 //12
	LEX_MORE_EQ, //13
	LEX_LESS_EQ, //14
	LEX_NEQ,	 //15
	LEX_NULL	 //16
};


Lex
Scanner::get_lex()
{
	long num = 0, pos = 0;;
	curr_state = H;
	
	do{
		switch (curr_state){
		case H:

			if (c == ';') {
				return Lex(LEX_FIN, 0);
			}
			if (isspace(c))
				gc ();
			else if (isalpha(c)) {
				clear ();
				add ();
				gc ();
				curr_state = IDENT;

			} else if (isdigit(c)) {
				num = c - '0';
				gc ();
				curr_state = NUMB;
			
			} else if (c == '<' || c == '>' || c == '!'){
				clear();
				add(); 
				gc();
				curr_state = ALE;
			
			} else if (c == '\'') {
				clear();
				gc();
				curr_state = STRING;
			}
			else {
				curr_state = DELIM;
			}
			break;

		case IDENT:

			if (isalpha(c) || isdigit(c) || c == '_') {
				add();
				gc();
			} else {
				if ((pos = look(buf, table_words))) {
					return Lex(lex_words[pos], pos);

				} else {
					pos = table_ident.put(buf);
					return Lex(LEX_ID, pos);
				}
			}
			break;

		case NUMB:

			if (isdigit(c)){
				num = num * 10 + (c - '0');
				gc();
			} else {
				return Lex(LEX_NUM, num);
			}
			break;

		case STRING:
			if (c == '\'') {
				gc();
				pos = put_str(buf);
				return Lex(LEX_STRING, pos);
			
			} else if (c == ';' || c == EOF) {
				throw "expected sym <'> ";

			} else {
				add();
				gc();
			}
			break;

		case ALE:
			if (c == '='){
				add ();
				gc ();
				pos = look(buf, table_delim);
				return Lex(lex_delim[pos], pos);

			} else {
				if ((pos = look(buf, table_delim))) {
					return Lex(lex_delim[pos], pos);
				} else {
					throw c;
				}
				
			}
			break;

		case DELIM:
			clear ();
			add ();
			if ((pos = look(buf, table_delim))){
				gc ();
				return Lex(lex_delim[pos], pos);
			} else {
				throw c;
			}
			break;
		default:
			break;
		} // end switch
	} while (true);
}

