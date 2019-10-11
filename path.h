#ifndef PATH_H
#define PATH_H

#include <vector>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdexcept>

using namespace std ;

class Path {

	string abs_path_str ;

	string get_absolute_path_str(const string&) ;

	public:

		Path() ;

		Path(const string&) ;

		// existence and type
		bool exists() const ;
		bool is_file() const ;
		bool is_dir() const ;

		// file operations
		void remove_file() const ;
		void rename(const Path, bool) ;
		void rename(const string&, bool) ;
		void rename(const Path) ;
		void rename(const string&) ;

		// dir operations 
		bool dir_is_empty() ;
		vector<string> get_dir_list() ;
		void make_dir() ;
		void remove_dir_recursively() ;

		// path operations
		string to_string() { return this->abs_path_str ; }
		string to_relative_path_string() ;
		Path get_parent_path() ;
		string get_filename() ;
		string get_filename_stem() ;
		Path join(const string&) ;
		Path join(Path&) ;
} ;

#endif