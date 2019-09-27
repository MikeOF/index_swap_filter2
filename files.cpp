#include "files.h"

using namespace std ;

mode_t get_mode(string path) {

	struct stat pstat ;
	stat(path.c_str(), &pstat) ;

	return(pstat.st_mode) ;
}

string get_parent_path(string path) {

	int last_sep = path.rfind('/') ;

	if (last_sep == string::npos) {
		return "." ;
	} else {
		return path.substr(0, last_sep) ;
	}
}

string get_absolute_path(string path) {

	if (path.find('/') == 0) return path ;

	char buf [PATH_MAX] ;

	getcwd(buf, PATH_MAX) ;

	return string(buf) + "/" + path ;
}

bool file_exists(string path) {

	struct stat pstat ;
	stat(path.c_str(), &pstat) ;

	return (stat(path.c_str(), &pstat) == 0) ;
}

bool is_file(string path) {

	if (!file_exists(path)) return false ;

	mode_t pmode = get_mode(path) ;

	return (S_IFREG & pmode) ;
}

bool is_dir(string path) {

	if (!file_exists(path)) return false ;

	mode_t pmode = get_mode(path) ;

	return (S_IFDIR & pmode) ;
}

vector<string> get_dir_list(string path) {

	if (!is_dir(path)) throw runtime_error ("attempt to read a non-directory path, " + path) ;
	path = remove_trailing_slashes(path) ;

	DIR * dir;
	struct dirent * ent;
	vector<string> ret_vect ;

	if ((dir = opendir(path.c_str())) != NULL) {

		/* collect all the files and directories within directory */

		while ((ent = readdir (dir)) != NULL) {

			string file_name = string(ent->d_name) ;

			if (file_name != "." && file_name != "..") ret_vect.push_back(path + "/" + string(ent->d_name)) ;

		}
		closedir (dir);

	} else {

		/* could not open directory */
		perror ("");
	}

	return ret_vect ;
} ;

bool dir_is_empty(string path) {

	vector<string> ret_vect = get_dir_list(path) ;

	return ret_vect.empty() ;

}

string get_filename(string path) {

	if (!is_file(path)) { throw path + " is not a file" ; }

	int last_sep = path.rfind('/') ;

	if (last_sep == string::npos) { return path ; }

	return path.substr(last_sep + 1) ;
}

string get_file_stem(string path) {

	string file_name = get_filename(path) ;

	int first_dot = file_name.find('.') ;

	if (first_dot == string::npos) { return file_name ; }

	return file_name.substr(0, first_dot) ;
}

string remove_trailing_slashes(string path) {

	while (path.at(path.size() - 1) == '/') { path = path.substr(0, path.size() - 1) ; }

	return path ;
}

void make_dir(string path) {

	int rval = mkdir(path.c_str(),  0777) ;
	if (rval) throw runtime_error("Could not make directory: " + path) ;
}

void remove_file(string path) {

	if (!is_file(path)) throw runtime_error("attempt to remove a non-file path, " + path) ;

	int rval = remove(path.c_str()) ;
	if (rval) throw runtime_error("could not remove the file, " + path) ;
}

void remove_dir_recursively(string path) {

	if (!is_dir(path)) throw runtime_error("attempt to recursively remove a non-directory path, " + path) ;

	for (string sub_path : get_dir_list(path)) {
		if (is_dir(sub_path)) { remove_dir_recursively(sub_path) ; }
		else { remove_file(sub_path) ; }
	}

	int rval = rmdir(path.c_str()) ;
	if (rval) throw runtime_error("could not remove the directory, " + path) ;
}
