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
bool hasUnsavedChanges = false;
bool uploadFromFile = false;
bool flagFirstOpen = false;

Spacy::Spacy spacy;
auto nlp = spacy.load("ru_core_news_lg");

struct Word {
    string word;
    string sentence;
    int count;

    Word(string& w, string& s) : word(w), sentence(s), count(1) {}
};

void printMenu();
string createNumberedBufferFile(string&);
string checkingFileName(int);
bool isNounOrPron(string);
void spacyMain(vector<Word>&);
void printResults(vector<Word>&);
bool saveToFile(vector<Word>&, string&);
bool exitSave(vector<Word>& );

void printMenu() {
    cout << "Выберите операцию: " << endl
         << "1: загрузить текст из файла" << endl
         << "2: сохранить результаты в файл" << endl
         << "3: вывести результат на консоль" << endl
         << "4: выйти" << endl;
}

string getSurroundingSentence(string& fullText, string& targetWord) {
    // Найти индекс начала и конца слова в тексте
    size_t wordStart = fullText.find(targetWord);
    size_t wordEnd = wordStart + targetWord.length();

    // Определить границы предложения, в котором находится слово
    size_t sentenceStart = fullText.rfind('.', wordStart);
    size_t sentenceEnd = fullText.find('.', wordEnd);

    // Обработать случай, когда нет точек перед/после слова
    if (sentenceStart == string::npos) {
        sentenceStart = 0;
    } else {
        sentenceStart++;
    }

    if (sentenceEnd == string::npos) {
        sentenceEnd = fullText.length();
    }

    // Выделить предложение с учетом слова в квадратных скобках
    string sentence = "[" + fullText.substr(sentenceStart, sentenceEnd - sentenceStart) + "]";

    return sentence;
}

// Функция для поиска слова в массиве words
int findWordIndex(vector<Word>& words, string& word) {
    for (size_t i = 0; i < words.size(); ++i) {
        if (words[i].word == word) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

string checkingFileName(int mode) {
    string path;
    ifstream buf;
    string fileName;
    bool flag;
    do {
        cin.ignore(1000, '\n');
        cin.clear();
        flag = true;
        system("clear");
        if (mode == 1)
            cout << "Введите символ ‘=’, для того чтобы прекратить ввод названия входного файла и вернуться в главное меню." << endl;
        else
            cout << "Введите символ ‘=’, для того чтобы прекратить ввод названия выходного файла и вернуться в главное меню." << endl;

        getline(cin, fileName);
        if (fileName == "=") {
            return fileName;
        }

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
            if (mode == 1) {
                if (fileName.find(".txt") == string::npos) {
                    fileName += ".txt";
                }
                path = fs::current_path().string() + "/" + fileName;
                buf.open(path);
                if (!buf.is_open()) {
                    cout << "Файл не найден проверьте его наличие по пути: " << path << endl;
                    cout << "Нажмите клавишу Enter для продолжения." << endl;
                    flag = false;
                }
            }
            if (mode == 2) {
                if (fileName.find(".txt") == string::npos) {
                    fileName += ".txt";
                }
                if (fs::exists(fileName)) {
                    int count = 1;
                    string baseName, extension, newFileName;
                    size_t dotPos = fileName.find_last_of('.');
                    if (dotPos != string::npos) {
                        baseName = fileName.substr(0, dotPos);
                        extension = fileName.substr(dotPos);
                    }

                    do {
                        newFileName = baseName + "(" + to_string(count) + ")" + extension;
                        count++;
                    } while (fs::exists(newFileName));
                    path = fs::current_path().string() + "/" + newFileName;
                    cout << "Файл с таким названием уже существует, поэтому будет создан файл:\n " << path << endl;

                    return newFileName;
                } else {
                    path = fs::current_path().string() + "/" + fileName;
                    cout << "Текущие данные были записаны в файл по пути " << path << endl;
                    return fileName;
                }
            }
        }

    } while (flag == false);
    return fileName;
}

bool isNounOrPron(string tokenPos) {
    if (tokenPos == "NOUN" || tokenPos == "PRON" || tokenPos == "PROPN") {
        return true;
    } else {
        return false;
    }
}

// Функция для разделения текста на предложения
vector<string> splitSentences(string& text) {
    vector<string> sentences;
    istringstream sentenceStream(text);
    string sentence;

    while (getline(sentenceStream, sentence, '.')) {
        if (!sentence.empty()) {
            sentences.push_back(sentence);
        }
    }

    return sentences;
}

// Функция для обработки файла
string processFile(string& inputFileName) {
    ifstream inputFile(inputFileName);
    string outputFileName = "buf_" + inputFileName;
    ofstream outputFile(outputFileName);

    string line;
    while (getline(inputFile, line)) {
        // Разделение строки на предложения
        vector<string> sentences = splitSentences(line);

        // Запись каждого предложения на отдельной строке в выходной файл
        for (string& sentence : sentences) {
            outputFile << sentence + "." << endl;
        }
    }

    return outputFileName;
}

void parceText(vector<Word>& words, ifstream& input, string str)
{
    while (getline(input, str)) 
        {
            auto doc = nlp.parse(str);
            for (auto& token : doc.tokens()) 
            {
                auto head = token.head();
                if (isNounOrPron(token.pos_()) &&
                (token.dep_() == "obl" || token.dep_() == "obj" || token.dep_() == "nmod" || token.dep_() == "iobj" || token.dep_() == "conj") &&
                !(token.dep_() == "obj" && head.dep_() == "xcomp") && !(token.dep_() == "obl" && head.dep_() == "xcomp") &&
                !(token.dep_() == "obl" && head.dep_() == "acl") && !(token.dep_() == "obl" && head.dep_() == "advcl") &&
                !(token.dep_() == "conj" && head.dep_() == "obl") && !(token.dep_() == "obl" && head.dep_() == "ROOT") &&
                !(token.dep_() == "conj" && head.dep_() == "nsubj") && !(token.dep_() == "conj" && head.dep_() == "obl") &&
                !(token.dep_() == "conj" && head.dep_() == "obj")) 
                {
                    string word = token.text();
                    // Заменить слово в самой строке, добавив квадратные скобки
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

void spacyMain(vector<Word>& words) {
    string str, text;
    string fileName;
    int count = 0;
    fileName = checkingFileName(1);
    if (fileName == "=") {
        return;
    }

    fileName = processFile(fileName);
    ifstream input(fileName);
    if (input.is_open()) {
        uploadFromFile = true;
        parceText(words, input, str);
    }
    input.close();
    remove(fileName.c_str());
}

// Функция для вывода результатов
void printResults(vector<Word>& words) {
    for (auto& word : words) {
        cout << "Слово: " << word.word << endl;
        cout << "Предложение: " << word.sentence << endl;
        cout << "Количество: " << word.count << endl;
        cout << "---------------------" << endl;
    }
    cin.get();
}

bool saveToFile(vector<Word>& words, string& fileName) {
    system("clear");
    ofstream output(fileName);
    if (!output.is_open()) {
        cout << "Ошибка при открытии файла для записи." << endl;
        return false;
    }

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

bool exitSave(vector<Word>& words) {
    bool validInput = false;
    char input;
    cout << "Хотите сохранить список книг перед выходом? (y/n)" << endl;
    cout << "Или введите символ ‘=’, для того чтобы вернуться в главное меню. " << endl;

    while (!validInput) {
        input = getchar();
        if (input == 'y' || input == 'Y') {
            string saveFileName = checkingFileName(2);
            if (saveFileName == "=") {
                return false;
            }
            bool exit = saveToFile(words, saveFileName);
            if (!exit) {
                return false;
            }
            break;
        } else if (input == 'n' || input == 'N') {
            break;
        } else if (input == '=') {
            return false;
        }
    }
    return true;
}

int main() {
    vector<Word> words;
    bool validInput = false;
    char input;
    setlocale(LC_ALL, "ru_RU.UTF-8");
    system("clear");
    printMenu();
    while (!validInput) {
        input = getchar();
        if (input == '1') {
            words.clear();
            spacyMain(words);
            hasUnsavedChanges = true;
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear");
            printMenu();
            continue;
        }
        if (input == '2') {
            if (words.empty() == 1)
            {
                system("clear");
                cout << "Нет данных для сохранения в файл." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                cin.clear();
                cin.ignore(1000, '\n');
                cin.get();
                system("clear");
                printMenu();
                continue;
            }
            else
            {
            string saveFileName = checkingFileName(2);
            if (saveFileName == "=") {
                system("clear");
                printMenu();
                continue;
            }

            if (saveToFile(words, saveFileName)) {
                hasUnsavedChanges = false;
            }
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear");
            printMenu();
            continue;
            }

        }
        if (input == '3') {
            if (words.empty() == 1)
            {
                system("clear");
                cout << "Нет данных для отображения." << endl;
                cout << "Нажмите клавишу Enter для продолжения." << endl;
                cin.clear();
                cin.ignore(1000, '\n');
                cin.get();
                system("clear");
                printMenu();
                continue;
            }
            printResults(words);
            cout << "Нажмите клавишу Enter для продолжения." << endl;
            cin.get();
            system("clear");
            printMenu();
            continue;
        }
        if (input == '4') {
            if (uploadFromFile == false) {
                break;
            } else {
                if (hasUnsavedChanges == true) {
                    system("clear");
                    bool exit = exitSave(words);
                    if (exit) {
                        validInput = true;
                        break;
                    } else {
                        system("clear");
                        printMenu();
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
