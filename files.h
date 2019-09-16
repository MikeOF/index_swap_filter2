#ifndef FILES_H
#define FILES_H

#include <vector>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

std::string get_parent_path(std::string) ;
std::string get_absolute_path(std::string) ;
bool file_exists(std::string) ;
bool is_file(std::string) ;
bool is_dir(std::string) ;
std::vector<std::string> get_dir_list(std::string) ;
bool dir_is_empty(std::string) ;
std::string get_filename(std::string) ;
std::string get_file_stem(std::string) ;
std::string remove_trailing_slashes(std::string) ;
void make_dir(std::string) ;

#endif