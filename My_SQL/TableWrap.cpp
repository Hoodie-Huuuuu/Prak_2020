#include "TableWrap.hpp"

extern "C" {
    #include "table.h"
}


//конструктор
Table_w::Table_w(const string & name, struct TableStruct fields)
{
    name_tab = name;
    //создаем таблицу
    Errors res = createTable(name.c_str(), &fields);
    if (res != OK) throw res;
}


//для уже существующих таблиц в памяти
Table_w::Table_w(const string & name)
{
  name_tab = name;
  Errors res = openTable(name_tab.c_str(), &td);
  if (res != OK) throw res;
}


//открыть таблицу
void
Table_w::open()
{
    //открываем файл таблицы
    Errors res = openTable(name_tab.c_str(), &td);
    if (res != OK) throw res;
}


//деструктор
Table_w::~Table_w()
{
    //закрываем таблицу
    closeTable(td);
    
}


//удаляет таблицу из памяти
void
Table_w::del()
{
    //удаляем файл с таблцей
    Errors res = deleteTable(name_tab.c_str());
    if (res != OK) throw res;
}


//Функция устанавливает указатель файла на первую запись (если она есть)
void
Table_w::move_first()
{
    Errors res = moveFirst(td);
    if (res != OK) throw res;
    return;
}


//Функция устанавливает указатель файла на последнюю запись (если она есть)
void
Table_w::move_last()
{
    Errors res = moveLast(td);
    if (res != OK) throw res;
    return;
}


//Функция устанавливает указатель файла на следующую в файле запись (если она есть)
void
Table_w::move_next()
{
    Errors res = moveNext(td);
    if (res != OK) throw res;
    return;
}


//Функция устанавливает указатель файла на предыдущую запись (если она есть)
void
Table_w::move_prev()
{
    Errors res = movePrevios(td);
    if (res != OK) throw res;
    return;
}


//описание в хэддере
bool
Table_w::begin() const
{
    return beforeFirst(td);
}


//описание в хэддере
bool
Table_w::end() const
{
    return afterLast(td);
}


 //получить из текущей записи значение поля типа текст
string
Table_w::get_text(const string &field_name) const
{
    char *buf;
    Errors res = getText(td, field_name.c_str(), &buf);
    if (res != OK) throw res;
    return buf;
}


//получить из текущей записи значение поля типа лонг
long
Table_w::get_long(const string &field_name) const
{
    long ret;
    Errors res = getLong(td, field_name.c_str(), &ret);
    if (res != OK) throw res;
    return ret;
}


//положить в текущую запись в поле типа текст строку
void
Table_w::put_text(const string & field_name, const string & val)
{
    Errors res = startEdit(td);
    if (res != OK) throw res;

    res = putText(td, field_name.c_str(), val.c_str());
    if (res != OK) throw res;

    res = finishEdit(td);
    if (res != OK) throw res;
}


//положить в текущую запись в поле типа лонг число
void
Table_w::put_long(const string & field_name, long val)
{
    Errors res = startEdit(td);
    if (res != OK) throw res;

    res = putLong(td, field_name.c_str(), val);
    if (res != OK) throw res;

    res = finishEdit(td);
    if (res != OK) throw res;
}


//создать новую пустую запись и заполнять ее следующими функциями
void
Table_w::new_rec()
{
    Errors res = createNew(td);
    if (res != OK) throw res;
    return;   
}


//описание в хэддере
void
Table_w::del_rec()
{
    Errors res = deleteRec(td);
    if (res != OK) throw res;
    return;
}


//заполняет поле типа тект в новой записи
void
Table_w::put_text_new(const string & field_name, const string & val)
{
    Errors res = putTextNew(td, field_name.c_str(), val.c_str());
    if (res != OK) throw res;
    return;
}


//заполняет поле типа лонг в новой записи
void
Table_w::put_long_new(const string & field_name, long val)
{
    Errors res = putLongNew(td, field_name.c_str(), val);
    if (res != OK) throw res;
    return;
}


void
Table_w::insert()
{
    Errors res = insertNew(td);
    if (res != OK) throw res;
    return;
}


void
Table_w::insert_begin()
{
    Errors res = insertaNew(td);
    if (res != OK) throw res;
    return;
}


void
Table_w::insert_end()
{
    Errors res = insertzNew(td);
    if (res != OK) throw res;
    return;
}


unsigned
Table_w::get_field_len(const string & field_name) const
{
    unsigned ret;
    Errors res = getFieldLen(td, field_name.c_str(), &ret);
    if (res != OK) throw res;
    return ret;
}


FieldType
Table_w::get_field_type(const string & field_name) const
{
    FieldType ret;
    Errors res = getFieldType(td, field_name.c_str(), &ret);
    if (res != OK) throw res;
    return ret;
}


unsigned
Table_w::get_field_num() const
{
    unsigned ret;
    Errors res = getFieldsNum(td, &ret);
    if (res != OK) throw res;
    return ret;
}


string
Table_w::get_field_name(unsigned idx) const
{
    char *buf;
    Errors res = getFieldName(td, idx, &buf);
    if (res != OK) throw res;
    return string(buf);
}


string
Table_w::get_tab_name() const
{
    return name_tab;
}




