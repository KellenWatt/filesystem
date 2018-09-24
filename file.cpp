#include "file.h"
#include <ctime>
#include <string>

#include <iostream>
#include <vector>
#include <algorithm>


uint64_t Inode::id_counter = 0;

Directory* NewDir(uint16_t perms, const std::string& name, uint16_t user, uint16_t group, Directory* parent) {
    Directory* dir = new Directory;
    dir->id = Inode::id_counter++;
    dir->perms = perms;
    dir->name = name;
    dir->type = filetype::DIRECTORY;
    time_t tmp_time = std::time(0);
    dir->timestamp = *std::localtime(&tmp_time);
    dir->user = user;
    dir->group= group;
    dir->parent = parent;
    if(parent != NULL) {
        parent->children.push_back(dir);
    }
    dir->size = 0;
    return dir;
}

Directory* NewDirStrPerm(const std::string& perms, const std::string& name, uint16_t user, uint16_t group, Directory* parent) {
    uint16_t perm = perm_bits(perms);
    return NewDir(perm, name, user, group, parent);
}

Directory* NewRoot() {
    Directory* root = NewDir(0755, "", 0, 0, NULL);
    root->parent = root;
    return root;
}

File* NewFile(uint16_t perms, const std::string& name, uint16_t user, uint16_t group, Directory* parent) {
    File* f = new File;
    f->id = Inode::id_counter++;
    f->perms = perms;
    f->name = name;
    f->type = filetype::TEXT;
    time_t tmp_time = std::time(0);
    f->timestamp = *std::localtime(&tmp_time);
    f->user = user;
    f->group = group;
    // assert parent != NULL
    f->parent = parent;
    parent->children.push_back(f);
    f->size = 0;
    return f;
}

File* NewFileStrPerm(const std::string& perms, const std::string& name, uint16_t user, uint16_t group, Directory* parent) {
    uint16_t perm = perm_bits(perms);
    return NewFile(perm, name, user, group, parent);
}

const std::string perm_string(Inode* node) {
    char* perm = new char[11];
    perm[10] = '\0';
    const char* rwx = "rwx";
    perm[0] = (node->type == filetype::DIRECTORY) ? 'd' : '-';
    
    for(int i=1; i<10; i++) {
        if((node->perms >> (9-i)) & 1) {
            perm[i] = rwx[(i-1) % 3];
        } else {
            perm[i] = '-';
        }
    }
    return std::string(perm);
}

uint16_t perm_bits(const std::string& perm_string) {
    uint16_t perm = 0;
    for(int i=1; i<10; i++) {
        perm <<= 1;
        if(perm_string[i-1] != '-') {
            perm |= 1;
        }
    }
    return perm;
}

const std::string& name_lookup(std::map<uint16_t,std::string> name_table, uint16_t id) {
    return name_table[id];
}

const char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const std::string parse_month(const tm timestamp) {
    return months[timestamp.tm_mon];
}

int parse_year(const tm& timestamp) {
    return 1900 + timestamp.tm_year;
}

int hardlink_count(Inode* node) {
    if(node->type == filetype::TEXT) {
        return 1;
    } else {
        int total = 2;
        Directory* n = static_cast<Directory*>(node);
        for(unsigned long i=0; i< n->children.size(); i++) {
            if(n->children[i]->type == filetype::DIRECTORY) {
                total += 1;
            }
        }
        return total;
    }
}


