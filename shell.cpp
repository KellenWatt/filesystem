#include "shell.h"
#include "file.h"
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

void print_long_listing(Inode* node, uint16_t user, uint16_t group, std::map<uint16_t,std::string>& users, std::map<uint16_t,std::string>& groups) {
    time_t now = std::time(0);
    double diff = difftime(now, std::mktime(&node->timestamp));
    
    std::string fmt = "%s  %d %s  %s %5d %s %2d ";
    if(diff > 31536000) { // magic number is a year of seconds
        fmt += node->timestamp.tm_year;
    } else {
        fmt += std::to_string(node->timestamp.tm_hour) + ":" + std::to_string(node->timestamp.tm_min);
    }
    fmt += " %s\n";

    printf(fmt.c_str(),
            perm_string(node).c_str(),
            hardlink_count(node),
            name_lookup(users, user).c_str(),
            name_lookup(groups, group).c_str(),
            (node)->size,
            parse_month((node)->timestamp).c_str(),
            (node)->timestamp.tm_mday,
            (node)->name.c_str());
}


void list_files(std::vector<std::string> args, Directory* cwd, uint16_t user, uint16_t group, std::map<uint16_t,std::string>& users, std::map<uint16_t,std::string>& groups) {
    if(args.size() > 0 && args[0] == "-l") {
        //long form
        args.erase(args.begin());

        if(!args.empty()) {
            // list current directory
            for(int i=0; i<args.size(); i++) {
                std::string fname = args[i];
                Inode* file = NULL;
                for(int j=0; j<cwd->children.size(); j++) {
                    if(cwd->children[j]->name == fname) {
                        file = cwd->children[j];
                    }
                }
                if(file == NULL) {
                    std::cerr << "ls: " << fname << ": No such file or directory" << std::endl;
                    return;
                }
                
                if(file->type == filetype::DIRECTORY) {
                    Directory* d = static_cast<Directory*>(file);
                    printf("%s:\n", d->name.c_str());
                    for(auto c = d->children.begin(); c != d->children.end(); c++) {
                        print_long_listing(*c, user, group, users, groups);
                    }
                } else {
                    print_long_listing(file, user, group, users, groups);
                }
            }
        } else {
            for(auto c = cwd->children.begin(); c != cwd->children.end(); c++) {
                print_long_listing(*c, user, group, users, groups);
            }
        }
    } else {
        if(!args.empty()) {
            for(int i=0; i<args.size(); i++) {
                std::string fname = args[i];
                Inode* file = NULL;
                for(int j=0; j<cwd->children.size(); j++) {
                    if(cwd->children[j]->name == fname) {
                        file = cwd->children[j];
                    }
                }
                if(file == NULL) {
                    std::cerr << "ls: " << fname << ": No such file or directory" << std::endl;
                    return;
                }
                if(file->type == filetype::DIRECTORY) {
                    Directory* d = static_cast<Directory*>(file);
                    printf("%s:\n", d->name.c_str());
                    for(auto c = d->children.begin(); c != d->children.end(); c++) {
                        printf("%s\n", (*c)->name.c_str());
                    }
                } else {
                    printf("%s\n", file->name.c_str());
                }
            }
        } else {
            for(auto c = cwd->children.begin(); c != cwd->children.end(); c++) {
                printf("%s\n", (*c)->name.c_str());
            } 
        }
    }
}

// HUGE NOTE: cwd functions as both an in- and out-parameter
void change_directory(std::vector<std::string> args, Directory*& cwd, uint16_t user, uint16_t group) {
    if(args.empty()) {
        Directory* d = cwd;
        while(d->parent != d) {
            d = static_cast<Directory*>(d->parent);
        }
        cwd = d;
        return;
    } 
    // only pay attention to args[0]
    std::string fname = args[0];
    if(args[0] == "..") {
        cwd = static_cast<Directory*>(cwd->parent);
    } else {
        for(auto i=cwd->children.begin(); i!=cwd->children.end(); i++) {
            if((*i)->name == fname) {
                if((*i)->type == filetype::DIRECTORY) {
                    cwd = static_cast<Directory*>(*i);
                    return;
                } else {
                    std::cerr << "cd: " << fname << ": Not a directory" << std::endl;
                    return;
                }
            }
        }
        std::cerr << "cd: " << fname << ": No such file or directory" << std::endl;
    }
}

void print_working_directory(Directory* cwd) {
    std::vector<std::string> path;
    Directory* d = cwd;
    while(d->parent != d) {
        path.insert(path.begin(), d->name);
        d = static_cast<Directory*>(d->parent);
    }
    path.insert(path.begin(), d->name);
    for(int i=0; i<path.size(); i++) {
        std::cout << path[i] << "/";
    }
    std::cout << std::endl;
}

void make_directory(std::vector<std::string> args, Directory* cwd, uint16_t user, uint16_t group) {
    if(args.empty()) {
        std::cerr << "mkdir: missing operand" << std::endl;
        return;
    }
    for(auto i = args.begin(); i != args.end(); i++) {
        for(auto c = cwd->children.begin(); c != cwd->children.end(); c++) {
            if((*c)->name == (*i)){
                std::cerr << "mkdir: " << *i << ": File exists" << std::endl;
                return;
            }
        }        
        NewDir(0775, *i, user, group, cwd);
    }
}

void remove_directory(std::vector<std::string> args, Directory* cwd, uint16_t user, uint16_t group) {
    if(args.empty()) {
        std::cerr << "rmdir: missing operand" << std::endl;
        return;
    }
    for(auto i = args.begin(); i != args.end(); i++) {
        for(auto c = cwd->children.begin(); c != cwd->children.end(); c++) {
            if((*c)->name == (*i)) {
                if((*c)->type == filetype::DIRECTORY) {
                    Directory* dir = static_cast<Directory*>(*c);
                    if(!dir->children.empty()) {
                        std::cerr << "rmdir: " << *i << ": Directory not empty" << std::endl;
                        return;
                    } else {
                        Directory* parent = static_cast<Directory*>(dir->parent);
                        auto pos = std::find(parent->children.begin(), parent->children.end(), dir);
                        parent->children.erase(pos);

                        // I am abundantly aware of the fact that there is a memory leak at timestamp
                        delete dir;
                        //DeleteDir(dir, static_cast<Directory*>(dir->parent));
                        return;
                    }
                } else {
                    std::cerr << "rmdir: " << *i <<": Not a directory" << std::endl;
                    return;
                }
            }
        }
        std::cerr << "rmdir: " << *i << ": No such file or directory" << std::endl;
    }
}

void remove_file(std::vector<std::string> args, Directory* cwd, uint16_t user, uint16_t group) {
    if(args.empty()) {
        std::cerr << "rm: missing operand" << std::endl;
        return;
    }
    for(auto i = args.begin(); i != args.end(); i++) {
        for(auto c = cwd->children.begin(); c  != cwd->children.end(); c++) {
            if((*c)->name == (*i)) {
                if((*c)->type == filetype::TEXT) {
                    File* file = static_cast<File*>(*c);
                    Directory* parent = static_cast<Directory*>(file->parent);
                    
                    auto pos = std::find(parent->children.begin(), parent->children.end(), file);
                    parent->children.erase(pos);

                    // I am abundantly aware of the fact that there is a memory leak at timestamp
                    delete file;
                    return;
                } else {
                    std::cerr << "rm: " << *i << ": is a directory" << std::endl;
                    return;
                }
            }
        }
        std::cerr << "rm: " << *i << ": No such file or directory" << std::endl;
    }
}

void change_permissions(std::vector<std::string> args, Directory* cwd, uint16_t user, uint16_t group) {
    // chmod {{+|-}[a|o|g]{r|w|x} | <perm-string> | octal}
    // literally only handles the octal case.
    if(args.size() < 2) {
        std::cerr << "chmod: Not enough arguments supplied" << std::endl;
        return;
    }
    std::vector<Inode*> nodes;
    for(auto i = args.begin()+1; i != args.end(); i++) {
        for(auto c = cwd->children.begin(); c != cwd->children.end(); c++) {
            if((*c)->name == *i) {
                nodes.push_back(*c);
            }
        }
    }
    if(nodes.empty()) {
        std::cerr << "chmod: " << args[1] << ": No such file or directory";
        return;
    }
    
    uint16_t perms = static_cast<uint16_t>(strtol(args[0].c_str(), NULL, 8));
    //octal - assume valid input
    for(auto n = nodes.begin(); n != nodes.end(); n++) {
        (*n)->perms = perms;
    }

}

void update_timestamp(std::vector<std::string> args, Directory* cwd, uint16_t user, uint16_t group) {
    if(args.empty()) {
        std::cerr << "touch: missing operand" << std::endl;
        return;
    }
    for(auto i = args.begin(); i != args.end(); i++) {
        bool found = false;
        for(auto c = cwd->children.begin(); c != cwd->children.end(); c++) {
            if((*c)->name == *i) {
                time_t tmp = std::time(0);
                (*c)->timestamp = *std::localtime(&tmp);
                found = true;
            }
        }
        if(!found) {
            NewFile(0664, *i, user, group, cwd);
        }
    }
}





