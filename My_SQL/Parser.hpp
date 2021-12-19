#ifndef PARSER_MODULE
#define PARSER_MODULE

#include <vector>
#include <stack>
#include <string>

#include "Scanner.hpp"
#include "TableWrap.hpp"


using std::vector;
using std::string;
using std::stack;


class Parser{
	Lex curr_lex;              //текущая лексема
	Type_of_lex c_type;       //тип текущей лексемы
	Scanner scan;            //сканнер
    stack<Lex> pol_stack;   //стек для полиза
    Type_of_lex where_type;//тип where клаузы
    vector<Lex> poliz;	  //полиз
    

    //для отсеивания цепочек
    enum Types { BOOL, INT, STR, ID };
    stack <Types> types;

    //вспомогательные функции
    void push_rbracket();  //при встрече ) выталкивает стек до (
    void push_op();       //выталкивает из стека операции учитывая приоритеты
    void pop_all_stack();//выталкивает весь стек в полиз
    
    void select(vector<Lex>::iterator it);    //выполняет полиз команды SELECT
    void insert(vector<Lex>::iterator it);   //выполняет полиз команды INSERT
	void update(vector<Lex>::iterator it);  //выполняет полиз команды UPDATE
	void del(vector<Lex>::iterator it);    //выполняет полиз команды DELETE
	void create(vector<Lex>::iterator it);//выполняет полиз команды CREATE
	void drop(vector<Lex>::iterator it); //выполняет полиз команды DROP

	//выдает отформатированную текстовую строку со значениями по названию полей
	string select_fields(const vector<string> & fields, const Table_w &tab);
	string title_line(vector<string> & fields, const Table_w &tab);         //возвращает заголовок таблицы
	void set_field(vector<Lex>::iterator it_eq, vector<Lex>::iterator it_wh,//считает выражение и меняет 
			const string & field_name_set, Table_w & tab);					//значение поля на результат

    //возвращает функцию считающую where 
	bool (*ret_where())(vector<Lex>::iterator f, vector<Lex>::iterator l,
			const Table_w &tab, const Scanner &scan);

    //грамматика
	void SQL();
	void SEL();
	void INS();
	void UPD();
	void DEL();
	void CRE();
	void DROP();
	void LIST(); //список полей
	void WHERE_CL();
	void VAL_FIELD();
	void STRING();
	void NUM();
	void LIST_OF_DES(); //список описаний полей
	void DES();
	void TYPE();

	void LONG_LOGIC_SEN(); //объединение выражения типа лонг и логического
	void WHERE_HELP();
	void WHERE_ADD();
	void CONST_LIST();
	void LONG_LOGIC_TERM();
	void OP_ADD();
	void LONG_LOGIC_MULT();
	void OP_MULT();
	void VALUE();
	void ADD();
	void OP_COMP();


public:
	Parser(const string &command);//конструктор
	Parser(const Parser &);		 //конструктор
	void get_lex();			    //получить лексему
	void parse(const string &command); //распарсить команду
	void calculate_poliz();			  //посчитать полиз

	vector<string> result_text;//текстовый вывод результата команды
    						  //отправится клиенту

};


//считает полиз от start до end
Lex calc_pol(vector<Lex>::iterator start, vector<Lex>::iterator end,
		const Table_w &tab, const Scanner &scan);

//вспомогательная для предыдущей функции, выполняет одну операцию
Lex execute_op(stack<Lex> & st, const Table_w &tab, Type_of_lex op, const Scanner &scan);

//4 функции высчитывающие WHERE клаузы соотвествующих типов
bool where_all(vector<Lex>::iterator first, vector<Lex>::iterator last ,
		const Table_w &tab, const Scanner &scan);

bool where_log(vector<Lex>::iterator first, vector<Lex>::iterator last, const Table_w &tab,
		const Scanner &scan);

bool where_like(vector<Lex>::iterator first, vector<Lex>::iterator last, const Table_w &tab,
		const Scanner &scan);

bool where_in(vector<Lex>::iterator first, vector<Lex>::iterator last, const Table_w &tab,
		const Scanner &scan);


#endif


