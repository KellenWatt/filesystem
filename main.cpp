#include <iostream>
#include <string>
#include <map>
#include <cstdint>
#include <vector>

#include "file.h"
#include "shell.h"

using std::getline;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::pair;
using std::string;
using std::vector;


string filepath(Directory* cwd) {
    string path;
    Directory* d = cwd;
    while(d->parent != d) {
        path = d->name + "/" + path;
        d = static_cast<Directory*>(d->parent);
    }
    return d->name + "/" + path;
}

int main() {
    map<uint16_t,string> users;
    users.insert(pair<uint16_t, string>(0, "root"));
    map<uint16_t,string> groups;
    groups.insert(pair<uint16_t, string>(0, "root"));

    Directory* wd;

    Directory* root = NewRoot();
    Directory* child = NewDir(0777, "subdir", 0, 0, root);
    Directory* subchild = NewDir(0777, "subsubdir", 0, 0, child);
    File* file = NewFile(0777, "file", 0, 0, root);
    File* file2 = NewFile(0777, "subfile", 0, 0, child);

    wd = root;
    vector<string> args;
    string input;

    int USER = 0;
    int GROUP = 0;

    while(true) {
        cout << users[USER] + ":" + filepath(wd) + "# ";

        getline(cin, input);
        for(int i=0; i<input.size(); i++) {
            string word;
            bool dquote = false;
            bool squote = false;
            while(i<input.size() && input[i] != ' ' && !(dquote || squote)) {
                if(i+1 < input.size() && input[i] == '\\' && input[i+1] == ' ') {
                    word += ' ';
                    i++;
                } else if(input[i] == '"' && !squote) {
                    dquote = !dquote;
                } else if (input[i] == '\'' && !dquote) {
                    squote = !squote;
                } else {
                    word += input[i];
                }
                i++;
            }
            args.push_back(word);
        }

        if(args.empty()) continue;

        string command = args[0];
        args.erase(args.begin());

        if(command == "ls") {
            list_files(args, wd, USER, GROUP, users, groups);
        } else if(command == "cd") {
            change_directory(args, wd, USER, GROUP);
        } else if(command == "pwd") {
            print_working_directory(wd);
        } else if(command == "mkdir") {
            make_directory(args, wd, USER, GROUP);
        } else if(command == "rmdir") {
            remove_directory(args, wd, USER, GROUP);
        } else if(command == "rm") {
            remove_file(args, wd, USER, GROUP);
        } else if(command == "chmod") {
            change_permissions(args, wd, USER, GROUP);
        } else if(command == "touch") {
            update_timestamp(args, wd, USER, GROUP);
        } else if(command == "exit" || command == "quit") {
            return 0;
        } else {
            cout << args[0] << ": command not found" << endl;
        }

        args.clear();
    }
}
