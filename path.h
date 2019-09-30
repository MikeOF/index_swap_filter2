#ifndef FILES_H
#define FILES_H

#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

using namespace std ;

class Path {

	string path ;
	string abs_path ;
	bool p_exists = false ;
	bool has_mode = false ; 
	mode_t mode ;

	void set_mode() ;

	public:

		Path() { path = "." ; }

		Path(string) ;

		// existence and type
		bool exists() ;
		bool is_file() ;
		bool is_dir() ;

		// file operations
		void remove_file() ;

		// dir operations 
		bool dir_is_empty() ;
		vector<string> get_dir_list() ;
		void make_dir() ;
		void remove_dir_recursively() ;

		// path operations
		bool is_absolute() ;
		string to_string() { return this->path ; }
		string get_parent_path() ;
		Path get_absolute_path() ;
		string get_filename() ;
		string get_filename_stem() ;
		Path join(const string& p) ;
		Path join(Path& p) ;
} ;

#endif