#include "Scanner.hpp"
#include "Parser.hpp"

//конструктор
Parser::Parser(const string &command): scan(command)
{
	where_type = LEX_NULL;
}


//конструктор
Parser::Parser(const Parser &p): scan(p.scan)
{
	where_type = LEX_NULL;
}


//получить лексему
void
Parser::get_lex()
{
	curr_lex = scan.get_lex();
	c_type = curr_lex.get_type();
	return;
}


//запустить парсер
void
Parser::parse(const string &command)
{
	//чистка для новой команды
	result_text.clear();
	poliz.clear();
	while(!types.empty()){
    	types.pop();
	}
    while(!pol_stack.empty()){
    	pol_stack.pop();
    }
    //конец чистки
	scan = Scanner(command);
	scan.gc();
	get_lex();
	SQL();
	if (c_type == LEX_FIN) {
		std::cout << "GREATE" << std::endl;///////////////можно убрать
	} else {
		std::cout << "BAD" << std::endl;/////////////////можно убрать
	}
	while (pol_stack.size() > 0){ //заталкивем в полиз все, что осталось в стеке
		poliz.push_back(pol_stack.top());
		pol_stack.pop();
	}
	return;
}

//===================================== ГРАММАТИКА =======================================//
//попутно собирается полиз

//SQL -> select SEL | insert INS | update UPD | delete DEL | create CRE | drop DROP 
void
Parser::SQL()
{
	switch (c_type) {
	case LEX_SELECT:
		poliz.push_back(curr_lex);
		get_lex();
		SEL();
		break;

	case LEX_INSERT:
		poliz.push_back(curr_lex);
		get_lex();
		INS();
		break;
	
	case LEX_UPDATE:
		poliz.push_back(curr_lex);
		get_lex();
		UPD();
		break;
	
	case LEX_DELETE:
		poliz.push_back(curr_lex);
		get_lex();
		DEL();
		break;

	case LEX_CREATE:
		poliz.push_back(curr_lex);
		get_lex();
		CRE();
		break;

	case LEX_DROP:
		poliz.push_back(curr_lex);
		get_lex();
		DROP();
		break;

	default:
		throw "expected SELECT | INSERT | UPDATE | DELETE | CREATE | DROP";
	} //end switch;
	return;
}


//SEL -> ID {, ID} from ID WHERE_CL | * from ID WHERE_CL
void
Parser::SEL()
{
	if (c_type == LEX_ID){
		poliz.push_back(curr_lex);
		get_lex();
		while (c_type == LEX_COMMA){
			get_lex();
			if (c_type != LEX_ID) throw "expected id after SELECT";
			poliz.push_back(curr_lex);
			get_lex();
		}
	} else if (c_type == LEX_STAR) {
		poliz.push_back(curr_lex);
		get_lex();
	} else {
		throw "expected id or * in SELECT command";
	}

	if (c_type != LEX_FROM) throw "expected FROM";
	poliz.push_back(curr_lex);

	get_lex();
	if (c_type != LEX_ID) throw "expected id after FROM";
	poliz.push_back(curr_lex);

	get_lex();
	WHERE_CL();
}


//INS-> into ID (VAL_FIELD {, VAL_FIELD})
void
Parser::INS()
{
	if (c_type != LEX_INTO) throw "expected INTO after INSERT";

	get_lex();
	if (c_type != LEX_ID) throw "expected id after INTO";
	poliz.push_back(curr_lex);

	get_lex();
	if (c_type != LEX_LBRACKET) throw "expected ( in INSERT command";

	get_lex();
	VAL_FIELD();

	get_lex();
	while (c_type == LEX_COMMA){
		get_lex();
		VAL_FIELD();
		get_lex();
	}
	if (c_type != LEX_RBRACKET) throw "expected ) in INSERT command";
	poliz.push_back(curr_lex);
	get_lex();//конец строки
}


//VAL_FIELD-> STRING | NUM
void
Parser::VAL_FIELD()
{
	if (c_type == LEX_MINUS) {
		get_lex();
		if (c_type != LEX_NUM) throw "expected NUM or STRING const";
		poliz.push_back(Lex(LEX_NUM, (curr_lex.get_value()) * (-1) ));
		return;
	}
	if (c_type != LEX_STRING && c_type != LEX_NUM) throw "expected text or long const";
	poliz.push_back(curr_lex);
}


//UPD -> ID set ID = LONG_LOGIC_SEN WHERE_CL
void
Parser::UPD()
{
	if (c_type != LEX_ID) throw "expected id after UPDATE";
	poliz.push_back(curr_lex);

	get_lex();
	if (c_type != LEX_SET) throw "expected SET after id";

	get_lex();
	if (c_type != LEX_ID) throw "expected id after SET";
	poliz.push_back(curr_lex);

	get_lex();
	if (c_type != LEX_EQ) throw "expected = after id";
	poliz.push_back(curr_lex);

	get_lex();
	LONG_LOGIC_SEN();

	//отсеивание цепочек
	if (types.top() == BOOL){
		types.pop();
		throw "set costruction must be long or string type";
	}
	types.pop();
	//конец отсеивания

	pop_all_stack();
	WHERE_CL();
}


//DEL-> from ID WHERE_CL
void
Parser::DEL()
{
	if (c_type != LEX_FROM) throw "expected FROM after DELETE";

	get_lex();
	if (c_type != LEX_ID) throw "expected id after FROM in DELETE command";
	poliz.push_back(curr_lex);

	get_lex();
	WHERE_CL();
}


//CRE -> table ID (LIST_OF_DES)
void
Parser::CRE()
{
	if (c_type != LEX_TABLE_COM) throw "expected TABLE after CREATE";

	get_lex();
	if (c_type != LEX_ID) throw "expected id after TABLE in CREATE command";
	poliz.push_back(curr_lex); 

	get_lex();
	if (c_type != LEX_LBRACKET) throw "expected ( in CREATE command";

	get_lex();
	LIST_OF_DES();
	//get_lex внтури цикла while в процедуре LIST_OF_DES
	if (c_type != LEX_RBRACKET) throw "expected ) in CREATE command";
	poliz.push_back(curr_lex);
	get_lex();//конец строки
}


//LIST_OF_DES -> DES {, DES}
void
Parser::LIST_OF_DES()
{
	DES();
	get_lex();
	while (c_type == LEX_COMMA){
		get_lex();
		DES();
		get_lex();
	}
}


//DES-> ID TYPE
void
Parser::DES()
{
	if (c_type != LEX_ID) throw "expected id in description of field";
	poliz.push_back(curr_lex);
	get_lex();
	TYPE();
}


//TYPE -> text (NUM) | long
void
Parser::TYPE()
{
	if (c_type == LEX_TEXT){
		poliz.push_back(curr_lex);

		get_lex();
		if (c_type != LEX_LBRACKET) throw "expected ( after TEXT";
		get_lex();
		if (c_type != LEX_NUM) throw "expected number after TEXT";
		poliz.push_back(curr_lex);

		get_lex();
		if (c_type != LEX_RBRACKET) throw "expected ) in CREATE command";
		return;
	}
	if (c_type == LEX_LONG){
		poliz.push_back(curr_lex);
		return;
	}
	throw "expected LONG or TEXT in description of fields";
	return;
}


//DROP -> table ID
void
Parser::DROP()
{
	if (c_type != LEX_TABLE_COM) throw "expected TABLE after DROP";
	get_lex();
	if (c_type != LEX_ID) throw "expected id after TABLE";
	poliz.push_back(curr_lex);
	get_lex();//конец строки
}


//WHERE_CL -> where WHERE_HELP
void
Parser::WHERE_CL()
{
	if (c_type != LEX_WHERE) {
		throw "expected WHERE";
	}
	poliz.push_back(curr_lex);
	get_lex();
	WHERE_HELP();
}


//WHERE_HELP -> all | LONG_LOGIC_SEN WHERE_ADD
void
Parser::WHERE_HELP()
{
	if (c_type == LEX_ALL){
		where_type = LEX_ALL;
		poliz.push_back(curr_lex);
		get_lex();//конец строки
		return;
	} else {
		LONG_LOGIC_SEN();
		WHERE_ADD();


	}
}


//WHERE_ADD -> [not] like STRING | [not] in (CONST_LIST) | eps
void
Parser::WHERE_ADD()
{
	if (c_type == LEX_NOT){
		pop_all_stack();
		poliz.push_back(curr_lex);
		get_lex();
	}
	if (c_type == LEX_LIKE){

		//отсеивание цепочек
		if (types.top() != ID) {
			types.pop();
			throw "expected Field text type in WHERE ... LIKE";
		}
		//конец отсеиввания

		where_type = LEX_LIKE;//для счета полиза
		pop_all_stack();
		get_lex();
		if (c_type != LEX_STRING) throw "expected STRING after LIKE";
		poliz.push_back(curr_lex);
		get_lex();//конец строки

	} else if (c_type == LEX_IN){

		//отсеивание цепочек
		if (types.top() == BOOL) {
			types.pop();
			throw "expected STRING or LONG in WHERE ... IN";
		}
		//конец отсеивания

		where_type = LEX_IN;//для счета полиза
		pop_all_stack();
		poliz.push_back(curr_lex);
		get_lex();
		if (c_type != LEX_LBRACKET) throw "expected ( after IN";
		get_lex();
		CONST_LIST();
		if (c_type != LEX_RBRACKET) throw "expected ) after list of consts";
		poliz.push_back(curr_lex);
		get_lex();//конец строки

	} else {
		//значит where логический и результа должен быть логический
		if (types.top() != BOOL) {
			types.pop();
			throw "WHERE result must be bool";
		}
	}
	//eps
	return;
}


//CONST_LIST -> STRING {, STRING} | NUM {, NUM}
void
Parser::CONST_LIST()
{
	if (c_type == LEX_STRING){
		poliz.push_back(curr_lex);
		get_lex();
		while (c_type == LEX_COMMA){
			get_lex();
			if (c_type != LEX_STRING) throw "expected STRING in const list";
			poliz.push_back(curr_lex);
			get_lex();
		}

	} else if (c_type == LEX_NUM || c_type == LEX_MINUS){
		if (c_type == LEX_MINUS) {
			get_lex();
			if (c_type != LEX_NUM) throw "expected NUM or STRING in const list";
			poliz.push_back(Lex(LEX_NUM, (curr_lex.get_value()) * (-1) ));
		} else {
			poliz.push_back(curr_lex);
		}
		get_lex();
		while (c_type == LEX_COMMA){
			get_lex();
			if (c_type == LEX_MINUS) {
				get_lex();
				if (c_type != LEX_NUM) throw "expected NUM in const list";
				poliz.push_back(Lex(LEX_NUM, (curr_lex.get_value()) * (-1) ));
			} else {
				if (c_type != LEX_NUM) throw "expected NUM in const list";
				poliz.push_back(curr_lex);
			}
			get_lex();
		}
	} else {
		throw "expected NUM or STRING in const list";
	}
	return;
}


//LONG_LOGIC_SEN -> LONG_LOGIC_TERM {OP_ADD LONG_LOGIC_TERM}
//OP_ADD -> + | - | or
void
Parser::LONG_LOGIC_SEN()
{
	LONG_LOGIC_TERM();
	while (c_type == LEX_PLUS || c_type == LEX_MINUS || c_type == LEX_OR) {
		push_op();
		Type_of_lex curr_op = c_type;//запомнить операцию
		get_lex();
		LONG_LOGIC_TERM();

		//отсеивание цепочек
		Types second_type = types.top();
		types.pop();
		Types first_type = types.top();
		types.pop();
		
		if ((curr_op == LEX_PLUS || curr_op == LEX_MINUS) &&
				(first_type == BOOL || first_type == STR ||
				second_type == BOOL || second_type == STR)){
			throw "+ or - of bool or string types";
		}
			
		if (curr_op == LEX_OR && (first_type != BOOL || second_type != BOOL)){
			throw "OR of not bool types";
		}

		if (curr_op == LEX_OR) {
			types.push(BOOL);
		} else {
			types.push(INT);
		}
		//конец отсеивания
	}
	return;
}


//LONG_LOGIC_TERM -> LONG_LOGIC_MULT {OP_MULT LONG_LOGIC_MULT}
//OP_MULT -> * | / | % | and
void
Parser::LONG_LOGIC_TERM()
{
	LONG_LOGIC_MULT();
	while (c_type == LEX_STAR || c_type == LEX_SLASH || c_type == LEX_PERCENT || c_type == LEX_AND){
		push_op();
		Type_of_lex curr_op = c_type;//запомнить операцию
		get_lex();
		LONG_LOGIC_MULT();

		//отсеивание цепочек
		Types second_type = types.top();
		types.pop();
		Types first_type = types.top();
		types.pop();

		if ((curr_op != LEX_AND) &&
				(first_type == BOOL || first_type == STR ||
				second_type == BOOL || second_type == STR)){
			throw "* or / or 0/0 of bool or string types";
		}

		if (curr_op == LEX_AND && (first_type != BOOL || second_type != BOOL)){
			throw "AND of not bool types";
		}

		if (curr_op == LEX_AND) {
			types.push(BOOL);
		} else {
			types.push(INT);
		}
		//конец отсеивания
	}
	return;
}


//LONG_LOGIC_MULT -> VALUE | (LONG_LOGIC_SEN ADD | not LONG_LOGIC_MULT
//VALUE -> ID | NUM | STRING
void
Parser::LONG_LOGIC_MULT()
{
	if (c_type == LEX_ID){
		poliz.push_back(curr_lex);
		types.push(ID);
		get_lex();//конец строки
		return;
	}
	if (c_type == LEX_NUM) {
		poliz.push_back(curr_lex);
		types.push(INT);
		get_lex();//конец строки
		return;
	}
	if (c_type == LEX_STRING){
		poliz.push_back(curr_lex);
		types.push(STR);
		get_lex();//конец строки
		return;
	}
	if (c_type == LEX_LBRACKET){
		pol_stack.push(curr_lex);
		get_lex();
		LONG_LOGIC_SEN();
		ADD();
	}
	if (c_type == LEX_NOT) {
		get_lex();
		LONG_LOGIC_MULT();
	}
}


//ADD -> OP_COMP LONG_LOGIC_SEN ) | )
//OP_COMP -> = | > | < | >= | <= | !=
void
Parser::ADD()
{
	if (c_type == LEX_EQ || c_type == LEX_MORE || c_type == LEX_LESS ||
			c_type == LEX_MORE_EQ || c_type == LEX_LESS_EQ || c_type == LEX_NEQ){
		push_op();
		get_lex();
		LONG_LOGIC_SEN();

		//отсеивание цепочек
		Types second_type = types.top();
		types.pop();
		Types first_type = types.top();
		types.pop();

		if (first_type == BOOL || second_type == BOOL ||
				(first_type == STR && second_type == INT) ||
				(second_type == STR && first_type == INT)) {
			throw "error types compare";
		} else {
			types.push(BOOL);
		}
		//конец отсеивания

		if (c_type != LEX_RBRACKET) throw "expected ) in WHERE sentence";
		push_rbracket();

		get_lex();//конец строки
	} else if (c_type == LEX_RBRACKET){
		push_rbracket();
		get_lex();//конец строки
	} else {
		throw "expected ) in WHERE sentence";
	}
	return;
}

























