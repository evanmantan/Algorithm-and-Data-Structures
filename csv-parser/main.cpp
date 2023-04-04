#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <map>
#include <iomanip>

using namespace std;
namespace fs = filesystem;

typedef unordered_map<string, map<string, string>> nested_map_string;
typedef unordered_map<string, map<string, int>> nested_map_int;
typedef unordered_map<string, int> u_map;

void PrintIDNameScore(string id, string name, string score) {
    cout << id << setw(25) << name << setw(10) << score << endl;
}

void PrintIDNameScore(string id, string name, float score) {
    cout << id << setw(25) << name << setw(10) << score << endl;
}

void CategorizeScore(string class_name, int score, nested_map_int *pCat, bool firstRun) {
    string range[] = {"0-60", "61-75", "76-90", "91-100"};
    if (firstRun) {
        for (int i = 0; i < range->size(); i++) {
            (*pCat)[class_name][range[i]] = 0;
        }
    }
    if (score <= 60) (*pCat)[class_name][range[0]] += 1;
    else if (score <= 75) (*pCat)[class_name][range[1]] += 1;
    else if (score <= 90) (*pCat)[class_name][range[2]] += 1;
    else if (score <= 100) (*pCat)[class_name][range[3]] += 1;
}

// Read CSV using provided path
nested_map_string ReadCsvToStruct(string path, string class_name, nested_map_string student, u_map *pMax, nested_map_int *pCat) {
    string line, word;
    fstream fin(path, ios::in);
    if (fin.is_open()) {
        int max = 0;
        bool firstRun = true;
        while (getline(fin, line)) {
            stringstream ss(line);
            int i = 0;
            string id;
            while (getline(ss, word, ';')) {
                if (i % 3 == 0) id = word;
                else if (i % 3 == 1) student[id]["Name"] = word;
                else if (i % 3 == 2) {
                    student[id][class_name] = word;
                    CategorizeScore(class_name, stoi(word), pCat, firstRun);
                    if (firstRun) {
                        firstRun = false;
                        max = int(stoi(word));
                    }   
                    if (max < stoi(word)) max = stoi(word);
                }
                i++;
            }
        }
        (*pMax)[class_name] = max;
        return student;
    } else {
        cout << "File with path '" << path << "' doesn't exist" << endl;
    }
    return student;
}

nested_map_string ScanDirectory(fs::path database, u_map *pMax, nested_map_int *pCat) {
    nested_map_string student;
    for (auto const& dir_entry : fs::directory_iterator{database}) {
        string path = dir_entry.path().u8string();
        string class_name = dir_entry.path().stem().u8string();
        student = ReadCsvToStruct(path, class_name, student, pMax, pCat);
    }
    return student;
}

void PrintScore(string id, nested_map_string student) {
    if (student[id].empty()) {
        cout << "No student in record with that ID" << endl;
        return;
    }
    cout << "ID\t: " << id << endl;
    cout << "NAME\t: " << student[id]["Name"] << endl;
    for (const auto& [key, value] : student[id]) {
        if (key != "Name") cout << key << "\t: " << value << endl;
    }
}

void PrintAverage(string id, nested_map_string student) {
    if (student[id].empty()) {
        cout << "No student in record with that ID" << endl;
        return;
    }
    cout << "ID\t\t: " << id << endl;
    cout << "NAME\t\t: " << student[id]["Name"] << endl;
    float average = 0;
    for (const auto& [key, value] : student[id]) {
        if (key != "Name") average += stoi(value);
    }
    cout << "AVERAGE SCORE : " << average / (student[id].size() - 1) << endl;
}

void PrintHighest(string class_name, nested_map_string student, u_map max) {
    if (!max[class_name]) {
        cout << "No class in record with that name" << endl;
        return;
    }
    string max_stringified = to_string(max[class_name]);
    cout << "ID" << setw(25) << "NAME" << setw(10) << "SCORE" << endl;
    for (const auto& [key, _] : student) {
        if (student[key][class_name] == max_stringified) {
            // cout << key << "\t" << student[key]["Name"] << " " << student[key][class_name] << endl;
            PrintIDNameScore(key, student[key]["Name"], student[key][class_name]);
        }
    }
}

void PrintCategory(string class_name, nested_map_int cat) {
    if (cat[class_name].empty()) {
        cout << "No class in record with name" << endl;
        return;
    }
    string range[] = {"0-60", "61-75", "76-90", "91-100"};
    for (int i = 0; i < range->size(); i++) {
        cout << range[i] << "\t: ";
        cout << cat[class_name][range[i]] << endl;
    }
}

multimap<float, string> FindTopScorer(nested_map_string student) {
    float average;
    multimap<float, string> average_score;
    for (const auto& [id, _] : student) {
        int total = 0;
        for (const auto& [key, value] : student[id]) {
            if (key != "Name") total += stoi(value);
        }
        average = float(total) / (student[id].size() - 1);
        average_score.insert({average, id});
    }
    return average_score;
}

void PrintTopScorer(int nums, nested_map_string student) {
    multimap<float, string> average_score = FindTopScorer(student);
    int i = 0;
    cout << "ID" << setw(25) << "NAME" << setw(10) << "SCORE" << endl;
    for (auto it = average_score.crbegin(); it != average_score.crend(); ++it) {
        // std::cout << it->second << "\t" << student[it->second]["Name"] << "\t" << it->first << endl;
        PrintIDNameScore(it->second, student[it->second]["Name"], it->first);
        i++;
        if (i == nums) break;
    }
    return;
}

void WipeScreen() {
    cout << "\033[2J\033[1;1H";
}

void PrintIntro() {
    WipeScreen();
    cout << "CSV PARSER by Evan Manuel Tan" << endl;
    cout << "=============================" << endl;
}

void PrintMenu() {
    cout << "\nList of commands\n";
    cout << " 1. Show scores from a single student by ID\n";
    cout << " 2. Show the average score from a single student by ID\n";
    cout << " 3. Show name of student(s) who scored the highest by class\n";
    cout << " 4. Show the distribution of score by class\n";
    cout << " 5. Show n number of student(s) with the highest average score\n";
    cout << "\nEnter any other input to exit\n" << endl;
}

void PrintOutro() {
    WipeScreen();
    cout << "====================================\n";
    cout << "Thanks for using! Have a nice day :D\n";
    cout << "====================================" << endl;
}

int main() {
    fs::path database{"database"};
    u_map max;
    nested_map_int cat; // cat stands for Category
    nested_map_string student = ScanDirectory(database, &max, &cat);
    PrintIntro();
    while (true) {
        int n;
        bool quit = false;
        PrintMenu();
        cout << "Enter choice : ";
        cin >> n;
        string choice;
        switch (n)
        {
        case 1:
            cout << "Input Student ID : ";
            cin >> choice;
            PrintScore(choice, student);
            break;
        case 2:
            cout << "Input Student ID : ";
            cin >> choice;
            PrintAverage(choice, student);
            break;
        case 3:
            cout << "Input Class Name : ";
            cin >> choice;
            PrintHighest(choice, student, max);
            break;
        case 4:
            cout << "Input Class Name : ";
            cin >> choice;
            PrintCategory(choice, cat);
            break;
        case 5:
            cout << "Input Number : ";
            cin >> choice;
            PrintTopScorer(stoi(choice), student);
            break;
        default:
            quit = true;
            break;
        }
        if (quit) break;
        cin.ignore();

        cout << "\nPress enter to go back to menu" << endl;
        getchar();
        WipeScreen();
    }
    PrintOutro();

    return 0;
}