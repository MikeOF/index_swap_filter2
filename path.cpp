#include "path.h"

using namespace std ;

Path::Path(string path) {

	// make sure the path isn't empty
	if (path.empty()) throw runtime_error("the path string is empty") ;

	// remove trailing slashes 	
	while (path.at(path.size() - 1) == '/') { path = path.substr(0, path.size() - 1) ; }

	// remove double slashes
	while (path.find("//") != string::npos) { path = path.replace(path.find("//"), 2, "/") ; }

	this->path = string(path) ;

	if (this->path.at(0) == '/') this->abs_path = this->path ;
}

// existence and type

void Path::set_mode() {

	struct stat pstat ;
	int result = stat(this->path.c_str(), &pstat) ;

	this->p_exists = (result == 0) ;
	this->mode = pstat.st_mode ;
	this->has_mode = true ;
}

bool Path::exists() {

	this->set_mode() ;
	return this->p_exists ;
}

bool Path::is_file() {

	this->set_mode() ;
	return (S_IFREG & this->mode) ;
}

bool Path::is_dir() {

	this->set_mode() ;
	return (S_IFDIR & this->mode) ;
}

// file operations

void Path::remove_file() {

	if (!this->is_file()) throw runtime_error("attempt to file-remove a non-file path, " + this->path) ;

	int result = remove(this->path.c_str()) ;
	if (result) throw runtime_error("could not remove the file, " + this->path) ;
}

void rename(const Path& new_path, bool exists_ok) {

	if (!exists_ok && new_path.exists()) runtime_error("cannot overwrite this path") ;

	if (this->exists())
 
	this->path = new_path.to_string() ;
	this->abs_path = this->get_absolute_path() ;
}

void rename(const string& new_path_str, bool exists_ok) { this->rename(Path(new_path_str), exists_ok) ; }
void rename(const Path& new_path) { this->rename(new_path, false) ; }
void rename(const string& new_path_str) { this->rename(Path(new_path_str), false) ; }

// dir operations 

vector<string> Path::get_dir_list() {

	if (!this->is_dir()) throw runtime_error ("attempt to read a non-directory path, " + this->path) ;

	DIR * dir;
	struct dirent * ent;
	vector<string> ret_vect ;

	if ((dir = opendir(this->path.c_str())) != NULL) {

		/* collect all the files and directories within directory */

		while ((ent = readdir (dir)) != NULL) {

			string file_name = string(ent->d_name) ;

			if (file_name != "." && file_name != "..") ret_vect.push_back(this->path + "/" + string(ent->d_name)) ;

		}
		closedir (dir);

	} else {

		/* could not open directory */
		perror ("");
	}

	return ret_vect ;
} ;

bool Path::dir_is_empty() {

	vector<string> ret_vect = this->get_dir_list() ;
	return ret_vect.empty() ;
}

void Path::make_dir() {

	if (this->exists()) throw runtime_error("attempt to make a dir at an existant path") ; 

	int rval = mkdir(this->path.c_str(),  0777) ;
	if (rval) throw runtime_error("Could not make directory: " + path) ;
}

void Path::remove_dir_recursively() {

	if (!this->is_dir()) throw runtime_error("attempt to recursively remove a non-directory path, " + this->path) ;

	for (string sub_path_str : this->get_dir_list()) {
		Path sub_path = Path(sub_path_str) ;
		if (sub_path.is_dir()) { sub_path.remove_dir_recursively() ; }
		else { sub_path.remove_file() ; }
	}

	int result = rmdir(this->path.c_str()) ;
	if (result) throw runtime_error("could not remove the directory, " + this->path) ;
}

// path operations

bool Path::is_absolute() { 

	return this->path.at(0) == '/' ; 
}

Path Path::get_parent_path() {

	int last_sep = this->path.rfind('/') ;

	if (last_sep == string::npos) { return Path(".") ; } 
	else { return Path(this->path.substr(0, last_sep)) ; }
}

Path Path::get_absolute_path() {

	// check to see if we already have it
	if (!this->abs_path.empty()) return Path(this->abs_path) ;

	char buf [PATH_MAX] ;

	getcwd(buf, PATH_MAX) ;

	this->abs_path = string(buf) + "/" + path ;

	return Path(this->abs_path) ;
}

string Path::get_filename() {

	int last_sep = path.rfind('/') ;

	if (last_sep == string::npos) { return string(this->path) ; }

	return this->path.substr(last_sep + 1) ;
}

string Path::get_filename_stem() {

	string filename = this->get_filename() ;

	int first_dot = filename.find('.') ;

	if (first_dot == string::npos) { return filename ; }

	return filename.substr(0, first_dot) ;
}

Path Path::join(Path& p) { 

	if (p.is_absolute()) throw runtime_error("cannot append an absolute path") ;
	return Path(this->path + "/" + p.to_string()) ; 
}

Path Path::join(const string& s) { 

	Path p = Path(s) ;
	return this->join(p) ;
}
