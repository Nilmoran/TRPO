#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <cstdio>

using namespace std;

struct Word 
{
    string subject; // Вектор для хранения всех подлежащих из предложения
    string pronoun; // Вектор для хранения всех сказуемых из предложения
    string modifier; // Вектор для хранения всех определений из предложения
    string object; // Вектор для хранения всех дополнений из предложения
    string adverbial; // Вектор для хранения всех обстоятельств из предложения
    string sentence; // Предложение в котором определяются члены предложения 

    Word(string& sub, string& pron, string& mod, string& obj, string& adv, string sent) 
    : subject(sub), pronoun(pron), modifier(mod), object(obj), adverbial(adv), sentence(sent) {} // Стандартный конструктор для структуры
};
// Функция, которая отвечает за вывод меню
void printMenu();

// Функция, которая отвечает за проверку названия файла на запрещенные символы
string checkingFileName(int);

// Функция, которая разделяет текст на предложения
vector<string> splitSentences(string&);

// Функция, которая формирует буферный файл в котором каждое преложение находится на отдельной строке
string processFile(string&);

// Функция, которая анализирует предложения по словам
void parceText(vector<Word>&, ifstream&, string);

// Функция, которая обеспечивает взаимодействие с текстовым файлом
void fileMain(vector<Word>&);

// Функция, которая отвечает за отображение результатов анализа
void printResults(vector<Word>&, int);

// Функция, которая отвечает за сохранение данных в текстовый файл
bool saveToFile(vector<Word>&, int);

// Функция, которая отвеает за сохранение данных перед выходом из программы
bool exitSave(vector<Word>&);
