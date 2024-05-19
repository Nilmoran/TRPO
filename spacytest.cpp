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

// Флаг для отслеживания несохраненных изменений 
bool hasUnsavedChanges = false;

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

        getline(cin, fileName);
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

        if (flag == true) {
            // Добавить расширение к названию файла, если оно отсутствует
            if (mode == 1) {
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
            
            if (mode == 2) {
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
                    do {
                        newFileName = baseName + "(" + to_string(count) + ")" + extension;
                        count++;
                    } while (fs::exists(newFileName));

                    // Отобразить путь до файла
                    path = fs::current_path().string() + "/" + newFileName;
                    cout << "Файл с таким названием уже существует, поэтому будет создан файл:\n " << path << endl;

                    return newFileName;
                } else 
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
    return (ch == '.' || ch == '!' || ch == '?' ||  ch == ';');
}

// Функция для разделения текста на предложения
// Возвращает вектор предложений
vector<string> splitSentences(string& text) 
{
    vector<string> sentences;
    istringstream sentenceStream(text);
    string sentence;

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
            } else 
            {
                // Ищем следующий знак завершения предложения
                endPos = sentence.find_first_of(".!?;", endPos + 1);
            }
        }

        // Добавляем последнее предложение (если есть)
        if (startPos < sentence.length()) 
        {
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

void prepareToVector(auto token, auto head, string& word, string& sentence, char c)
{
    word += token.text() + "; ";
    string sentenceBuf = sentence;
    string wordBuf;
    bool flagHighlight = true;
    sentence = "";
    stringstream iss(sentenceBuf);

    // Функция, которая проверяет, является ли символ знаком пунктуации
    auto isPunctuation = [](char ch) {
        return ispunct(static_cast<unsigned char>(ch));
    };

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
        if (cleanWord == token.text() && flagHighlight == true)
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

bool isPronoun(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "VERB" && (token.dep_() == "ROOT" || token.dep_() == "conj" 
    || token.dep_() == "parataxis" || token.dep_() == "xcomp")) 
    || (token.pos_() == "PART" && token.dep_() == "advmod"))
    && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "conj")) 
    || (head.pos_() == "ADJ" && head.dep_() == "ROOT") || (head.pos_() == "NOUN" && (head.dep_() == "ROOT" 
    || head.dep_() == "nsubj")))) 
    {
        prepareToVector(token, head, word, sentence, c); 
        return true;              
    }
    else
        return false;
}

bool isModifier(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "ADJ" && (token.dep_() == "amod" || token.dep_() == "conj")) || (token.pos_() == "DET" && token.dep_() == "det"))
    && (head.pos_() == "NOUN" && (head.dep_() == "nsubj" || head.dep_() == "obl" || head.dep_() == "obj" || head.dep_() == "conj" 
	|| head.dep_() == "nmod" || head.dep_() == "iobj" || head.dep_() == "ROOT")))
    {
        prepareToVector(token, head, word, sentence, c);
        return true;  
    }
    else
        return false;
}

bool isSubject(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "NOUN" && (token.dep_() == "nsubj" || token.dep_() == "nsubj:pass")) 
    || (token.pos_() == "PRON" && token.dep_() == "nsubj") || (token.pos_() == "PROPN" && token.dep_() == "nsubj")
	|| (token.pos_() == "NOUN" && token.dep_() == "conj") || (token.pos_() == "PROPN" && token.dep_() == "appos")) 
    && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "parataxis" || head.dep_() == "conj"))
    || (head.pos_() == "NOUN" && head.dep_() == "ROOT") || (head.pos_() == "NOUN" && head.dep_() == "nsubj")))
    {
        prepareToVector(token, head, word, sentence, c);
        return true;
    }
    else
        return false;
}

bool isAdverbial(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "NOUN" && (token.dep_() == "obl" || token.dep_() == "nmod")) 
    || (token.pos_() == "ADP" && token.dep_() == "case")
    || (token.pos_() == "ADV" && token.dep_() == "advmod")
    || (token.pos_() == "ADJ" && token.dep_() == "amod")) 
    && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "conj" || head.dep_() == "advcl")) 
    || (head.pos_() == "NOUN" && (head.dep_() == "obl" || head.dep_() == "nmod" || head.dep_() == "conj"))))
    {
        prepareToVector(token, head, word, sentence, c);
        return true;
    }
    else
        return false;
}

bool isObject(auto token, auto head, string& word, string& sentence, char c)
{
    if (((token.pos_() == "NOUN" && (token.dep_() == "obl" || token.dep_() == "obj" 
    || token.dep_() == "nmod" || token.dep_() == "conj" || token.dep_() == "iobj")) 
    || (token.pos_() == "ADP" && token.dep_() == "case")
    || (token.pos_() == "PRON" && (token.dep_() == "obj" || token.dep_() == "obl" || token.dep_() == "iobj"))) 
    && ((head.pos_() == "VERB" && (head.dep_() == "ROOT" || head.dep_() == "conj" || head.dep_() == "advcl" 
	|| head.dep_() == "xcomp" || head.dep_() == "parataxis")) 
    || (head.pos_() == "NOUN" && (head.dep_() == "ROOT" || head.dep_() == "conj" || head.dep_() == "obl" 
    || head.dep_() == "obj" || head.dep_() == "nmod" || head.dep_() == "nsubj"))))
    {
        prepareToVector(token, head, word, sentence, c);
        return true;
    }
    else
        return false;
}

// Функция, которая анализирует предложения из текстового файла  
void parceText(vector<Word>& words, ifstream& input, string str)
{
    
    // Перебор всех предложений из файла
    while (getline(input, str)) 
    {
        if (str == "\r")
            continue;
        auto doc = nlp.parse(str); // Парсер предобученной модели SpaCy
        string sentence = str;
        string subject, pronoun, modifier, object, adverbial = "";
        for (auto& token : doc.tokens()) // Перебор предложения по словам
        {
            auto head = token.head(); // Главное слово в словосочетании

            // Проверка является ли слово дополнением
            if(isObject(token, head, object, sentence, '/'))
                continue;
            // Проверка является ли слово обстоятельством
            if(isAdverbial(token, head, adverbial, sentence, '*'))
                continue;
            // Проверка является ли слово подлежащие
            if(isSubject(token, head, subject, sentence, '%'))
                continue;
            // Проверка является ли слово сказуемым
            if(isPronoun(token, head, pronoun, sentence, '#'))
                continue;
            // Проверка является ли слово определением
            if(isModifier(token, head, modifier, sentence, '$'))
                continue;

        }
        words.push_back(Word(subject, pronoun, modifier, object, adverbial, sentence));
    }
}

// Функция, которая анализирует предложения введенные с клавиатуры
void parceText(vector<Word>& words)
{
    string str = "";
    cout << "Введите символ ‘=’, для того чтобы вернуться в главное меню." << endl;
    // Перебор всех предложений, которые введет пользователь
    do 
    {
        if (str == "=")
            break;
        auto doc = nlp.parse(str); // Парсер предобученной модели SpaCy
        string sentence = str;
        string subject, pronoun, modifier, object, adverbial = "";
        for (auto& token : doc.tokens()) // Перебор предложения по словам
        {
            auto head = token.head(); // Главное слово в словосочетании

            // Проверка является ли слово дополнением
            if(isObject(token, head, object, sentence, '/'))
                continue;
            // Проверка является ли слово обстоятельством
            if(isAdverbial(token, head, adverbial, sentence, '*'))
                continue;
            // Проверка является ли слово подлежащие
            if(isSubject(token, head, subject, sentence, '%'))
                continue;
            // Проверка является ли слово сказуемым
            if(isPronoun(token, head, pronoun, sentence, '#'))
                continue;
            // Проверка является ли слово определением
            if(isModifier(token, head, modifier, sentence, '$'))
                continue;

        }
        words.push_back(Word(subject, pronoun, modifier, object, adverbial, sentence));
    }while (getline(cin, str));
}

// Функция, которая обеспечивает взаимодействие с текстовым файлом
void fileMain(vector<Word>& words) 
{
    string str, text;
    string fileName;
    int count = 0;
    fileName = checkingFileName(1); // Проверка введенного названия текстового файла
    if (fileName == "=") 
    {
        return;
    }

    fileName = processFile(fileName); // Создание буферного файла
    ifstream input(fileName); // Открытие и дальнейшая работа с буферным файлом
    if (input.is_open()) 
    {
        parceText(words, input, str); // Анализирование слов в буферном файле 
    }

    input.close();
    remove(fileName.c_str());
}

void printResults(vector<Word>& words, int mode) 
{
    system("clear");
    // Отобразить информацию о каждом предложении в векторе words
    for (auto& word : words) 
    {
        if(word.sentence.empty())
            continue;
        string str ="";
        switch (mode)
        {
        case 1:
            cout << "%Подлежащие%: " << word.subject << endl;
            cout << "#Сказуемое#: " << word.pronoun << endl;
            cout << "$Определение$: " << word.modifier << endl;
            cout << "/Дополнение/: " << word.object << endl;
            cout << "*Обстоятельство*: " << word.adverbial << endl;
            cout << "Предложение: " << word.sentence << endl;
            cout << "---------------------" << endl;
            break;
        case 2:
            if (word.subject.empty())
                continue;
            cout << "%Подлежащие%: " << word.subject << endl;
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '#' || str[i] == '$' || str[i] == '/'|| str[i] == '*')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            cout << "Предложение: " << str << endl;
            cout << "---------------------" << endl;
            break;        
        case 3:
            if (word.pronoun.empty())
                continue;
            cout << "#Сказуемое#: " << word.pronoun << endl;
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '%' || str[i] == '$' || str[i] == '/'|| str[i] == '*')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            cout << "Предложение: " << str << endl;
            cout << "---------------------" << endl;
            break;
        case 4:
            if (word.modifier.empty())
                continue;
            cout << "$Определение$: " << word.modifier << endl;
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '#' || str[i] == '%' || str[i] == '/'|| str[i] == '*')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            cout << "Предложение: " << str << endl;
            cout << "---------------------" << endl;
            break;
        case 5:
            if (word.object.empty())
                continue;
            cout << "/Дополнение/: " << word.object << endl;
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '#' || str[i] == '$' || str[i] == '%'|| str[i] == '*')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            cout << "Предложение: " << str << endl;
            cout << "---------------------" << endl;
            break;
        case 6:
            if (word.adverbial.empty())
                continue;
            cout << "*Обстоятельство*: " << word.adverbial << endl;           
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '#' || str[i] == '$' || str[i] == '/'|| str[i] == '%')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            cout << "Предложение: " << str << endl;
            cout << "---------------------" << endl;
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
// Возвращает истину после сохрананения данных в текстовый файл
bool saveToFile(vector<Word>& words, int mode) 
{
    system("clear");
    string fileName = checkingFileName(2); // Ввод названия выходного файла
    if (fileName == "=") {
        return false; // Если пользователь ввел хотел вернуться в главное меню
    }
    ofstream output(fileName);
    if (!output.is_open()) {
        cout << "Ошибка при открытии файла для записи." << endl;
        return false;
    }

    // Отобразить информацию о каждом предложении в векторе words 
    for (auto& word : words) 
    {
        if(word.sentence.empty())
            continue;
        string str = "";
        switch (mode)
        {
        case 1:
            output << "%Подлежащие%: " << word.subject << endl;
            output << "#Сказуемое#: " << word.pronoun << endl;
            output << "$Определение$: " << word.modifier << endl;
            output << "/Дополнение/: " << word.object << endl;
            output << "*Обстоятельство*: " << word.adverbial << endl;
            output << "Предложение: " << word.sentence << endl;
            output << "---------------------" << endl;
            break;
        case 2:
            if (word.subject.empty())
                continue;
            output << "%Подлежащие%: " << word.subject << endl;
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '#' || str[i] == '$' || str[i] == '/'|| str[i] == '*')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            output << "Предложение: " << str << endl;
            output << "---------------------" << endl;
            break;        
        case 3:
            if (word.pronoun.empty())
                continue;        
            output << "#Сказуемое#: " << word.pronoun << endl;
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '%' || str[i] == '$' || str[i] == '/'|| str[i] == '*')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            output << "Предложение: " << str << endl;
            output << "---------------------" << endl;
            break;
        case 4:
            if (word.modifier.empty())
                continue;        
            output << "$Определение$: " << word.modifier << endl;
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '#' || str[i] == '%' || str[i] == '/'|| str[i] == '*')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            output << "Предложение: " << str << endl;
            output << "---------------------" << endl;
            break;
        case 5:
            if (word.object.empty())
                continue;
            output << "/Дополнение/: " << word.object << endl;
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '#' || str[i] == '$' || str[i] == '%'|| str[i] == '*')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            output << "Предложение: " << str << endl;
            output << "---------------------" << endl;
            break;
        case 6:
            if (word.adverbial.empty())
                continue;
            output << "*Обстоятельство*: " << word.adverbial << endl;           
            str = word.sentence;
            for(size_t i = 0; i < str.length(); ++i)
            {
                if(str[i] == '#' || str[i] == '$' || str[i] == '/'|| str[i] == '%')
                {
                    str.erase(i, 1);
                    --i; // Уменьшаем i, чтобы не пропустить следующий символ
                }
            }
            output << "Предложение: " << str << endl;
            output << "---------------------" << endl;
            break;
        default:
            break;
        }
    }
    hasUnsavedChanges = false;
    cout << "Результаты сохранены в файл: " << fileName << endl;
    cout << "Нажмите клавишу Enter для продолжения." << endl;
    cin.get();
    output.close();
    return true;
}

// Функция, которая отвечает за сохранение данных перед выходом из программы
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
            bool exit = saveToFile(words, 1);
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

// Функция, которая отвечает за вывод основного меню
void printMenu() {
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
void printSubMenu() {
    cout << "Подменю: " << endl
        << "Для возвращения в основное меню введите символ '='" << endl
         << "1: Выделение в тексте всех членов предложения" << endl
         << "2: Выделение в тексте только подлежащих" << endl
         << "3: Выделение в тексте только сказуемых" << endl
         << "4: Выделение в тексте только определений" << endl
         << "5: Выделение в тексте только дополнение" << endl
         << "6: Выделение в тексте только обстоятельств" << endl;
}

// Функция обработки подменю
void subMenuForSaveFile(vector<Word>& words) {
    char subInput;
    bool subMenuActive = true; // Флаг для зацикливания подменю
    while (subMenuActive) {
        system("clear");
        printSubMenu();
        subInput = getchar();
        // Вызов функции сохранения в файл с разными режимами,
        // которые зависят от выбора пользователя
        switch (subInput) {
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
void subMenuForPrintResult(vector<Word>& words) {
    char subInput;
    bool subMenuActive = true;
    while (subMenuActive) {
        system("clear");
        printSubMenu();
        subInput = getchar();
        switch (subInput) {
            case '1':
                printResults(words, 1);
                break;
            case '2':
                printResults(words, 2);
                break;
            case '3':
                printResults(words, 3);
                break;
            case '4':
                printResults(words, 4);
                break;
            case '5':
                printResults(words, 5);
                break;
            case '6':
                printResults(words, 6);
                break;
            case '=':
                subMenuActive = false;
                break;
            default:
                break;
        }
    }
}

// Функция для разделения строки на слова
vector<string> split( string& s) {
    vector<string> tokens;
    istringstream iss(s);
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Функция для подсчета числа появлений каждого слова в каждой роли
void countWordRoles(vector<Word>& sentences) {
    map<string, map<string, int>> wordRoleCounts;

    string fileName = checkingFileName(2); // Ввод названия выходного файла
    if (fileName == "=") {
        return; // Если пользователь ввел хотел вернуться в главное меню
    }
    ofstream output(fileName);
    if (!output.is_open()) {
        cout << "Ошибка при открытии файла для записи." << endl;
        return;
    }

    for (auto& sentence : sentences) {
        vector<pair<string, string>> roles = {
            {sentence.subject, "подлежащее"},
            {sentence.pronoun, "сказуемое"},
            {sentence.modifier, "определение"},
            {sentence.object, "дополнение"},
            {sentence.adverbial, "обстоятельство"}
        };

        for (auto& role : roles) {
            if (!role.first.empty()) {
                vector<string> words = split(role.first);
                for ( auto& word : words) {
                    wordRoleCounts[word][role.second]++;
                }
            }
        }
    }

    // Вывод результатов
    for (auto& wordEntry : wordRoleCounts) {
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

int main() 
{
    vector<Word> words; // Вектор в котором хранятся все члены предложений и само предложение
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
                cin.clear();
                cin.ignore(1000, '\n'); // Очистка потока cin
                system("clear"); // Очистка консоли
                printMenu(); // Вывод меню
                break;
            case '2':
                words.clear(); // Очистка вектора
                system("clear");
                parceText(words);
                hasUnsavedChanges = true; // Установить несохраненные изменения
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                cin.clear();
                cin.ignore(1000, '\n'); // Очистка потока cin
                system("clear"); // Очистка консоли
                printMenu(); // Вывод меню
                break;
            case '3':
                if (words.empty())
                {
                    system("clear"); // Очистка консоли
                    cout << "Нет данных для дополнительного анализа." << endl;
                    cout << "Нажмите клавишу Enter для продолжения." << endl;
                    cin.clear();
                    cin.ignore(1000, '\n'); // Очистка потока cin
                    cin.get();
                    system("clear");
                    printMenu(); // Вывод меню
                }
                else
                {
                    system("clear");
                    countWordRoles(words);
                    cout << "Нажмите клавишу Enter для продолжения." << endl;
                    cin.clear();
                    cin.ignore(1000, '\n'); // Очистка потока cin
                    system("clear");
                    printMenu(); // Вывод меню
                }
                break;
            case '4':
                if (words.empty()) 
                { // Если вектор дополнений пустой
                    system("clear"); // Очистка консоли
                    cout << "Нет данных для сохранения в файл." << endl;
                    cout << "Нажмите клавишу Enter для продолжения." << endl;
                    cin.clear();
                    cin.ignore(1000, '\n'); // Очистка потока cin
                    cin.get();
                    system("clear");
                    printMenu(); // Вывод меню
                } 
                else 
                {
                    subMenuForSaveFile(words);
                    system("clear"); // Очистка консоли
                    printMenu(); // Вывод меню
                }
                break;
            case '5':
                if (words.empty()) { // Если вектор дополнений пустой
                    system("clear"); // Очистка консоли
                    cout << "Нет данных для отображения." << endl;
                    cout << "Нажмите клавишу Enter для продолжения." << endl;
                    cin.clear();
                    cin.ignore(1000, '\n'); // Очистка потока cin
                    cin.get();
                    system("clear");
                    printMenu(); // Вывод меню
                } 
                else 
                {
                    subMenuForPrintResult(words); // Вызов подменю
                    system("clear"); // Очистка консоли
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