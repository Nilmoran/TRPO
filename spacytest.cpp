#define SPACY_HEADER_ONLY
#include "spacy/spacy"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <cstdio>
#include <map>

using namespace std;
namespace fs = filesystem;

// Загрузка предобученной модели SpaCy
Spacy::Spacy spacy;
auto nlp = spacy.load("ru_core_news_lg");

// Флаг для отслеживания несохраненных изменений
bool hasUnsavedChanges = false;

struct memberOfSentence
{
    string subject; // Строка для хранения всех подлежащих из предложения
    string pronoun; // Строка для хранения всех сказуемых из предложения
    string modifier; // Строка для хранения всех определений из предложения
    string object; // Строка для хранения всех дополнений из предложения
    string adverbial; // Строка для хранения всех обстоятельств из предложения
    string sentence; // Предложение в котором определяются члены предложения 

    // Стандартный конструктор для структуры
    memberOfSentence(string& sub, string& pron, string& mod, string& obj, string& adv, string sent)
        : subject(sub), pronoun(pron), modifier(mod), object(obj), adverbial(adv), sentence(sent) {}
};

// Функция, которая отвечает за проверку названия файла на запрещенные символы
string checkingFileName(int);

// Функция, которая отвечает за определения символа конца предложения
bool isSentenceEnd(char);

// Функция, которая проверяет является ли слово сказуемым
bool isPronoun(auto, auto, string&, string&, char);

// Функция, которая проверяет является ли слово дополнением
bool isModifier(auto, auto, string&, string&, char);

// Функция, которая проверяет является ли слово подлежащим
bool isSubject(auto, auto, string&, string&, char);

// Функция, которая проверяет является ли слово обстоятельством
bool isAdverbial(auto, auto, string&, string&, char);

// Функция, которая проверяет является ли слово определением
bool isObject(auto, auto, string&, string&, char);

// Функция для разделения строки на слова
vector<string> split(string&);

// Функция для подсчета числа появлений каждого слова в каждой роли
void countWordRoles(vector<memberOfSentence>&);

// Функция для очистки предложения от лишних спецсимволов при отображении в консоли
void clearString(string, char, char, char, char);

// Функция для очистки предложения от лишних спецсимволов при сохранении в файл
void clearString(string, ofstream&, char, char, char, char);

// Функция, которая отвечает за отображение результатов анализа в консоли
void printResults(vector<memberOfSentence>&, int);

// Функция, которая отвечает за сохранение данных в текстовый файл
bool saveToFile(vector<memberOfSentence>&, int);

// Функция, которая отвечает за сохранение данных перед выходом из программы
bool exitSave(vector<memberOfSentence>&);

// Функция для обработки подменю при сохранении результатов анализа во внешний 
// текстовый файл, в зависимости от выбора пользователя
void subMenuForSaveFile(vector<memberOfSentence>&);

// Функция для обработки подменю при отображении результатов анализа в консоли,
// в зависимости от выбора пользователя
void subMenuForPrintResult(vector<memberOfSentence>&);

// Функция, которая отвечает за добавление слова в поле структуры и перезапись 
// предложения с выделением слова спецсимволами
void prepareToVector(string, string&, string&, char);

// Функция для разделения текста на предложения
vector<string> splitSentences(string&);

// Функция, которая создает буферный файл
string processFile(string&);

// Функция, которая анализирует предложения из текстового файла  
void parceText(vector<memberOfSentence>&, ifstream&, string);

// Функция, которая анализирует предложения введенные с клавиатуры
void parceText(vector<memberOfSentence>&);

// Функция, которая обеспечивает взаимодействие с текстовым файлом
void fileMain(vector<memberOfSentence>&);

// Функция, которая отвечает за вывод основного меню
void printMenu();

// Функция, которая отвечает за вывод подменю
void printSubMenu();

// Функция, которая очищает консоль
void cclear();

// Функция, которая отвечает за проверку названия файла на запрещенные символы
// Также функция выполняет проверку на наличие файла с таким же названием
// Возвращает корректное название файла
string checkingFileName(int mode)
{
    string path; // Динамический путь к файлу
    ifstream buf; // Буферный поток для проверки открытия
    string fileName; // Название файла
    bool flag; // Флаг корректного ввода
    do {
        cin.ignore(1000, '\n');
        cin.clear();
        flag = true;
        system("clear");
        if (mode == 1) // Если проверяется название входного файла
            cout << "Введите символ ‘=’, для того чтобы прекратить ввод названия входного файла и вернуться в меню." << endl;
        else // Если проверяется название выходного файла
            cout << "Введите символ ‘=’, для того чтобы прекратить ввод названия выходного файла и вернуться в меню." << endl;

        getline(cin, fileName); // Чтение названия файла
        if (fileName == "=")
        { // Выход в главное меню
            return fileName;
        }

        // Проверка на запрещенные символы
        for (int i = 0; i < fileName.size(); i++)
        {
            if ((fileName[i] == '\\') ||
                (fileName[i] == '/') ||
                (fileName[i] == ':') ||
                (fileName[i] == '*') ||
                (fileName[i] == '?') ||
                (fileName[i] == '"') ||
                (fileName[i] == '<') ||
                (fileName[i] == '|'))
            {
                cout << "Не корректный ввод, повторите попытку, не используя специальные символы!" << '\n';
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                flag = false;
                break;
            }
        }

        if (flag == true) 
        {
            // Добавить расширение к названию файла, если оно отсутствует
            if (mode == 1) 
            {
                if (fileName.find(".txt") == string::npos)
                {
                    fileName += ".txt";
                }
                // Задать динамический путь и проверить возможность открытия текстового файла
                path = fs::current_path().string() + "/" + fileName;
                buf.open(path);
                if (!buf.is_open()) {
                    cout << "Файл не найден проверьте его наличие по пути: " << path << endl;
                    cout << "Нажмите клавишу Enter для продолжения." << endl;
                    flag = false;
                }
            }

            if (mode == 2) 
            {
                // Добавить расширение к названию файла, если оно отсутствует
                if (fileName.find(".txt") == string::npos)
                {
                    fileName += ".txt";
                }

                // Если файл с таким названием уже существует добавить к названию индекс
                if (fs::exists(fileName))
                {
                    int count = 1;
                    string baseName, extension, newFileName;
                    size_t dotPos = fileName.find_last_of('.');

                    // Разделение названия файла на подстроки
                    if (dotPos != string::npos)
                    {
                        baseName = fileName.substr(0, dotPos);
                        extension = fileName.substr(dotPos);
                    }

                    // Поиск доступного индекса
                    do 
                    {
                        newFileName = baseName + "(" + to_string(count) + ")" + extension;
                        count++;
                    } while (fs::exists(newFileName));

                    // Отобразить путь до файла
                    path = fs::current_path().string() + "/" + newFileName;
                    cout << "Файл с таким названием уже существует, поэтому будет создан файл:\n " << path << endl;

                    return newFileName;
                }
                else
                {
                    // Отобразить путь до файла
                    path = fs::current_path().string() + "/" + fileName;
                    cout << "Текущие данные были записаны в файл по пути " << path << endl;
                    return fileName;
                }
            }
        }

    } while (flag == false);
    return fileName;
}

// Функция, которая отвечает за определения символа конца предложения
// Возвращает символ конца предложения
bool isSentenceEnd(char ch)
{
    // Знаки пунктуации, которые могут завершать предложение в русском языке
    return (ch == '.' || ch == '!' || ch == '?' || ch == ';');
}

// Функция, которая проверяет является ли слово сказуемым
// Возвращает истину если слово является сказуемым, в противном случае возвращает ложь
bool isPronoun(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "VERB" && (token.dep_() == "ROOT" || token.dep_() == "conj" || token.dep_() == "parataxis" 
        || token.dep_() == "xcomp")) || (token.pos_() == "PART" && token.dep_() == "advmod")) && ((head.pos_() == "VERB" 
        && (head.dep_() == "ROOT" || head.dep_() == "conj")) || (head.pos_() == "ADJ" && head.dep_() == "ROOT") 
        || (head.pos_() == "NOUN" && (head.dep_() == "ROOT" || head.dep_() == "nsubj"))))
    {
        // Добавление в структуру и выделение сказуемого в предложении
        prepareToVector(token.text(), word, sentence, c);
        return true;
    }
    else
        return false;
}

// Функция, которая проверяет является ли слово дополнением
// Возвращает истину если слово является дополнением, в противном случае возвращает ложь
bool isModifier(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "ADJ" && (token.dep_() == "amod" || token.dep_() == "conj")) || (token.pos_() == "DET" 
        && token.dep_() == "det")) && (head.pos_() == "NOUN" && (head.dep_() == "nsubj" || head.dep_() == "obl" 
        || head.dep_() == "obj" || head.dep_() == "conj" || head.dep_() == "nmod" || head.dep_() == "iobj" 
        || head.dep_() == "ROOT")))
    {
        // Добавление в структуру и выделение дополнения в предложении
        prepareToVector(token.text(), word, sentence, c);
        return true;
    }
    else
        return false;
}

// Функция, которая проверяет является ли слово подлежащим
// Возвращает истину если слово является подлежащим, в противном случае возвращает ложь
bool isSubject(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "NOUN" && (token.dep_() == "nsubj" || token.dep_() == "nsubj:pass"))
        || (token.pos_() == "PRON" && token.dep_() == "nsubj") || (token.pos_() == "PROPN" && token.dep_() == "nsubj")
        || (token.pos_() == "NOUN" && token.dep_() == "conj") || (token.pos_() == "PROPN" && token.dep_() == "appos"))
        && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "parataxis" || head.dep_() == "conj"))
        || (head.pos_() == "NOUN" && head.dep_() == "ROOT") || (head.pos_() == "NOUN" && head.dep_() == "nsubj")))
    {
        // Добавление в структуру и выделение подлежащего в предложении
        prepareToVector(token.text(), word, sentence, c);
        return true;
    }
    else
        return false;
}

// Функция, которая проверяет является ли слово обстоятельством
// Возвращает истину если слово является обстоятельством, в противном случае возвращает ложь
bool isAdverbial(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "NOUN" && (token.dep_() == "obl" || token.dep_() == "nmod"))
        || (token.pos_() == "ADP" && token.dep_() == "case") || (token.pos_() == "ADV" && token.dep_() == "advmod")
        || (token.pos_() == "ADJ" && token.dep_() == "amod")) && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" 
        || head.dep_() == "conj" || head.dep_() == "advcl")) || (head.pos_() == "NOUN" && (head.dep_() == "obl" 
        || head.dep_() == "nmod" || head.dep_() == "conj"))))
    {
        // Добавление в структуру и выделение обстоятельства в предложении
        prepareToVector(token.text(), word, sentence, c);
        return true;
    }
    else
        return false;
}

// Функция, которая проверяет является ли слово определением
// Возвращает истину если слово является определением, в противном случае возвращает ложь
bool isObject(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "NOUN" && (token.dep_() == "obl" || token.dep_() == "obj" || token.dep_() == "nmod" 
        || token.dep_() == "conj" || token.dep_() == "iobj")) || (token.pos_() == "ADP" && token.dep_() == "case") 
        || (token.pos_() == "PRON" && (token.dep_() == "obj" || token.dep_() == "obl" || token.dep_() == "iobj")))
        && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "conj" || head.dep_() == "advcl" 
        || head.dep_() == "xcomp" || head.dep_() == "parataxis")) || (head.pos_() == "NOUN" && (head.dep_() == "ROOT" 
        || head.dep_() == "conj" || head.dep_() == "obl" || head.dep_() == "obj" || head.dep_() == "nmod" 
        || head.dep_() == "nsubj"))))
    {
        // Добавление в структуру и выделение определения в предложении
        prepareToVector(token.text(), word, sentence, c);
        return true;
    }
    else
        return false;
}

// Функция для разделения строки на слова
vector<string> split(string& s)
{
    vector<string> tokens;
    // Представление строки в виде потока
    istringstream iss(s);
    string token;
    // Разделение строки по пробелам (стандартный разделитель при чтении с помощью оператора >>)
    while (iss >> token)
    {
        // Формирование вектора из разделенной строки
        tokens.push_back(token);
    }
    return tokens;
}

// Функция для подсчета числа появлений каждого слова в каждой роли
void countWordRoles(vector<memberOfSentence>& sentences) 
{
    map<string, map<string, int>> wordRoleCounts;

    string fileName = checkingFileName(2); // Ввод названия выходного файла
    if (fileName == "=") 
    {
        return; // Если пользователь ввел хотел вернуться в главное меню
    }

    ofstream output(fileName);
    // Проверка на открытие файла
    if (!output.is_open())
    {
        cout << "Ошибка при открытии файла для записи." << endl;
        return;
    }

    // Поэлементный перебор вектора
    for (auto& sentence : sentences)
    {
        // Вектор пар, где первый элемент пары слово, а второй его роль в предложении
        vector<pair<string, string>> roles =
        {
            {sentence.subject, "подлежащее"},
            {sentence.pronoun, "сказуемое"},
            {sentence.modifier, "определение"},
            {sentence.object, "дополнение"},
            {sentence.adverbial, "обстоятельство"}
        };

        // Подсчет числа встречаемости слова в определенной роли
        for (auto& role : roles)
        {
            if (!role.first.empty())
            {
                vector<string> words = split(role.first);
                for (auto& word : words)
                {
                    wordRoleCounts[word][role.second]++;
                }
            }
        }
    }

    // Вывод результатов
    for (auto& wordEntry : wordRoleCounts) 
    {
        output << "Слово: " << wordEntry.first << endl;

        auto subj = wordEntry.second.find("подлежащее");
        output << "В роли \"подлежащего\" - " << (subj != wordEntry.second.end() ? subj->second : 0) << endl;

        auto pred = wordEntry.second.find("сказуемое");
        output << "В роли \"сказуемого\" - " << (pred != wordEntry.second.end() ? pred->second : 0) << endl;

        auto mod = wordEntry.second.find("определение");
        output << "В роли \"определения\" - " << (mod != wordEntry.second.end() ? mod->second : 0) << endl;

        auto obj = wordEntry.second.find("дополнение");
        output << "В роли \"дополнения\" - " << (obj != wordEntry.second.end() ? obj->second : 0) << endl;

        auto adv = wordEntry.second.find("обстоятельство");
        output << "В роли \"обстоятельства\" - " << (adv != wordEntry.second.end() ? adv->second : 0) << endl;

        output << endl;
    }

}

// Функция для очистки предложения от лишних спецсимволов при отображении в консоли
void clearString(string str, char a, char b, char c, char d)
{
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == a || str[i] == b || str[i] == c || str[i] == d)
        {
            str.erase(i, 1);
            --i; // Уменьшаем i, чтобы не пропустить следующий символ
        }
    }
    // Выводим очищенное предложение
    cout << "Предложение: " << str << endl;
    cout << "---------------------" << endl;
}

// Функция для очистки предложения от лишних спецсимволов при сохранении в файл
void clearString(string str, ofstream& file, char a, char b, char c, char d)
{
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == a || str[i] == b || str[i] == c || str[i] == d)
        {
            str.erase(i, 1);
            --i; // Уменьшаем i, чтобы не пропустить следующий символ
        }
    }
    // Выводим очищенное предложение
    file << "Предложение: " << str << endl;
    file << "---------------------" << endl;
}

// Функция, которая отвечает за отображение результатов анализа в консоли
void printResults(vector<memberOfSentence>& words, int mode)
{
    system("clear");
    // Перебор каждого предложения в векторе words
    for (auto& word : words)
    {
        // Если была пустая строка в файле, то пропускаем её
        if (word.sentence.empty())
            continue;
        string str = "";
        // В зависимости от выбора пользователя отображаем нужные данные
        switch (mode)
        {
        case 1: // Отображение всех членов предложения
            cout << "%Подлежащие%: " << word.subject << endl;
            cout << "#Сказуемое#: " << word.pronoun << endl;
            cout << "$Определение$: " << word.modifier << endl;
            cout << "/Дополнение/: " << word.object << endl;
            cout << "*Обстоятельство*: " << word.adverbial << endl;
            cout << "Предложение: " << word.sentence << endl;
            cout << "---------------------" << endl;
            break;
        case 2: // Отображение только подлежащего
            if (word.subject.empty())
                continue;

            cout << "%Подлежащие%: " << word.subject << endl;
            // Очистка предложения  от спецсимволов, которыми выделяются другие члены предложения 
            clearString(word.sentence, '#', '$', '/', '*');

            break;
        case 3: // Отображение только сказуемого
            if (word.pronoun.empty())
                continue;

            cout << "#Сказуемое#: " << word.pronoun << endl;
            // Очистка предложения  от спецсимволов, которыми выделяются другие члены предложения 
            clearString(word.sentence, '%', '$', '/', '*');

            break;
        case 4: // Отображение только определения
            if (word.modifier.empty())
                continue;

            cout << "$Определение$: " << word.modifier << endl;
            // Очистка предложения  от спецсимволов, которыми выделяются другие члены предложения 
            clearString(word.sentence, '#', '%', '/', '*');

            break;
        case 5: // Отображение только дополнения
            if (word.object.empty())
                continue;

            cout << "/Дополнение/: " << word.object << endl;
            // Очистка предложения  от спецсимволов, которыми выделяются другие члены предложения 
            clearString(word.sentence, '#', '$', '%', '*');

            break;
        case 6: // Отображение только обстоятельства
            if (word.adverbial.empty())
                continue;

            cout << "*Обстоятельство*: " << word.adverbial << endl;
            // Очистка предложения  от спецсимволов, которыми выделяются другие члены предложения 
            clearString(word.sentence, '#', '$', '/', '%');

            break;
        default:
            break;
        }
    }

    cout << "Нажмите клавишу Enter для продолжения." << endl;
    cin.clear();
    cin.ignore(1000, '\n'); // Очистка потока cin    
    cin.get();
}

// Функция, которая отвечает за сохранение данных в текстовый файл
// Возвращает истину после сохранения данных в текстовый файл
bool saveToFile(vector<memberOfSentence>& words, int mode)
{
    system("clear");
    string fileName = checkingFileName(2); // Ввод названия выходного файла
    
    if (fileName == "=") 
    {
        return false; // Если пользователь ввел хотел вернуться в главное меню
    }

    ofstream output(fileName);
    // Проверка на открытие файла
    if (!output.is_open()) 
    {
        cout << "Ошибка при открытии файла для записи." << endl;
        return false;
    }

    // Перебор каждого предложения в векторе words
    for (auto& word : words)
    {
        if (word.sentence.empty())
            continue;

        string str = "";
        // В зависимости от выбора пользователя сохраняем нужные данные
        switch (mode)
        {
        case 1: // Сохранение всех членов предложения
            output << "%Подлежащие%: " << word.subject << endl;
            output << "#Сказуемое#: " << word.pronoun << endl;
            output << "$Определение$: " << word.modifier << endl;
            output << "/Дополнение/: " << word.object << endl;
            output << "*Обстоятельство*: " << word.adverbial << endl;
            output << "Предложение: " << word.sentence << endl;
            output << "---------------------" << endl;
            break;
        case 2: // Сохранение только подлежащего
            if (word.subject.empty())
                continue;

            output << "%Подлежащие%: " << word.subject << endl;
            // Очистка предложения от спецсимволов, которыми выделяются другие члены предложения
            clearString(word.sentence, output, '#', '$', '/', '*');

            break;
        case 3: // Сохранение только сказуемого
            if (word.pronoun.empty())
                continue;

            output << "#Сказуемое#: " << word.pronoun << endl;
            // Очистка предложения от спецсимволов, которыми выделяются другие члены предложения
            clearString(word.sentence, output, '%', '$', '/', '*');

            break;
        case 4: // Сохранение только определений
            if (word.modifier.empty())
                continue;

            output << "$Определение$: " << word.modifier << endl;
            // Очистка предложения от спецсимволов, которыми выделяются другие члены предложения
            clearString(word.sentence, output, '#', '%', '/', '*');

            break;
        case 5: // Сохранение только дополнений
            if (word.object.empty())
                continue;

            output << "/Дополнение/: " << word.object << endl;
            // Очистка предложения от спецсимволов, которыми выделяются другие члены предложения
            clearString(word.sentence, output, '#', '$', '%', '*');

            break;
        case 6: // Сохранение только обстоятельств
            if (word.adverbial.empty())
                continue;

            output << "*Обстоятельство*: " << word.adverbial << endl;
            // Очистка предложения от спецсимволов, которыми выделяются другие члены предложения
            clearString(word.sentence, output, '#', '$', '/', '%');

            break;
        default:
            break;
        }
    }

    // Снятие флага несохраненных изменений
    hasUnsavedChanges = false;
    cout << "Результаты сохранены в файл: " << fileName << endl;
    cout << "Нажмите клавишу Enter для продолжения." << endl;
    cin.get();
    output.close();
    return true;
}

// Функция, которая отвечает за сохранение данных перед выходом из программы
// Возвращает истину при выходе из программы
bool exitSave(vector<memberOfSentence>& words) 
{
    bool validInput = false; // Флаг корректного ввода 
    char input;
    cout << "Хотите сохранить данные перед выходом? (y/n)" << endl;
    cout << "Или введите символ ‘=’, для того чтобы вернуться в главное меню. " << endl;

    // Цикл в котором определяется нужно ли сохранение данных перед выходом из программы
    while (!validInput) 
    {
        input = getchar();
        if (input == 'y' || input == 'Y') 
        {
            bool exit = saveToFile(words, 1);
            if (!exit) 
            {
                return false; // Возврат в главное меню
            }
            break;
        }
        else if (input == 'n' || input == 'N') 
        {
            break;
        }
        else if (input == '=') 
        {
            return false; // Возврат в главное меню
        }
    }
    return true; // Завершение программы
}

// Функция для обработки подменю при отображении результатов анализа в консоли,
// в зависимости от выбора пользователя
void subMenuForSaveFile(vector<memberOfSentence>& words) 
{
    char subInput;
    bool subMenuActive = true; // Флаг для зацикливания подменю
    while (subMenuActive) 
    {
        system("clear");
        printSubMenu(); // Отображение подменю
        subInput = getchar();
        // Вызов функции сохранения в файл с разными режимами,
        // которые зависят от выбора пользователя
        switch (subInput) 
        {
        case '1':
            saveToFile(words, 1); // Сохранение всех членов предложения в файл
            break;
        case '2':
            saveToFile(words, 2); // Сохранение только подлежащих
            break;
        case '3':
            saveToFile(words, 3); // Сохранение только сказуемых
            break;
        case '4':
            saveToFile(words, 4); // Сохранение только определений
            break;
        case '5':
            saveToFile(words, 5); // Сохранение только дополнение
            break;
        case '6':
            saveToFile(words, 6); // Сохранение только обстоятельств
            break;
        case '=':
            subMenuActive = false;
            break;
        default:
            break;
        }
    }
}

// Функция обработки подменю
void subMenuForPrintResult(vector<memberOfSentence>& words) 
{
    char subInput;
    bool subMenuActive = true; // Флаг для зацикливания подменю
    while (subMenuActive) 
    {
        system("clear");
        printSubMenu(); // Отображение подменю
        subInput = getchar();
        switch (subInput) 
        {
        // Вызов функции отображения данных в консоли с разными режимами,
        // которые зависят от выбора пользователя
        case '1':
            printResults(words, 1); // Отображение всех членов предложения 
            break;
        case '2':
            printResults(words, 2); // Отображение только подлежащих
            break;
        case '3':
            printResults(words, 3); // Отображение только сказуемых
            break;
        case '4':
            printResults(words, 4); // Отображение только определений
            break;
        case '5':
            printResults(words, 5); // Отображение только дополнение
            break;
        case '6':
            printResults(words, 6); // Отображение только обстоятельств
            break;
        case '=':
            subMenuActive = false;
            break;
        default:
            break;
        }
    }
}

// Функция, которая отвечает за добавление слова в поле структуры и перезапись 
// предложения с выделением слова спецсимволами
void prepareToVector(string token, string& word, string& sentence, char c)
{
    // Формирование строки, которая состоит из всех слов, определенной роли из одного предложения
    word += token + "; ";
    // Буферная строка для того что бы не изменить оригинальное предложение раньше времени
    string sentenceBuf = sentence;
    // Буферная строка для записи в нее слов из предложения
    string wordBuf;
    bool flagHighlight = true;
    sentence = "";
    // Создаем поток из буферной строки
    stringstream iss(sentenceBuf);

    // Лямбда-функция, которая проверяет, является ли символ знаком пунктуации
    auto isPunctuation = [](char ch) 
    {
        return ispunct(static_cast<unsigned char>(ch));
    };

    // Разделение строки по пробелам (стандартный разделитель при чтении с помощью оператора >>)
    while (iss >> wordBuf)
    {
        // Сохраняем пунктуацию в отдельной строке
        string punctuation = "";
        for (int i = wordBuf.size() - 1; i >= 0; --i)
        {
            if (isPunctuation(wordBuf[i]))
            {
                punctuation = wordBuf[i] + punctuation;
            }
            else
            {
                break;
            }
        }

        // Убираем пунктуацию из слова
        string cleanWord = "";
        for (char ch : wordBuf)
        {
            if (!isPunctuation(ch))
            {
                cleanWord += ch;
            }
        }

        // Проверяем, является ли очищенное слово токеном
        if (cleanWord == token && flagHighlight == true)
        {
            // Если слово является токеном, добавляем его с обрамлением символами и сохраненной пунктуацией
            sentence += c + cleanWord + c + punctuation + " ";
            flagHighlight = false;
        }
        else
        {
            sentence += wordBuf + " ";
        }
    }
}

// Функция для разделения текста на предложения
// Возвращает вектор предложений
vector<string> splitSentences(string& text)
{
    vector<string> sentences;
    // Создаем поток из строки
    istringstream sentenceStream(text);
    string sentence;

    // Читаем из потока данные и записываем отдельные предложения в строку
    while (getline(sentenceStream, sentence))
    {
        size_t startPos = 0;
        size_t endPos = sentence.find_first_of(".!?;", startPos);

        while (endPos != string::npos) {
            // Если текущий знак завершает предложение, добавляем его и предложение в вектор
            if (isSentenceEnd(sentence[endPos]))
            {
                sentences.push_back(sentence.substr(startPos, endPos - startPos + 1));

                // Ищем начало следующего предложения
                startPos = endPos + 1;
                endPos = sentence.find_first_of(".!?;", startPos);
            }
            else
            {
                // Ищем следующий знак завершения предложения
                endPos = sentence.find_first_of(".!?;", endPos + 1);
            }
        }

        // Добавляем последнее предложение (если есть)
        if (startPos < sentence.length())
        {
            // Формируем вектор получившихся строк
            sentences.push_back(sentence.substr(startPos));
        }
    }

    return sentences;
}

// Функция, которая создает буферный файл
// Возвращает название буферного файла
string processFile(string& inputFileName)
{
    ifstream inputFile(inputFileName);
    string outputFileName = "buf_" + inputFileName; // Задаем название буферного файла
    ofstream outputFile(outputFileName);

    string line;

    while (getline(inputFile, line))
    {
        // Разделение строки на предложения
        vector<string> sentences = splitSentences(line);

        // Запись каждого предложения на отдельной строке в выходной файл
        for (string& sentence : sentences)
        {
            outputFile << sentence << endl;
        }
    }

    return outputFileName;
}

// Функция, которая анализирует предложения из текстового файла  
void parceText(vector<memberOfSentence>& words, ifstream& input, string str)
{
    // Перебор всех предложений из файла
    while (getline(input, str))
    {
        // Если пустая строка в файле, то пропускаем её 
        if (str == "\r")
            continue;

        auto doc = nlp.parse(str); // Парсер предобученной модели SpaCy
        string sentence = str;
        string subject, pronoun, modifier, object, adverbial = "";
        for (auto& token : doc.tokens()) // Перебор предложения по словам
        {
            auto head = token.head(); // Главное слово в словосочетании

            // Проверка является ли слово дополнением
            if (isObject(token, head, object, sentence, '/'))
                continue;
            // Проверка является ли слово обстоятельством
            if (isAdverbial(token, head, adverbial, sentence, '*'))
                continue;
            // Проверка является ли слово подлежащим
            if (isSubject(token, head, subject, sentence, '%'))
                continue;
            // Проверка является ли слово сказуемым
            if (isPronoun(token, head, pronoun, sentence, '#'))
                continue;
            // Проверка является ли слово определением
            if (isModifier(token, head, modifier, sentence, '$'))
                continue;

        }
        // Формируем вектор структур с заполненными полями
        words.push_back(memberOfSentence(subject, pronoun, modifier, object, adverbial, sentence));
    }
}

// Функция, которая анализирует предложения введенные с клавиатуры
void parceText(vector<memberOfSentence>& words)
{
    string str = "";
    cout << "Введите символ ‘=’, для того чтобы вернуться в главное меню." << endl;
    // Перебор всех предложений, которые введет пользователь
    do
    {
        // Если пользователь ввел символ '=', то вернуться в главное меню
        if (str == "=")
            break;

        auto doc = nlp.parse(str); // Парсер предобученной модели SpaCy
        string sentence = str;
        string subject, pronoun, modifier, object, adverbial = "";
        for (auto& token : doc.tokens()) // Перебор предложения по словам
        {
            auto head = token.head(); // Главное слово в словосочетании

            // Проверка является ли слово дополнением
            if (isObject(token, head, object, sentence, '/'))
                continue;
            // Проверка является ли слово обстоятельством
            if (isAdverbial(token, head, adverbial, sentence, '*'))
                continue;
            // Проверка является ли слово подлежащие
            if (isSubject(token, head, subject, sentence, '%'))
                continue;
            // Проверка является ли слово сказуемым
            if (isPronoun(token, head, pronoun, sentence, '#'))
                continue;
            // Проверка является ли слово определением
            if (isModifier(token, head, modifier, sentence, '$'))
                continue;

        }
        // Формируем вектор структур с заполненными полями
        words.push_back(memberOfSentence(subject, pronoun, modifier, object, adverbial, sentence));

    } while (getline(cin, str));
}

// Функция, которая обеспечивает взаимодействие с текстовым файлом
void fileMain(vector<memberOfSentence>& words)
{
    string str, text;
    string fileName;
    int count = 0;
    fileName = checkingFileName(1); // Проверка введенного названия входного текстового файла

    // Если пользователь ввел символ '=', то вернуться в главное меню
    if (fileName == "=")
    {
        return;
    }

    fileName = processFile(fileName); // Создание буферного файла
    ifstream input(fileName); // Открытие и дальнейшая работа с буферным файлом
    
    // Если поток с буферным файлом открыт
    if (input.is_open())
    {
        parceText(words, input, str); // Анализ всех предложений буферном файле 
    }

    input.close();
    // Удаление буферного файла
    remove(fileName.c_str());
}

// Функция, которая отвечает за вывод основного меню
void printMenu() 
{
    cout << "Данная программа предназначена для поиска членов предложений в тексте!" << endl;
    cout << "Выберите операцию: " << endl
         << "1: загрузить из файла текст для анализа" << endl
         << "2: ввести текст для анализа с клавиатуры" << endl
         << "3: выполнить дополнительный анализ о числе слов в определенной роли" << endl
         << "4: сохранить результаты анализа текста в файл" << endl
         << "5: вывести результат анализа в консоль" << endl
         << "6: выйти" << endl;
}

// Функция, которая отвечает за вывод подменю
void printSubMenu() 
{
    cout << "Подменю: " << endl
         << "Для возвращения в основное меню введите символ '='" << endl
         << "1: Выделение в тексте всех членов предложения" << endl
         << "2: Выделение в тексте только подлежащих" << endl
         << "3: Выделение в тексте только сказуемых" << endl
         << "4: Выделение в тексте только определений" << endl
         << "5: Выделение в тексте только дополнение" << endl
         << "6: Выделение в тексте только обстоятельств" << endl;
}

// Функция, которая очищает консоль
void cclear()
{
    cin.clear();
    cin.ignore(1000, '\n'); // Очистка потока cin
    system("clear"); // Очистка консоли
}

int main()
{
    vector<memberOfSentence> words; // Вектор структур в которой хранятся все члены предложений и само предложение
    bool validInput = false; // Флаг корректного ввода
    char input;
    setlocale(LC_ALL, "ru_RU.UTF-8"); // Установка кодировки utf-8 для консоли
    system("clear"); // Очистка консоли
    printMenu(); // Вывод меню
    while (!validInput)
    {
        input = getchar();
        switch (input)
        {
        case '1':
            words.clear(); // Очистка вектора
            fileMain(words); // Обработка входного текстового файла

            hasUnsavedChanges = true; // Установить несохраненные изменения
            cout << "Нажмите клавишу Enter для продолжения." << endl;

            cclear();
            printMenu(); // Вывод меню
            break;
        case '2':
            words.clear(); // Очистка вектора
            system("clear");
            parceText(words);

            hasUnsavedChanges = true; // Установить несохраненные изменения
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            
            cclear();
            printMenu(); // Вывод меню
            break;
        case '3':
            if (words.empty())
            {
                system("clear"); // Очистка консоли
                cout << "Нет данных для дополнительного анализа." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                
                cclear();
                system("clear");
                printMenu(); // Вывод меню
            }
            else
            {
                system("clear");
                countWordRoles(words);
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                
                cclear();
                printMenu(); // Вывод меню
            }
            break;
        case '4':
            if (words.empty())
            { // Если вектор дополнений пустой
                system("clear"); // Очистка консоли
                cout << "Нет данных для сохранения в файл." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                
                cclear();
                system("clear");
                printMenu(); // Вывод меню
            }
            else
            {
                subMenuForSaveFile(words);

                cclear();
                printMenu(); // Вывод меню
            }
            break;
        case '5':
            if (words.empty()) 
            { // Если вектор дополнений пустой
                system("clear"); // Очистка консоли
                cout << "Нет данных для отображения." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;

                cin.get();
                cclear();
                printMenu(); // Вывод меню
            }
            else
            {
                subMenuForPrintResult(words); // Вызов подменю

                cclear();
                printMenu(); // Вывод меню
            }
            break;
        case '6':
            if (hasUnsavedChanges == true)
            { // Если есть несохраненные изменения
                system("clear"); // Очистка консоли
                bool exit = exitSave(words); // Сохранение данных перед выходом из программы
                if (exit)
                {
                    validInput = true;
                }
                else
                {
                    system("clear");
                    printMenu(); // Вывод меню
                }
            }
            else
            {
                validInput = true;
            }
            break;
        default:
            break;
        }
    }
    return 0;
}