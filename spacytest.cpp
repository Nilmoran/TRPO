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
    string word; // Слово
    string sentence; // Предложение в котором встречается слово
    int count; // Счетчик того сколько раз встретилось слово

    Word(string& w, string& s) : word(w), sentence(s), count(1) {} // Стандартный конструктор для структуры
};

// Функция, которая отвечает за вывод меню
void printMenu();

// Функция, которая выделяет найденное слово в предложении
string highlightWord(string&, string&);

// Функция, которая выполняет поиска слова в векторе words
int findWordIndex(vector<Word>&, string&);

// Функция, которая отвечает за проверку названия файла на запрещенные символы
string checkingFileName(int);

// Функция, которая проверяет является ли слово существительным или местоимением
bool isNounOrPron(string);

// Функция, которая проверяет является ли слово дополнением
bool isObject(string);

// Функция, которая проверяет не является ли слово обстоятельством
bool isNotAdverbial(string, string);

// Функция, которая проверяет не является ли слово подлежащим
bool isNotSubject(string, string);

// Функция, которая отвечает за определения символа конца предложения
bool isSentenceEnd(char ch);

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

// Функция, которая выделяет найденное слово в предложении
// Возвращает предложение с выделенным словом
string highlightWord(string& targetSentence, string& targetWord) {
    // Найти индекс начала и конца слова в тексте
    size_t wordStart = targetSentence.find(targetWord);
    size_t wordEnd = wordStart + targetWord.length();

    // Определить границы предложения, в котором находится слово
    size_t sentenceStart = targetSentence.rfind('.', wordStart);
    size_t sentenceEnd = targetSentence.find('.', wordEnd);

    // Обработать случай, когда нет точек перед/после слова
    if (sentenceStart == string::npos) {
        sentenceStart = 0;
    } else {
        sentenceStart++;
    }

    // Если конец предложения не найден, задать его длиной предложения
    if (sentenceEnd == string::npos) {
        sentenceEnd = targetSentence.length();
    }

    // Предложение с учетом слова в квадратных скобках
    string sentence = "[" + targetSentence.substr(sentenceStart, sentenceEnd - sentenceStart) + "]";

    return sentence;
}

// Функция для поиска слова в векторе words
// Возвращает индекс слова в векторе
int findWordIndex(vector<Word>& words, string& word) {
    // Перебор вектора words в поисках нужного слова
    for (size_t i = 0; i < words.size(); ++i) {
        if (words[i].word == word) {
            // Вернуть индекс слова в векторе, если оно найдено
            return static_cast<int>(i);
        }
    }
    // Вернуть индекс -1, означающий, что слова нет в векторе
    return -1;
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

// Функция, которая проверяет является ли слово существительным или местоимением
// Возвращает истину, если ли слово является существительным или местоимением
bool isNounOrPron(string tokenPos) {
    if (tokenPos == "NOUN" || tokenPos == "PRON" || tokenPos == "PROPN") {
        return true; // Если слово являет существительным или местоимением
    } else {
        return false;
    }
}

// Функция, которая проверяет является ли слово дополнением
// Возвращает истину, если ли слово является дополнением
bool isObject(string tokenDep) 
{
    if ((tokenDep == "obl" || tokenDep == "obj" || tokenDep == "nmod" || tokenDep == "iobj" || tokenDep == "conj")) 
    {
        return true;
    } 

    else 
    {
        return false;
    }
}

// Функция, которая проверяет не является ли слово обстоятельством
// Возвращает истину, если ли слово не явялется обстоятельством
bool isNotAdverbial(string tokenDep, string headDep) 
{
    if (!(tokenDep == "obl" && headDep == "ROOT") && !(tokenDep == "obj" && headDep == "xcomp") &&
    !(tokenDep == "obl" && headDep == "xcomp") && !(tokenDep == "obl" && headDep == "acl") && 
    !(tokenDep == "obl" && headDep == "advcl")) 
    {
        return true;
    } 

    else 
    {
        return false;
    }
}

// Функция, которая проверяет не является ли слово подлежащим
// Возвращает истину, если слово не ялвялется подлежащим
bool isNotSubject(string tokenDep, string headDep) 
{
    if (!(tokenDep == "conj" && headDep == "obl")  && !(tokenDep == "conj" && headDep == "nsubj") && 
    !(tokenDep== "conj" && headDep == "obl") && !(tokenDep == "conj" && headDep == "obj")) 
    {
        return true;
    } 

    else 
    {
        return false;
    }
}

// Функция, которая анализирует предложения по словам
void parceText(vector<Word>& words, ifstream& input, string str)
{
    // Перебор всех предложений из файла
    while (getline(input, str)) 
        {
            auto doc = nlp.parse(str); // Парсер предобученной модели SpaCy
            for (auto& token : doc.tokens()) // Перебор предложения по словам
            {
                auto head = token.head(); // Главное слово в словосочетании
                
                // Проверка является ли слово доплнением
                if (isNounOrPron(token.pos_()) && isObject(token.dep_()) && isNotAdverbial(token.dep_(), head.dep_()) &&
                isNotSubject(token.dep_(), head.dep_())) 
                {
                    string word = token.text(); // Дополнение
                    // Выделение слова в предложении, добавив квадратные скобки
                    size_t wordStart = str.find(word);
                    size_t wordEnd = wordStart + word.length();
                    string sentence = str;
                    sentence = sentence.replace(wordStart, word.length(), "[" + word + "]");
                    int index = findWordIndex(words, word);
                    if (index != -1) 
                    {
                        // Слово уже встречалось, увеличить счетчик и обновить предложение, если оно новое
                        words[index].count++;
                        if (sentence != words[index].sentence) 
                        {
                            words[index].sentence += sentence;
                        }
                    } 
                    else 
                    {
                        // Слово встречается впервые, добавить его в массив
                        words.push_back(Word(word, sentence));
                    }
                }
            }
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
    remove(fileName.c_str()); // Удаление буферного файла
}

// Функция для сравнения двух объектов Word для сортировки
bool compareWords(const Word& a, const Word& b) {
    return a.word < b.word;
}

void printResults(std::vector<Word>& words) {
    // Сортировка вектора по полю word.word
    std::sort(words.begin(), words.end(), compareWords);

    // Отобразить информацию о каждом слове в отсортированном векторе words
    for (const auto& word : words) {
        std::cout << "Слово: " << word.word << std::endl;
        std::cout << "Предложение: " << word.sentence << std::endl;
        std::cout << "Количество: " << word.count << std::endl;
        std::cout << "---------------------" << std::endl;
    }
    std::cin.get();
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

    // Сортировка вектора по полю word.word
    std::sort(words.begin(), words.end(), compareWords);

    // Отобразить информацию о каждом слове в векторе words 
    for (auto& word : words) {
        output << "Слово: " << word.word << endl;
        output << "Предложение: " << word.sentence << endl;
        output << "Количество: " << word.count << endl;
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
    system("clear"); // Отчистка консоли
    printMenu(); // Вывод меню
    while (!validInput) {
        input = getchar();
        if (input == '1') {
            words.clear(); // Отчистка ветора дополнений
            fileMain(words); // Обработка входного текстового файла
            hasUnsavedChanges = true; // Установить несохранненные изменения
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear"); // Отчистка консоли
            printMenu(); // Вывод меню
            continue;
        }
        if (input == '2') {
            if (words.empty() == 1) // Если вектор дополнений пустой
            {
                system("clear"); // Отчистка консоли
                cout << "Нет данных для сохранения в файл." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                cin.clear();
                cin.ignore(1000, '\n'); // Отчистка потоко cin
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
                system("clear"); // Отчистка консоли
                printMenu(); // Вывод меню
                continue;
            }

        }
        if (input == '3') {
            if (words.empty() == 1) // Если вектор дополнений пустой
            {
                system("clear"); // Отчистка консоли
                cout << "Нет данных для отображения." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                cin.clear();
                cin.ignore(1000, '\n'); // Отчистка потока cin
                cin.get();
                system("clear");
                printMenu(); // Вывод меню
                continue;
            }
            printResults(words); // Отобразить дополнения
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear"); // Отчистка консоли
            printMenu(); // Вывод меню
            continue;
        }
        if (input == '4') {
            if (uploadFromFile == false) { // Если не было загрузки из файла
                break;
            } else {
                if (hasUnsavedChanges == true) { // Если есть несохраненные изменения
                    system("clear"); // Отчистка консоли
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