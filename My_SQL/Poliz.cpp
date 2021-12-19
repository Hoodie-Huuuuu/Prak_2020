#include <vector>
#include <string>
#include <list>
#include <stack>
#include <cstring>
#include <regex>

#include "Parser.hpp"
#include "Scanner.hpp"
#include "TableWrap.hpp"


extern "C" {
    #include "table.h"
}

enum{
	BUF_SIZE = 8,
	LEN_LONG = 20
};


using std::string;
using std::vector;
using std::stack;

//=========================== Calculate poliz ==================================
void
Parser::calculate_poliz()
{
	if (poliz.size() == 0) throw "empty poliz";
	vector<Lex>::iterator it = poliz.begin();
	if (it->get_type() == LEX_SELECT) select(it);
	if (it->get_type() == LEX_INSERT) insert(it);
	if (it->get_type() == LEX_UPDATE) update(it);
	if (it->get_type() == LEX_DELETE) del(it);
	if (it->get_type() == LEX_CREATE) create(it);
	if (it->get_type() == LEX_DROP) drop(it);
	return;
}

//это операция
#define is_op(c) ((c) == LEX_PLUS || (c) == LEX_MINUS || (c) == LEX_OR ||      \
					(c) == LEX_AND || (c) == LEX_STAR || (c) == LEX_SLASH ||    \
					(c) == LEX_PERCENT || (c) == LEX_EQ || (c) == LEX_MORE ||    \
					(c) == LEX_LESS || (c) == LEX_MORE_EQ || (c) == LEX_LESS_EQ ||\
					(c) == LEX_NEQ)

//посчитать полиз с заданным началом и концом
Lex
calc_pol(vector<Lex>::iterator start, vector<Lex>::iterator end, const Table_w &tab,
		const Scanner &scan)
{
	stack<Lex> result;
	start += 1;
	while (start != end){
		if (!is_op(start->get_type())) {
			result.push(*start);
		} else {
			result.push(execute_op(result, tab, start->get_type(), scan));
		}
		start += 1;
	}
	//может вернуть просто имя поля, если выражение состоит только из него
	return result.top();
}


Lex
execute_op(stack<Lex> & st, const Table_w &tab, Type_of_lex op, const Scanner &scan)
{
	Lex second = st.top();
	st.pop();
	Lex first = st.top();
	st.pop();
	
	long val_1_long = 0, val_2_long = 0;
	string str1, str2;

	//надо проверять на типы только идентификаторы
	FieldType type2, type1;
	if (second.get_type() == LEX_ID) {
		string field_name = scan.table_ident[second.get_value()].get_name();
		type2 = tab.get_field_type(field_name);

		if (type2 == Long) {
			val_2_long = tab.get_long(field_name);
		} else if (type2 == Text) {
			str2 = tab.get_text(field_name);
		}

	} else if (second.get_type() == LEX_STRING){
		type2 = Text;
		str2 = scan.table_strings[second.get_value()];

	} else if (second.get_type() == LEX_NUM){
		type2 = Long;
		val_2_long = second.get_value();
		
	} else { //BOOL
		val_2_long = second.get_value();
		type2 = Logic;
	}

	if (first.get_type() == LEX_ID){
		string field_name = scan.table_ident[first.get_value()].get_name();
		type1 = tab.get_field_type(field_name);

		if (type1 == Long) {
			val_1_long = tab.get_long(field_name);
		} else if (type1 == Text) {
			str1 = tab.get_text(field_name);
		}

	} else if (first.get_type() == LEX_STRING){
		type1 = Text;
		str1 = scan.table_strings[first.get_value()];

	} else if (first.get_type() == LEX_NUM){
		type1 = Long;
		val_1_long = first.get_value();

	} else { //BOOL
		type1 = Logic;
		val_1_long = first.get_value();
	}

	if (type1 != type2) throw "types error; check types of fields";

	if (op == LEX_PLUS) {
		return Lex(LEX_NUM, val_1_long + val_2_long);

	} else if (op == LEX_MINUS){
		return Lex(LEX_NUM, val_1_long - val_2_long);

	} else if (op == LEX_STAR) {
		return Lex(LEX_NUM, val_1_long * val_2_long);

	} else if (op == LEX_SLASH) {
		return Lex(LEX_NUM, val_1_long / val_2_long);

	} else if (op == LEX_PERCENT) {
		return Lex(LEX_NUM, val_1_long % val_2_long);

	} else if (op == LEX_AND) {
		return Lex(LEX_BOOL, val_1_long && val_2_long);

	} else if (op == LEX_OR) {
		return Lex(LEX_BOOL, val_1_long || val_2_long);

	} else if (op == LEX_MORE) {
		if (type1 == Long){
			return Lex(LEX_BOOL, val_1_long > val_2_long);
		} else {
			return Lex(LEX_BOOL, str1 > str2);
		}
		
	} else if (op == LEX_LESS) {
		if (type1 == Long){
			return Lex(LEX_BOOL, val_1_long < val_2_long);
		} else {
			return Lex(LEX_BOOL, str1 < str2);
		}

	} else if (op == LEX_MORE_EQ) {
		if (type1 == Long){
			return Lex(LEX_BOOL, val_1_long >= val_2_long);
		} else {
			return Lex(LEX_BOOL, str1 >= str2);
		}

	} else if (op == LEX_LESS_EQ) {
		if (type1 == Long){
			return Lex(LEX_BOOL, val_1_long <= val_2_long);
		} else {
			return Lex(LEX_BOOL, str1 <= str2);
		}

	} else if (op == LEX_NEQ) {
		if (type1 == Long){
			return Lex(LEX_BOOL, val_1_long != val_2_long);
		} else {
			return Lex(LEX_BOOL, str1 != str2);
		}

	}  else /*(op == LEX_EQ)*/ {
		if (type1 == Long){
			return Lex(LEX_BOOL, val_1_long == val_2_long);
		} else {
			return Lex(LEX_BOOL, str1 == str2);
		}
	}
}

//================================== SELECT ====================================
void
Parser::select(vector<Lex>::iterator it)
{
	vector<string> fields;//заполняем именами полей
	while (it->get_type() != LEX_FROM){
		it += 1;
		if (it->get_type() == LEX_ID){
			fields.push_back(scan.table_ident[it->get_value()].get_name());
		}
	}
	//если fields пусто значит встречена звездочка

	it += 1;
	Table_w table(scan.table_ident[it->get_value()].get_name());
	it += 1;//it на слове where

	//напечатать заглавие таблицы
	result_text.push_back("================== table " + table.get_tab_name() + " ==================");
	result_text.push_back(title_line(fields, table));

	//указатель на функцию
	bool (*where)(vector<Lex>::iterator, vector<Lex>::iterator,
			const Table_w &, const Scanner &);
	where = ret_where();//вернет нужную функцию

	table.move_first();
	while (!table.end()){
		if (where(it, poliz.end(), table, scan) == true) {
			result_text.push_back(select_fields(fields, table));
		}
		table.move_next();
	}
	result_text.push_back("OK");
}


//вернуть строку - заголовок таблицы
string
Parser::title_line(vector<string> & fields, const Table_w &tab)
{
	string ret;
	if (fields.size() == 0){
		for (unsigned i = 0; i < tab.get_field_num(); ++i){
			string field_name = tab.get_field_name(i);
			fields.push_back(field_name);//заполняем всеми именами для select_fields
			field_name.resize(tab.get_field_len(field_name), ' ');
			ret += field_name;
			ret += ' ';
		}
	} else {
		for (unsigned i = 0; i < fields.size(); ++i){
			bool field_exist = false;
			for (unsigned j = 0; j < tab.get_field_num(); ++j){
				if (tab.get_field_name(j) == fields[i]){
					field_exist = true;
					string field_name = tab.get_field_name(i);;

					field_name.resize(tab.get_field_len(field_name), ' ');
					ret += field_name;
					ret += ' ';
					break;
				}
			}
			if (field_exist == false) throw "field " + fields[i] + " does not exist";
		}
	}
	return ret;
}


string
Parser::select_fields(const vector<string> & fields, const Table_w &tab)
{
	string ret;

	unsigned num_fields;
	num_fields = tab.get_field_num();
	if (fields.size() > num_fields) throw "to much num of fields";

	bool find_field = false;
	for (unsigned i = 0; i < fields.size(); ++i){
		for (unsigned j = 0; j < num_fields; ++j){
			if (tab.get_field_name(j) == fields[i]){
				find_field = true;
				string field;
				FieldType type = tab.get_field_type(fields[i]);
				if (type == Long) {
					field = std::to_string(tab.get_long(fields[i]));
				} else {//text
					field = tab.get_text(fields[i]);
				}

				//форматирование строки
				field.resize(tab.get_field_len(fields[i]), ' ');
				ret += field;
				ret += ' ';
				break;
			}
		}
		if (!find_field){
			string mail;
			mail += "field " + fields[i] + " does not exist";
			throw mail;
		}
		find_field = false;
	}
	return ret;
}
//================================ UPDATE =====================================
void
Parser::update(vector<Lex>::iterator it)
{
	it += 1;
	Table_w table(scan.table_ident[it->get_value()].get_name());
	it += 1;
	string field_name_set = scan.table_ident[it->get_value()].get_name();
	it += 1;
	vector<Lex>::iterator it_eq = it;//итератор указывающий на знак равно

	while (it->get_type() != LEX_WHERE){
		it += 1;
	}
	//it на слове where
	//указатель на функцию
	bool (*where)(vector<Lex>::iterator, vector<Lex>::iterator,
			const Table_w &, const Scanner &);
	where = ret_where();//вернет нужную функцию

	table.move_first();
	while (!table.end()){
		if (where(it, poliz.end(), table, scan) == true) {
			set_field(it_eq, it, field_name_set, table);
		}
		table.move_next();
	}
	result_text.push_back("OK");	
}


void
Parser::set_field(vector<Lex>::iterator it_eq, vector<Lex>::iterator it_wh,
		const string & field_name_set, Table_w & tab)
{
	FieldType type = tab.get_field_type(field_name_set);
	Lex result = calc_pol(it_eq, it_wh, tab, scan);

	if (result.get_type() == LEX_NUM && type == Long){
		tab.put_long(field_name_set, result.get_value());

	} else if (result.get_type() == LEX_STRING && type == Text){
		tab.put_text(field_name_set, scan.table_strings[result.get_value()]);

	} else if (result.get_type() == LEX_ID){
		string id_name = scan.table_ident[result.get_value()].get_name();
		FieldType type_id = tab.get_field_type(id_name);

		if (type_id == Text && type == Text) {
			tab.put_text(field_name_set, tab.get_text(id_name));
		} else if (type_id == Long && type == Long){
			tab.put_long(field_name_set, result.get_value());
		} else {
			throw "check types of fields in UPDATE sentence";
		}
	} else {
		throw "in update sentence type of field and the expression type are different";
	}
	return;
}


//================================ DElETE =====================================
void
Parser::del(vector<Lex>::iterator it)
{
	it += 1;
	Table_w table(scan.table_ident[it->get_value()].get_name());
	it += 1; //it на слове where
	//указатель на функцию
	bool (*where)(vector<Lex>::iterator, vector<Lex>::iterator,
			const Table_w &, const Scanner &);
	where = ret_where();//вернет нужную функцию
	
	table.move_first();
	while (!table.end()){
		if (where(it, poliz.end(), table, scan) == true) {
			table.del_rec();
		}
		table.move_next();
	}
	result_text.push_back("OK");
}


//=================================== DROP =====================================
void
Parser::drop(std::vector<Lex>::iterator it)
{
	it += 1;
	Table_w table(scan.table_ident[it->get_value()].get_name());
	table.del();
	result_text.push_back("OK");
}


//================================== INSERT ====================================
void
Parser::insert(std::vector<Lex>::iterator it)
{
	it += 1;
	Table_w table(scan.table_ident[it->get_value()].get_name());//открывает существующую таблицу 
	table.new_rec();//новая пустая запись

	
	int counter = 0;//счетчик полей
	//заполняем новую запись значениями
	it += 1;
	while (it->get_type() != LEX_RBRACKET){
		
		if (it->get_type() == LEX_NUM){
			FieldType type = table.get_field_type(table.get_field_name(counter));  //смотрим тип поля
			if (type != Long) throw "not TEXT arg in insert command";
			table.put_long_new(table.get_field_name(counter), it->get_value());//добавляем значение
																		//в новую запись
		} else { //STRING
			FieldType type = table.get_field_type(table.get_field_name(counter));
			if (type != Text) throw "not LONG arg in insert command";
			//сравниваем длину поля
			if (table.get_field_len(table.get_field_name(counter)) <
					scan.table_strings[it->get_value()].size())
				throw "bad size of text field in insert";
			//добавляем строку в поле записи
			table.put_text_new(table.get_field_name(counter), scan.table_strings[it->get_value()]);
		}
		it += 1;
		++counter;
	}
	if (counter != (int)table.get_field_num()) throw "bad num of fields in insert";
	table.insert_end(); //вставляем запись в конец
	result_text.push_back("OK");
}


//====================================== CREATE ==========================================
void
Parser::create(vector<Lex>::iterator it)
{
	it += 1;
	long name_val = it->get_value();//имя таблицы
	TableStruct tab_struct;	
	
	FieldDef *mas = new FieldDef[BUF_SIZE];
	int buf_size = BUF_SIZE;
	int count_fields = 0;

	it += 1;
	while (it->get_type() != LEX_RBRACKET){
		//realloc
		if (count_fields == buf_size) {
			FieldDef *mas1 = new FieldDef[buf_size *= 2];
			for (int i = 0; i < buf_size/2; ++i){
				mas1[i] = mas[i];
			}
			delete [] mas;
			mas = mas1;
		}
		FieldDef field;
		strcpy(field.name, scan.table_ident[it->get_value()].get_name().c_str());
		it += 1;
		if (it->get_type() == LEX_LONG){
			field.type = Long;
			field.len = LEN_LONG;
		} else { //TEXT
			field.type = Text;
			it += 1;
			field.len = it->get_value();
		}
		mas[count_fields++] = field;
		it += 1;
	}
	tab_struct.numOfFields = count_fields;
	tab_struct.fieldsDef = mas;

	Table_w new_tab(scan.table_ident[name_val].get_name(), tab_struct);
	delete[] mas;
	new_tab.open();
	result_text.push_back("OK");
}



//================================== WHERE =========================================
bool
where_all(vector<Lex>::iterator first, vector<Lex>::iterator last, const Table_w &tab,
		const Scanner &scan)
{
	return true;
}


bool
where_log(vector<Lex>::iterator first, vector<Lex>::iterator last, const Table_w &tab,
		const Scanner &scan)
{
	Lex result(calc_pol(first, last, tab, scan));
	return result.get_value();
}


bool
where_like(vector<Lex>::iterator first, vector<Lex>::iterator last, const Table_w &tab,
		const Scanner &scan)
{
	first += 1;
	string name = scan.table_ident[first->get_value()].get_name();
	if (tab.get_field_type(name) != Text) throw "field is not text type in WHERE ... LIKE";
	string val = tab.get_text(name);

	first += 1;
	bool flag_not = false;
	if (first->get_type() == LEX_NOT){
		flag_not = true;
		first += 1;
	}

	string smpl = scan.table_strings[first->get_value()];

	size_t pos = 0;
	while ((pos = smpl.find("%", pos)) != string::npos) {
		smpl.replace(pos, 1, ".*");
		pos += 2;
	}

	pos = 0;
	while ((pos = smpl.find("_", pos)) != string::npos) {
		smpl.replace(pos, 1, ".");
		pos += 1;
	}	

	std::regex reg(smpl);
	if (regex_match(val, reg) && !flag_not) return true;
	return false;
}


bool
where_in(vector<Lex>::iterator first, vector<Lex>::iterator last, const Table_w &tab,
		const Scanner &scan)
{
	//проверка типов выражения и констант <выражение> IN (список констант)
	vector<Lex>::iterator it_in = first;

	bool flag_not = false;
	while (it_in->get_type() != LEX_IN){
		if (it_in->get_type() == LEX_NOT) {
			flag_not = true;
		}
		it_in += 1;
	}
	Lex result;
	if (flag_not) {
		result = calc_pol(first, it_in - 1, tab, scan);
	} else {
		result = calc_pol(first, it_in, tab, scan);
	}
	

	FieldType type_result;
	string str_value;
	long long_value;
	if (result.get_type() == LEX_ID) {
		string field_name = scan.table_ident[result.get_value()].get_name();
		type_result = tab.get_field_type(field_name);
		if (type_result == Long) {
			long_value = tab.get_long(field_name);
		} else {
			str_value = tab.get_text(field_name);
		}

	} else if (result.get_type() == LEX_NUM) {
		type_result = Long;
		long_value = result.get_value();
	} else { //LEX_STRING
		type_result = Text;
		str_value = scan.table_strings[result.get_value()];
	}

	it_in += 1;
	//определяем тип по первой константе
	FieldType type_consts;
	if (it_in->get_type() == LEX_STRING) {
		type_consts = Text;
	} else {
		type_consts = Long;
	}

	if (type_result != type_consts) throw "in WHERE ... IN consts must have another type";
	//сравнение
	if (type_result == Text) {
		while(it_in != last){
			if (str_value == scan.table_strings[it_in->get_value()]){
				if (flag_not) {
					return false;
				} else {
					return true;
				}
			} 
			it_in += 1;
		}
	} else { //Long
		while (it_in != last){
			if (long_value == it_in->get_value()){
				if (flag_not){
					return false;
				} else {
					return true;
				}
			}
			it_in += 1;
		}
	}
	if (flag_not) {
		return true;
	} else {
		return false;
	}
	
}


//возвращает указатель на функцию bool func(iterator, iterator, table)
bool (*Parser::ret_where())(vector<Lex>::iterator first, vector<Lex>::iterator last,
		const Table_w &tab, const Scanner &scan)
{
	switch (where_type){
		case LEX_ALL:
			return where_all;
			break;
		case LEX_LIKE:
			return where_like;
			break;
		case LEX_IN:
			return where_in;
			break;
		default:
			return where_log;
			break;
	}//end switch
}




//=========================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ============================

#define ITS_OP(c) (c == LEX_STAR || c == LEX_SLASH || c == LEX_PERCENT ||\
				  c == LEX_AND || c == LEX_PLUS || c == LEX_MINUS ||      \
				  c == LEX_OR || c == LEX_LESS || c == LEX_MORE ||         \
				  c == LEX_MORE_EQ || c == LEX_LESS_EQ || c == LEX_EQ ||    \
				  c == LEX_NEQ)

//при встрече ) выталкивает стек до (
void
Parser::push_rbracket()
{
	while (pol_stack.size() > 0){
		Lex top_lex(pol_stack.top());

		if (top_lex.get_type() == LEX_LBRACKET){
			pol_stack.pop();//убираем открывающуюся скобку из стека
			return;
		} else {
			poliz.push_back(top_lex);
			pol_stack.pop();
		}
	}
	throw "the brackets are not matched";
	return;
}


 //выталкивает из стека операции учитывая приоритеты
void
Parser::push_op()
{
	if (c_type == LEX_EQ || c_type == LEX_MORE || c_type == LEX_LESS ||
			c_type == LEX_MORE_EQ || c_type == LEX_LESS_EQ || c_type == LEX_NEQ){

		while (pol_stack.size() > 0){
			Lex top_lex(pol_stack.top());

			if (ITS_OP(top_lex.get_type())){
				//приоритет операции на вершине стека
				//больше или равен текущей, значит выталкиваем из стека
				poliz.push_back(top_lex);
				pol_stack.pop();
			} else {
				break;
			}
		}
		pol_stack.push(curr_lex);

	} else if (c_type == LEX_STAR || c_type == LEX_SLASH ||
				c_type == LEX_PERCENT || c_type == LEX_AND){
		while (pol_stack.size() > 0){
			Lex top_lex(pol_stack.top());
			if (top_lex.get_type() == LEX_STAR || top_lex.get_type() == LEX_SLASH ||
					top_lex.get_type() == LEX_PERCENT || top_lex.get_type() == LEX_AND){
				//приоритет операции на вершине стека
				//больше или равен текущей, значит выталкиваем из стека
				poliz.push_back(top_lex);
				pol_stack.pop();
			} else {
				break;
			}
		}
		pol_stack.push(curr_lex);
	} else if (c_type == LEX_PLUS || c_type == LEX_MINUS || c_type == LEX_OR) {
		while (pol_stack.size() > 0){
			Lex top_lex(pol_stack.top());
			if (top_lex.get_type() == LEX_STAR || top_lex.get_type() == LEX_SLASH ||
					top_lex.get_type() == LEX_PERCENT || top_lex.get_type() == LEX_AND ||
					top_lex.get_type() == LEX_PLUS || top_lex.get_type() == LEX_MINUS ||
					top_lex.get_type() == LEX_OR){ //приоритет операции на вершине стека
												  //больше или равен текущей, значит выталкиваем из стека
				poliz.push_back(top_lex);
				pol_stack.pop();
			} else {
				break;
			}
		}
		pol_stack.push(curr_lex);
	}
}


//выталкивает весь стек в полиз
void
Parser::pop_all_stack()
{
	while (pol_stack.size() > 0) {
		poliz.push_back(pol_stack.top());
		pol_stack.pop();
	}
	return;
}


















