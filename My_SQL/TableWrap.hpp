#ifndef TABLE_WRAP_MODULE
#define TABLE_WRAP_MODULE

#include <iostream>
#include <string>

//для преобразований Сишных имен функций
extern "C" {
    #include "table.h"
}


using std::string;

class Table_w {
    THandle td;      //дескриптор таблицы
    string name_tab;//имя таблицы
public:
    Table_w(const string & tab_name, struct TableStruct fields);//конструктор
    Table_w(const string & tab_name); //для уже существующих таблиц в памяти
    ~Table_w();            //деструктор  
    void open();          //открываем таблицу
    void del();          //удаляет таблицу тз памяти
    void move_first();  //Функция устанавливает указатель файла на первую запись (если она есть)
    void move_last();  //Функция устанавливает указатель файла на последнюю запись (если она есть)
    void move_next(); //Функция устанавливает указатель файла на следующую в файле запись (если она есть)
    void move_prev();//Функция устанавливает указатель файла на предыдущую запись (если она есть)

    bool begin() const;  //Функция выдает значение TRUE, если таблица пуста или если в состоянии
                        //"на первой записи" выполняется операция movePrevios,
                       //иначе выдается значение FALSE.
    bool end() const ;//Функция выдает значение TRUE, если таблица пуста или если в состоянии
                     //"на последней записи" выполняется операция moveNext,
                    //иначе выдается значение FALSE. */

    string get_text(const string & field_name) const; //получить из текущей записи значение поля
    long get_long(const string & field_name) const;

    void put_text(const string & field_name, const string & val); //положить в текущую запись
    void put_long(const string & field_name, long val);          //значение в нужное поле  

    void new_rec(); //создать новую пустую запись и заполнять ее следующими функциями
    void del_rec();//Функция удаляет текущую запись. При этом, если есть следующая запись,
                  //то она автоматически становится текущей, если нет,
                 //то появляется состояние "после последней".

    void put_text_new(const string & field_name, const string & val);//функции для заполнения
    void put_long_new(const string & field_name, long val);         //новой записи

    void insert();       //вставляет перед текущей записью
    void insert_begin();//вставляет в начало
    void insert_end(); //вставляет в конец

    unsigned get_field_len(const string & field_name) const;
    FieldType get_field_type(const string & field_name) const;
    unsigned get_field_num() const ;
    string get_field_name(unsigned idx) const;

    string get_tab_name() const;
};


#endif





