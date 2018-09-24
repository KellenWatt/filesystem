#ifndef OS_FILE_H
#define OS_FILE_H

#include <cstdint>
#include <string>
#include <ctime>    
#include <vector>
#include <map>

enum class filetype {
    TEXT,
    DIRECTORY
};

struct Inode {
    uint64_t id;
    uint16_t perms;
    std::string name;
    filetype type;
    tm timestamp;
    uint16_t user;
    uint16_t group;
    uint32_t size;
    Inode* parent;
    static uint64_t id_counter;
};

struct Directory : public Inode {
    std::vector<Inode*> children;
};

struct File : public Inode {};

// Begin Constructor Functions
Directory* NewDir(uint16_t perms, const std::string& name, uint16_t user, uint16_t group, Directory* parent);
Directory* NewDirStrPerm(const std::string& perms, const std::string& name, uint16_t user, uint16_t group, Directory* parent);
Directory* NewRoot();
File* NewFile(uint16_t perms, const std::string& name, uint16_t user, uint16_t group, Directory* parent);
File* NewFileStrPerm(const std::string& perms, const std::string& name, uint16_t user, uint16_t group, Directory* parent);


// Begin helper definitions
const std::string perm_string(Inode* node);
uint16_t perm_bits(const std::string&);
const std::string& name_lookup(std::map<uint16_t,std::string>, uint16_t);
const std::string parse_month(const tm);
int parse_year(const tm&);
int hardlink_count(Inode* node);

bool has_user_read_permission(uint16_t, Inode*);
bool has_user_write_permission(uint16_t, Inode*);
bool has_user_exec_permission(uint16_t, Inode*);

bool has_group_read_permission(uint16_t, Inode*);
bool has_group_write_permission(uint16_t, Inode*);
bool has_group_exec_permission(uint16_t, Inode*);

bool has_global_read_permission(Inode*);
bool has_global_write_permission(Inode*);
bool has_global_exec_permission(Inode*);

#endif
