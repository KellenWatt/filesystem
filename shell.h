#ifndef OS_SHELL_H
#define OS_SHELL_H

#include "file.h"
#include <vector>
#include <string>
#include <cstdint>
#include <map>

/* define:
 * ls,
 * ls -l,
 * cd,
 * pwd,
 * mkdir,
 * rmdir,
 * rm,
 * chmod (fully functional)(-ish)
 * touch (create file and update timestamp)
 * exit/quit
 */

// ls, ls -l
//                list of arguments,        cwd,   userid,  groupid
void list_files(std::vector<std::string>, Directory*, uint16_t, uint16_t, std::map<uint16_t,std::string>&, std::map<uint16_t,std::string>&);

// cd
void change_directory(std::vector<std::string>, Directory*&, uint16_t, uint16_t);

// pwd
void print_working_directory(Directory*);

// mkdir
void make_directory(std::vector<std::string>, Directory*, uint16_t, uint16_t);

// rmdir
void remove_directory(std::vector<std::string>, Directory*, uint16_t, uint16_t);

// rm
void remove_file(std::vector<std::string>, Directory*, uint16_t, uint16_t);

// chmod
void change_permissions(std::vector<std::string>, Directory*, uint16_t, uint16_t);

// touch
void update_timestamp(std::vector<std::string>, Directory*, uint16_t, uint16_t);

// pretty sure exit/quit are/should be handled at a higher level

#endif
