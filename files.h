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
#include <stdexcept>

using namespace std ;

string get_parent_path(string) ;
string get_absolute_path(string) ;
bool file_exists(string) ;
bool is_file(string) ;
bool is_dir(string) ;
vector<string> get_dir_list(string) ;
bool dir_is_empty(string) ;
string get_filename(string) ;
string get_file_stem(string) ;
string remove_trailing_slashes(string) ;
void make_dir(string) ;
void remove_file(string) ;
void remove_dir_recursively(string) ;

#endif