#define SPACY_HEADER_ONLY
#include "spacy/spacy"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdio.h>

using namespace std;

namespace fs = std::filesystem; 
bool hasUnsavedChanges = false;
bool uploadFromFile = false;
void printMenu()
{
    cout << "Выберите операцию: " << endl
        << "1: загрузить текст файла" << endl
        << "2: сохранить результаты в файл" << endl
        << "3: вывести результат на консоль"
        << "4: выйти" << endl;
}

// Функция, которая отвечает за проверку названия файла на запрещённые символы
// Также функция выполняет проверку на наличие в папке файл с таким же именем
// Возвращает корректное название файла
string checkingFileName(int mode)
{
    string path;
    ifstream buf;
    string fileName;
    bool flag;
    do
    {
        flag = true;
        if (mode == 1)
            cout << "Введите символ ‘=’, для того чтобы прекратить ввод названия входного файла и вернуться в главное меню." << endl;
        else
            cout << "Введите символ ‘=’, для того чтобы прекратить ввод названия выходного файла и вернуться в главное меню." << endl;

        getline(cin, fileName); // Получить название файла
        if (fileName == "=")
        {
            return fileName;
        }

        for (int i = 0; i < fileName.size(); i++) // Проверить название файла на запрещенные символы
        {
            if ((fileName[i] == '\\') ||
                 (fileName[i] == '/') ||
                 (fileName[i] == ':') ||
                 (fileName[i] == '*') ||
                 (fileName[i] == '?') ||
                 (fileName[i] == '"') ||
                 (fileName[i] == '<') ||
                 (fileName[i] == '|')
                )
            {
                cout << "Не корректный ввод, повторите попытку, не используя специальные символы!" << '\n';
                flag = false;
                break;
            }
        }

        if (flag == true)
        {
            if (mode == 1) // Если это название входного файла, то проверить его наличие в папке с программой

            {
                if (fileName.find(".txt") == string::npos) // Добавить расширение, если его нет
                {
                    fileName += ".txt";
                }
                path = fs::current_path().string() + "/" + fileName;
                buf.open(path);
                if (!buf.is_open()) // Если файл не удалось открыть
                {
                    cout << "Файл не найден проверьте его наличие по пути: " << path << endl;
                    flag = false;
                }
            }
            if (mode == 2) // Если это название выходного файла, то проверить на существование файла с таким же названием
            {
                if (fileName.find(".txt") == string::npos)
                {
                    fileName += ".txt";
                }
                if (fs::exists(fileName)) // Если файл существует, то добавить к нему номер
                {
                    int count = 1;
                    string baseName, extension, newFileName;
                    size_t dotPos = fileName.find_last_of('.'); // Определить позицию с которой начинается расширение

                    // Разделить полученное на название файла и его расширение
                    if (dotPos != string::npos)
                    {
                        baseName = fileName.substr(0, dotPos);
                        extension = fileName.substr(dotPos);
                    }

                    do
                    {
                        newFileName = baseName + "(" + to_string(count) + ")" + extension;  // Добавить номер между названием файла и его расширением
                        count++;
                    } while (fs::exists(newFileName));
                    path = fs::current_path().string() + "/" + newFileName;
                    cout << "Файл с таким названием уже существует, поэтому будет создан файл:\n " << path << endl;

                    return newFileName;
                }
                else
                {
                    path = fs::current_path().string() + "/" + fileName;
                    cout << "Текущая структура данных была записана в файл по пути " << path << endl;
                    return fileName;
                }
            }
        }

    } while (flag == false);

    return fileName;
}
bool isNounOrPron(string tokenpos){
    if(tokenpos == "NOUN" || tokenpos == "PRON" || tokenpos == "PROPN" ){
        return true;
    } else {
        return false;
    }
}
void spacy()
{
    string str, text;
    string fileName;
    fileName = checkingFileName(1); // Получить название входного файла
    ifstream input(fileName);
    Spacy::Spacy spacy;
    auto nlp = spacy.load("ru_core_news_lg");
    if(input.is_open()){
        int i = 1;
        uploadFromFile = true;

        while(getline(input,str)){

            auto doc = nlp.parse(str);

            for (auto& token : doc.tokens())
            {
            auto head = token.head();
            auto children = token.children();
            if(isNounOrPron(token.pos_()) && 
            (token.dep_() == "obl" || token.dep_() == "obj" || token.dep_() == "nmod" || token.dep_() == "iobj" || token.dep_() == "conj") && 
            !(token.dep_() == "obj" && head.dep_() == "xcomp") && !(token.dep_() == "obl" && head.dep_() == "xcomp") &&
            !(token.dep_() == "obl" && head.dep_() == "acl") && !(token.dep_() == "obl" && head.dep_() == "advcl") &&
            !(token.dep_() == "conj" && head.dep_() == "obl") && !(token.dep_() == "obl" && head.dep_() == "ROOT") &&
            !(token.dep_() == "conj" && head.dep_() == "nsubj") && !(token.dep_() == "conj" && head.dep_() == "obl") &&
            !(token.dep_() == "conj" && head.dep_() == "obj")) {
                cout << i << " " << token.text() << " [" << token.pos_() << "] " << token.dep_() << " " << head.text() << " " << head.dep_() << " " << endl;
                i++;
            }
            } 
        }
    }

    input.close();
}


// Функция, которая отвечает за сохранение данных из структуры во внешний текстовый файл
bool saveToFile(Book*& bookList)
{
    system("cls");
    if (bookList == nullptr)
    {
        cout << "Сохранение в файл невозможно. В списке нет ни одной книги." << endl;
        return 1;
    }
    string fileName = checkingFileName(2); // Получить название выходного файла
    if (fileName == "=")
    {
        return 0;
    }

    ofstream fOut(fileName);
    if (!fOut.is_open())
    {
        cout << "Не удалось открыть файл." << endl;
        return 1;
    }

    Book* currBook = bookList; // Обновление указателя на текущий список книг
    // Запись всех книг в выходной текстовый файл
    while (currBook != nullptr)
    {
        fOut << '[' << currBook->title << ']';
        if (currBook->authors == nullptr) // Если у книги нет авторов
        {
            fOut << " Нет авторов для этой книги" << endl;
        }
        else
        {
            // Запись всех авторов книги в выходной файл
            Author* currAuthor = currBook->authors;
            while (currAuthor != nullptr)
            {
                fOut << " " << currAuthor->name;
                currAuthor = currAuthor->next;
            }
            fOut << endl;
        }
        currBook = currBook->next;
    }
    hasUnsavedChanges = false; // Сброс флага несохраненных изменений
    fOut.close();
}

// Функция, которая отвечает за сохранение данных перед выходом из функции
bool exitSave()
{
    bool validInput = false;
    char input;
    cout << "Хотите сохранить список книг перед выходом? (y/n)" << endl;
    cout << "Или введите символ ‘=’, для того чтобы вернуться в главное меню. " << endl;

    while (!validInput)
    {
        input = getchar(); // Получить выбор пользователя

        if (input == 'y' || input == 'Y' || input == 'Н' || input == 'н')
        {
            bool exit = saveToFile(bookList);
            if (exit == 0)
            {
                return 0;
            }
            break;
        }
        else if (input == 'n' || input == 'N' || input == 'Т' || input == 'т')
        {
            break;
        }
        else if (input == '=')
        {
            return 0;
        }
    }
    return 1;
}


int main()
{
    // Цикл меню
    bool validInput = false;
    char input;
    system("cls");
    printMenu(); // Вывод главного меню
    while (!validInput)
    {
        input = getchar();  // Считать символ с клавиатуры и игнорировать все клавиши кроме 1-8

        if (input == '1')
        {
            spacy(); // Добавить книгу в структуру
            system("pause");
            system("cls");
            printMenu();
            continue;
        }
        if (input == '2')
        {
            system("pause");
            system("cls");
            printMenu();
            continue;
        }
        if (input == '3')
        {
            system("pause");
            system("cls");
            printMenu();
            continue;
        }
        if (input == '4')
        {
            if (uploadFromFile == false)
            {
                break;
            }
            else
            {
                if (hasUnsavedChanges == true) // Если есть несохранные изменения в структуре
                {
                    system("cls");
                    bool exit = exitSave(bookList); // Сохранить список книг перед выходом из программы
                    if (exit)
                    {
                        validInput = true;
                        break;
                    }
                    else
                    {
                        system("cls");
                        printMenu();
                        continue;
                    }
                }
                else
                {
                    validInput = true;
                    break;
                }
            }
        }
    }
    return 0;
}
