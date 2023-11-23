#define SPACY_HEADER_ONLY
#include "spacy/spacy"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>

using namespace std;

namespace fs = std::filesystem;
bool hasUnsavedChanges = false;
bool uploadFromFile = false;

struct WordInfo {
    std::string word;
    int count;

    WordInfo() : count(0) {}
};

void printMenu()
{
    cout << "Выберите операцию: " << endl
        << "1: загрузить текст из файла" << endl
        << "2: сохранить результаты в файл" << endl
        << "3: вывести результат на консоль" << endl
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
        cin.ignore(1000, '\n');
        cin.clear();
        flag = true;
        system("clear");
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

bool isNounOrPron(string tokenPos) { // Функция проверяющая является ли слово существительным или местоимением
    if (tokenPos == "NOUN" || tokenPos == "PRON" || tokenPos == "PROPN") {
        return true;
    }
    else {
        return false;
    }
}

void spacy(std::map<std::string, WordInfo>& wordMap)
{
    string str, text;
    string fileName;
    fileName = checkingFileName(1); // Получить название входного файла
    if (fileName == "=")
    {
        return;
    }
    ifstream input(fileName);
    Spacy::Spacy spacy;
    auto nlp = spacy.load("ru_core_news_lg");
    bool flagObjectiv = false;
    if (input.is_open()) {
        cout << "Файл успешно открыт!" << endl;
        uploadFromFile = true;

        while (getline(input, str))
        {
            auto doc = nlp.parse(str);

            for (auto& token : doc.tokens())
            {
                auto head = token.head();
                auto children = token.children();
                if (isNounOrPron(token.pos_()) &&
                    (token.dep_() == "obl" || token.dep_() == "obj" || token.dep_() == "nmod" || token.dep_() == "iobj" || token.dep_() == "conj") &&
                    !(token.dep_() == "obj" && head.dep_() == "xcomp") && !(token.dep_() == "obl" && head.dep_() == "xcomp") &&
                    !(token.dep_() == "obl" && head.dep_() == "acl") && !(token.dep_() == "obl" && head.dep_() == "advcl") &&
                    !(token.dep_() == "conj" && head.dep_() == "obl") && !(token.dep_() == "obl" && head.dep_() == "ROOT") &&
                    !(token.dep_() == "conj" && head.dep_() == "nsubj") && !(token.dep_() == "conj" && head.dep_() == "obl") &&
                    !(token.dep_() == "conj" && head.dep_() == "obj")) 
                {
                    std::string word = token.text();
                    flagObjectiv = true;
                    // Поиск слова в map и обновление счетчика
                    wordMap[word].count++;
                }
            }
        }
    }
    if (flagObjectiv == true)
    {
        cout << "Дополнения обнаружены в файле!" << endl;
        hasUnsavedChanges = true;
    }
    else
    {
        cout << "Дополнения не обнаружены в файле!" << endl;
    }

    input.close();
}

void printWordInfo(const std::map<std::string, WordInfo>& wordMap)
{
    cout << "Слова и их количество:" << endl;

    for (const auto& pair : wordMap)
    {
        cout << pair.first << ": " << pair.second.count << endl;
    }
    cin.get();
}

// Функция, которая отвечает за сохранение данных из структуры во внешний текстовый файл
bool saveToFile(const std::map<std::string, WordInfo>& wordMap, const std::string& fileName)
{
    system("clear");
    ofstream output(fileName);
    if (!output.is_open())
    {
        cout << "Ошибка при открытии файла для записи." << endl;
        return false;
    }

    for (const auto& pair : wordMap)
    {
        output << pair.first << ": " << pair.second.count << endl;
    }

    cout << "Результаты сохранены в файл: " << fileName << endl;
    output.close();
    return true;
}

// Функция, которая отвечает за сохранение данных перед выходом из функции
bool exitSave(const std::map<std::string, WordInfo>& wordMap)
{
    bool validInput = false;
    char input;
    cout << "Хотите сохранить список книг перед выходом? (y/n)" << endl;
    cout << "Или введите символ ‘=’, для того чтобы вернуться в главное меню. " << endl;

    while (!validInput)
    {
        input = getchar(); // Получить выбор пользователя

        if (input == 'y' || input == 'Y')
        {
            string saveFileName = checkingFileName(2);
            if(saveFileName == "=")
            {
                return 0;
            }
            bool exit = saveToFile(wordMap, saveFileName);
            if (exit == 0)
            {
                return 0;
            }
            break;
        }
        else if (input == 'n' || input == 'N')
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
    std::map<std::string, WordInfo> wordMap;
    // Цикл меню
    bool validInput = false;
    char input;
    system("clear");
    printMenu(); // Вывод главного меню

    while (!validInput)
    {
        input = getchar();
        if (input == '1')
        {
            spacy(wordMap); // Вызвать функцию парсинга текста
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear");
            printMenu();
            continue;
        }
        if (input == '2') 
        {
            string saveFileName = checkingFileName(2);
            if(saveFileName == "=")
            {
                system("clear");
                printMenu();
                continue;
            }
            
            if (saveToFile(wordMap, saveFileName)) // Вызвать функцию вывода найденных слов в файл
            {
                hasUnsavedChanges = false;
            }
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get(); 
            system("clear");
            printMenu();
            continue;
        }
        if (input == '3') 
        {
            printWordInfo(wordMap); // Вывод найденных слов в консоль
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear");
            printMenu();
            continue;
        }
        if (input == '4') // Выход их программы 
        {
            if (uploadFromFile == false)
            {
                break;
            }
            else
            {
                if (hasUnsavedChanges == true) // Если есть несохранные изменения в программе 
                {
                    system("clear");
                    bool exit = exitSave(wordMap); // Сохранить список слов перед выходом из программы
                    if (exit)
                    {
                        validInput = true;
                        break;
                    }
                    else
                    {
                        system("clear");
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