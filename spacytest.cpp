#define SPACY_HEADER_ONLY
#include "spacy/spacy"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>
#include <cstdio>

using namespace std;
namespace fs = filesystem;
// Флаг для отслеживания несохраненных изменений 
bool hasUnsavedChanges = false;
// Флаг для отслеживания был ли загружен файл
bool uploadFromFile = false;
// Флаг для отслеживания первого открытия файла
bool flagFirstOpen = false;

// Загрузка предобученной модели SpaCy
Spacy::Spacy spacy;
auto nlp = spacy.load("ru_core_news_lg");

// Структура для хранения слова и его атрибутов
struct Word {
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
void printResults(vector<Word>&);

// Функция, которая отвечает за сохранение данных в текстовый файл
bool saveToFile(vector<Word>&, string&);

// Функция, которая отвеает за сохранение данных перед выходом из программы
bool exitSave(vector<Word>&);

// Функция, которая отвечает за вывод меню
void printMenu() {
    cout << "Данная программа предназначена для поиска дополнений в тексте!" << endl;
    cout << "Выберите операцию: " << endl
         << "1: загрузить из файла текст для анализа " << endl
         << "2: сохранить результаты анализа текста в файл" << endl
         << "3: вывести результат анализа в консоль" << endl
         << "4: выйти" << endl;
}

// Функция, которая отвечает за проверку названия файла на запрещенные символы
// Также функция выполняет проверку на наличие файла с таким же названием
// Возвращает корректное название файла
string checkingFileName(int mode) {
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
            cout << "Введите символ ‘=’, для того чтобы прекратить ввод названия входного файла и вернуться в главное меню." << endl;
        else // Если проверяется название выходного файла
            cout << "Введите символ ‘=’, для того чтобы прекратить ввод названия выходного файла и вернуться в главное меню." << endl;

        getline(cin, fileName);
        if (fileName == "=") { // Выход в главное меню
            return fileName;
        }
        
        // Проверка на запрещенные символы
        for (int i = 0; i < fileName.size(); i++) {
            if ((fileName[i] == '\\') ||
                (fileName[i] == '/') ||
                (fileName[i] == ':') ||
                (fileName[i] == '*') ||
                (fileName[i] == '?') ||
                (fileName[i] == '"') ||
                (fileName[i] == '<') ||
                (fileName[i] == '|')) {
                cout << "Не корректный ввод, повторите попытку, не используя специальные символы!" << '\n';
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                flag = false;
                break;
            }
        }

        if (flag == true) {
            // Добавить расширение к названию файла, если оно отсутствует
            if (mode == 1) {
                if (fileName.find(".txt") == string::npos) {
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
            if (mode == 2) {
                // Добавить расширение к названию файла, если оно отсутствует
                if (fileName.find(".txt") == string::npos) {
                    fileName += ".txt";
                }

                // Если файл с таким названием уже существует добавить к названию индекс
                if (fs::exists(fileName)) {
                    int count = 1;
                    string baseName, extension, newFileName;
                    size_t dotPos = fileName.find_last_of('.');
                    
                    // Разделение названия файла на подстроки
                    if (dotPos != string::npos) {
                        baseName = fileName.substr(0, dotPos);
                        extension = fileName.substr(dotPos);
                    }

                    // Поиск доступного индекса
                    do {
                        newFileName = baseName + "(" + to_string(count) + ")" + extension;
                        count++;
                    } while (fs::exists(newFileName));

                    // Отобразить путь до файла
                    path = fs::current_path().string() + "/" + newFileName;
                    cout << "Файл с таким названием уже существует, поэтому будет создан файл:\n " << path << endl;

                    return newFileName;
                } else {
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
bool isSentenceEnd(char ch) {
    // Знаки пунктуации, которые могут завершать предложение в русском языке
    return (ch == '.' || ch == '!' || ch == '?' ||  ch == ';');
}

// Функция для разделения текста на предложения
// Возвращает вектор предложений
vector<string> splitSentences(string& text) {
    vector<string> sentences;
    istringstream sentenceStream(text);
    string sentence;

    while (getline(sentenceStream, sentence)) {
        size_t startPos = 0;
        size_t endPos = sentence.find_first_of(".!?;", startPos);

        while (endPos != string::npos) {
            // Если текущий знак завершает предложение, добавляем его и предложение в вектор
            if (isSentenceEnd(sentence[endPos])) {
                sentences.push_back(sentence.substr(startPos, endPos - startPos + 1));

                // Ищем начало следующего предложения
                startPos = endPos + 1;
                endPos = sentence.find_first_of(".!?;", startPos);
            } else {
                // Ищем следующий знак завершения предложения
                endPos = sentence.find_first_of(".!?;", endPos + 1);
            }
        }

        // Добавляем последнее предложение (если есть)
        if (startPos < sentence.length()) {
            sentences.push_back(sentence.substr(startPos));
        }
    }

    return sentences;
}

// Функция, которая создает буферный файл
// Возвращает название буферного файла
string processFile(string& inputFileName) {
    ifstream inputFile(inputFileName);
    string outputFileName = "buf_" + inputFileName; // Задаем название буферного файла
    ofstream outputFile(outputFileName);

    string line;

    while (getline(inputFile, line)) {
        // Разделение строки на предложения
        vector<string> sentences = splitSentences(line);

        // Запись каждого предложения на отдельной строке в выходной файл
        for (string& sentence : sentences) {
            outputFile << sentence << endl;
        }
    }

    return outputFileName;
}

// Функция, которая анализирует предложения по словам
void parceText(vector<Word>& words, ifstream& input, string str)
{
    // Перебор всех предложений из файла
    while (getline(input, str)) 
        {
            auto doc = nlp.parse(str); // Парсер предобученной модели SpaCy
            string sentence = str;
            string subject, pronoun, modifier, object, adverbial = "";

            for (auto& token : doc.tokens()) // Перебор предложения по словам
            {
                auto head = token.head(); // Главное слово в словосочетании
                // Проверка является ли слово сказуемым
                if (((token.pos_() == "VERB" && (token.dep_() == "ROOT" || token.dep_() == "conj" 
                || token.dep_() == "parataxis" || token.dep_() == "xcomp")) || (token.pos_() == "PART" && token.dep_() == "advmod"))
                && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "conj")) || (head.pos_() == "ADJ" && head.dep_() == "ROOT"))) 
                {
                    string pronounBuf = token.text();
                    pronoun += token.text()  + ", "; // подлежащие
                    // Выделение слова в предложении, добавив квадратные скобки
                    size_t wordStart = sentence.find(pronounBuf);
                    sentence = sentence.replace(wordStart, pronounBuf.length(), "[" + pronounBuf + "]");                    
                }

                // Проверка является ли слово определением
                else if (((token.pos_() == "ADJ" && token.dep_() == "amod") || (token.pos_() == "DET" && token.dep_() == "det"))
                && (head.pos_() == "NOUN" && (head.dep_() == "nsubj" || head.dep_() == "obl" || head.dep_() == "obj" || head.dep_() == "conj")))
                {
                    string modifierBuf = token.text();
                    modifier += token.text() + ", "; // определение
                    // Выделение слова в предложении, добавив квадратные скобки
                    size_t wordStart = sentence.find(modifierBuf);
                    sentence = sentence.replace(wordStart, modifierBuf.length(), "{" + modifierBuf + "}");
                }

                // Проверка является ли слово подлежащие
                else if (((token.pos_() == "NOUN" && (token.dep_() == "nsubj" || token.dep_() == "nsubj:pass")) 
                || (token.pos_() == "PRON" && token.dep_() == "nsubj") || (token.pos_() == "PROPN" && token.dep_() == "nsubj")) 
                && (head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "parataxis" || head.dep_() == "conj")
                || (head.pos_() == "ADJ" && head.dep_() == "ROOT")))
                {
                    string subjectBuf = token.text();
                    subject += token.text() + ", "; // подлежащие
                    // Выделение слова в предложении, добавив квадратные скобки
                    size_t wordStart = sentence.find(subjectBuf);
                    sentence = sentence.replace(wordStart, subjectBuf.length(), "(" + subjectBuf + ")");
                }

                // Проверка является ли слово обстоятельством
                else if (((token.pos_() == "NOUN" && (token.dep_() == "obl" || token.dep_() == "nmod")) 
                || (token.pos_() == "ADP" && token.dep_() == "case")
                || (token.pos_() == "ADV" && token.dep_() == "advmod")
                || (token.pos_() == "ADJ" && token.dep_() == "amod")) 
                && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "conj" || head.dep_() == "advcl")) 
                || (head.pos_() == "NOUN" && (head.dep_() == "obl" || head.dep_() == "nmod" || head.dep_() == "conj"))))
                {
                    string adverbialBuf = token.text();
                    adverbial += token.text() + ", "; // определение
                    // Выделение слова в предложении, добавив квадратные скобки
                    size_t wordStart = sentence.find(adverbialBuf);
                    sentence = sentence.replace(wordStart, adverbialBuf.length(), "/" + adverbialBuf + "/");
                }

                // Проверка является ли слово дополнением
                else if (((token.pos_() == "NOUN" && (token.dep_() == "obl" || token.dep_() == "obj" 
                || token.dep_() == "nmod" || token.dep_() == "conj")) 
                || (token.pos_() == "ADP" && token.dep_() == "case")
                || (token.pos_() == "PRON" && (token.dep_() == "obj" || token.dep_() == "obl"))) 
                && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "conj" || head.dep_() == "advcl")) 
                || (head.pos_() == "NOUN" && (head.dep_() == "ROOT" || head.dep_() == "conj" || head.dep_() == "obl" 
                || head.dep_() == "obj" || head.dep_() == "nmod" || head.dep_() == "nsubj"))
                || (head.pos_() == "ADJ" && head.dep_() == "ROOT")))
                {
                    string objectBuf = token.text();
                    object += token.text() + ", "; // определение
                    // Выделение слова в предложении, добавив квадратные скобки
                    size_t wordStart = sentence.find(objectBuf);
                    sentence = sentence.replace(wordStart, objectBuf.length(), "*" + objectBuf + "*");
                }

                else
                {
                    continue;
                }
            }
            words.push_back(Word(subject, pronoun, modifier, object, adverbial, sentence));
        }
}

// Функция, которая обеспечивает взаимодействие с текстовым файлом
void fileMain(vector<Word>& words) {
    string str, text;
    string fileName;
    int count = 0;
    fileName = checkingFileName(1); // Проверка введенного названия текстового файла
    if (fileName == "=") {
        return;
    }

    fileName = processFile(fileName); // Создание буферного файла
    ifstream input(fileName); // Открытие и дальнейшая работа с буферным файлом
    if (input.is_open()) {
        uploadFromFile = true;
        parceText(words, input, str); // Анализирование слов в буферном файле 
    }
    input.close();
}


void printResults(vector<Word>& words) {
    // Отобразить информацию о каждом предложении в векторе words
    for (const auto& word : words) {
        cout << "(Подлежащие): " << word.subject << endl;
        cout << "[Сказуемое]: " << word.pronoun << endl;
        cout << "{Определение}: " << word.modifier << endl;
        cout << "*Дополнение*: " << word.object << endl;
        cout << "/Обстоятельство/: " << word.adverbial << endl;
        cout << "Предложение: " << word.sentence << endl;
        cout << "---------------------" << endl;
    }
    cin.get();
}

// Функция, которая отвечает за сохранение данных в текстовый файл
// Возвращает истину после сохрананения данных в текстовый файл
bool saveToFile(vector<Word>& words, string& fileName) {
    system("clear");
    ofstream output(fileName);
    if (!output.is_open()) {
        cout << "Ошибка при открытии файла для записи." << endl;
        return false;
    }

    // Отобразить информацию о каждом предложении в векторе words 
    for (auto& word : words) {
        output << "(Подлежащие): " << word.subject << endl;
        output << "[Сказуемое]: " << word.pronoun << endl;
        output << "{Определение}: " << word.modifier << endl;
        output << "*Дополнение*: " << word.object << endl;
        output << "/Обстоятельство/: " << word.adverbial << endl;
        output << "Предложение: " << word.sentence << endl;
        output << "---------------------" << endl;
    }

    cout << "Результаты сохранены в файл: " << fileName << endl;
    output.close();
    return true;
}

// Функция, которая отвеает за сохранение данных перед выходом из программы
// Возвращает истину при выходе из программы
bool exitSave(vector<Word>& words) {
    bool validInput = false; // Флаг корректного ввода 
    char input;
    cout << "Хотите сохранить данные перед выходом? (y/n)" << endl;
    cout << "Или введите символ ‘=’, для того чтобы вернуться в главное меню. " << endl;

    // Цикл в котором определяется нужно ли сохранение данных перед выходом из программы
    while (!validInput) {
        input = getchar();
        if (input == 'y' || input == 'Y') { 
            string saveFileName = checkingFileName(2); // Проверка названия выходного файла
            if (saveFileName == "=") {
                return false; // Возврат в главное меню
            }
            bool exit = saveToFile(words, saveFileName);
            if (!exit) {
                return false; // Возврат в главное меню
            }
            break;
        } else if (input == 'n' || input == 'N') {
            break;
        } else if (input == '=') {
            return false; // Возврат в главное меню
        }
    }
    return true; // Завершение программы
}

int main() {
    vector<Word> words; // Вектор в котором хранятся дополнения
    bool validInput = false; // Флаг корректного ввода
    char input;
    setlocale(LC_ALL, "ru_RU.UTF-8"); // Установка кодировки utf-8 для консоли
    system("clear"); // Очистка консоли
    printMenu(); // Вывод меню
    while (!validInput) {
        input = getchar();
        if (input == '1') {
            words.clear(); // Очистка ветора дополнений
            fileMain(words); // Обработка входного текстового файла
            hasUnsavedChanges = true; // Установить несохранненные изменения
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear"); // Очистка консоли
            printMenu(); // Вывод меню
            continue;
        }
        if (input == '2') {
            if (words.empty() == 1) // Если вектор дополнений пустой
            {
                system("clear"); // Очистка консоли
                cout << "Нет данных для сохранения в файл." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                cin.clear();
                cin.ignore(1000, '\n'); // Очистка потока cin
                cin.get();
                system("clear");
                printMenu(); // Вывод меню
                continue;
            }
            else
            {
                string saveFileName = checkingFileName(2); // Ввод названия выходного файла
                if (saveFileName == "=") { // Если пользователь ввел хотел вернуться в главное меню
                    system("clear");
                    printMenu(); // Вывод меню
                    continue;
                }

                if (saveToFile(words, saveFileName)) {
                    hasUnsavedChanges = false; // Снимаем флаг несохраненных изменений
                }
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                cin.get();
                system("clear"); // Очистка консоли
                printMenu(); // Вывод меню
                continue;
            }

        }
        if (input == '3') {
            if (words.empty() == 1) // Если вектор дополнений пустой
            {
                system("clear"); // Очистка консоли
                cout << "Нет данных для отображения." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                cin.clear();
                cin.ignore(1000, '\n'); // Очистка потока cin
                cin.get();
                system("clear");
                printMenu(); // Вывод меню
                continue;
            }
            printResults(words); // Отобразить дополнения
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear"); // Очистка консоли
            printMenu(); // Вывод меню
            continue;
        }
        if (input == '4') {
            if (uploadFromFile == false) { // Если не было загрузки из файла
                break;
            } else {
                if (hasUnsavedChanges == true) { // Если есть несохраненные изменения
                    system("clear"); // Очистка консоли
                    bool exit = exitSave(words); // Сохранение данных перед выходом из программы
                    if (exit) {
                        validInput = true;
                        break;
                    } else {
                        system("clear");
                        printMenu(); // Вывод меню
                        continue;
                    }
                } else {
                    validInput = true;
                    break;
                }
            }
        }
    }
    return 0;
}