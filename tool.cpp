#define SPACY_HEADER_ONLY
#include "spacy/spacy"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>
#include <cstdio>
#include <cctype>

using namespace std;
namespace fs = filesystem;

// Загрузка предобученной модели SpaCy
Spacy::Spacy spacy;
auto nlp = spacy.load("ru_core_news_lg");

// Структура, в которой хранятся все необходимые данные
struct toolWord 
{
    string token;
    string head;
    string tokenPos;
    string headPos;
    string tokenDep;
    string headDep;
    int member;
};

// Функция, которая анализирует предложения по словам
// Первый цикл while нужен для заполнения полей token, head и member
// для того чтобы сохранить расставленные в ручную цифры
// Второй цикл while нужен для заполнения остальных полей
// но при этом анализируются слова без расставленных в ручную цифр, так как они влияют на тэги))))
void parceText(vector<toolWord>& words, fstream& input, string str)
{
    int index = 0;
    // Перебор всех предложений из файла
    while (getline(input, str)) 
    {
        auto doc = nlp.parse(str); // Парсер предобученной модели SpaCy
        for (auto& token : doc.tokens()) // Перебор предложения по словам
        {
            auto tokenHead = token.head();
            auto head = tokenHead.text(); // Главное слово в словосочетании           
            auto word = token.text(); // Зависимое слово в словосочетании

            words.push_back(toolWord()); // Создаем один элемент вектора words (элементом является структура с пустыми полями)
            words[index].token = word; // Заполняем поля структуры в векторе
            words[index].head = head;
            if (word[0] == '1')
            {
                words[index].member = 1;
            } 
            if (word[0] == '2')
            {
                words[index].member = 2;
            } 
            if (word[0] == '3')
            {
                words[index].member = 3;
            }
            if (word[0] == '4')
            {
                words[index].member = 4;
            }
            if (word[0] == '5')
            {
                words[index].member = 5;
            }                                                   
            index++; // Переходим к следующему элементу 
        }
    }

    index = 0;
    input.clear();
    input.seekg(0); // Возвращаем указатель на начало файла

    while (getline(input, str)) 
    {
        string result = "";
        for (char c : str) // Убираем все расставленные цифры
        {
            if (!isdigit(c)) {
            result += c;
            }
        }
        auto doc = nlp.parse(result);
        for (auto& token : doc.tokens())
        {
            auto head = token.head();
            auto word = token.text();

            words[index].tokenPos = token.pos_();
            words[index].tokenDep = token.dep_();
            words[index].headPos = head.pos_();
            words[index].headDep = head.dep_();       
            index++;
        }
    }

}
// Функция, которая обеспечивает взаимодействие с текстовым файлом
void fileMain(vector<toolWord>& words) {
    string str, text;
    string fileName;
    cout << "Введите название входного файла:" << endl; 
    getline(cin, fileName);
    fstream input(fileName);
    if (input.is_open()) 
    {
        parceText(words, input, str); // Парсим текст из файла 
    }
    input.close();
}

// Функция, которая отвечает за сохранение данных в текстовый файл
void saveToFile(vector<toolWord>& words, string fileName) 
{
    system("clear");
    ofstream output(fileName);
    if (!output.is_open()) 
    {
        cout << "Ошибка при открытии файла для записи." << endl;
    }

    // Отобразить информацию о каждом слове в векторе words 
    for (auto& word : words) 
    {
        output << "Token: " << word.token << " Head: " << word.head 
        << " Token Pos/Dep: " << word.tokenPos << "/" << word.tokenDep
        << " Head Pos/Dep: " << word.headPos << "/" << word.headDep << " Member of Sentence: " << word.member << endl;
    }

    cout << "Результаты сохранены в файле: " << fileName << endl;
    output.close();
}

int main()
{
    vector<toolWord> words; // Объявление вектора
    string outFile = "toolOutput.txt"; // Прости хоспаде, захардкоженный файл нейм
    setlocale(LC_ALL, "ru_RU.UTF-8"); // Адекватная кодировка для Linux терминала
    fileMain(words); // Работа с входным файлом
    saveToFile(words, outFile); // Работа с выходным файлом
    return 0;
}