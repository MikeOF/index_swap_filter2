#include "path.h"

using namespace std ;

Path::Path() { this->abs_path_str = Path(".").abs_path_str ; }

Path::Path(const string& path_str_arg) {

	string path_str = string(path_str_arg) ;

	// ignore paths with .. and . tokens that aren't just "."
	if (path_str != ".") {
		string token ;
		istringstream token_stream (path_str);
		while (getline(token_stream, token, '/')) {
			if (token == "." || token == "..") throw runtime_error("unsupported path string") ;
		}
	}

	// make sure the path isn't empty
	if (path_str.empty()) throw runtime_error("the path string is empty") ;

	// remove trailing slashes 	
	while (path_str.size() > 1 && path_str.at(path_str.size() - 1) == '/') { 
		path_str = path_str.substr(0, path_str.size() - 1) ; 
	}

	// remove double slashes
	while (path_str.find("//") != string::npos) { path_str = path_str.replace(path_str.find("//"), 2, "/") ; }

	if (path_str == ".") {

		this->abs_path_str = get_absolute_path_str("") ;

	} else if (path_str == "/") {

		this->abs_path_str = path_str ;

	} else if (path_str.at(0) == '/') { 

		this->abs_path_str = path_str ; 

	} else {

		this->abs_path_str = get_absolute_path_str(path_str) ;
	}
}

string Path::get_absolute_path_str(const string& path_str) {

	char buf [PATH_MAX] ;

	getcwd(buf, PATH_MAX) ;

	string abs_path_str = string(buf) ;

	if (!path_str.empty()) abs_path_str += "/" + path_str ;

	return abs_path_str ;
}

// existence and type

bool Path::exists() const { 

	struct stat pstat ;
	return stat(this->abs_path_str.c_str(), &pstat) == 0 ; 
}

bool Path::is_file() const {

	struct stat pstat ;
	int result = stat(this->abs_path_str.c_str(), &pstat) ;

	return result == 0 && S_IFREG & pstat.st_mode ;
}

bool Path::is_dir() const {

	struct stat pstat ;
	int result = stat(this->abs_path_str.c_str(), &pstat) ;

	return result == 0 && S_IFDIR & pstat.st_mode ;
}

// file operations

void Path::remove_file() const {

	if (!this->is_file()) throw runtime_error("attempt to file-remove a non-file path, " + this->abs_path_str) ;

	int result = remove(this->abs_path_str.c_str()) ;
	if (result) throw runtime_error("could not remove the file, " + this->abs_path_str) ;
}

void Path::rename(const Path new_path, bool exists_ok) {

	bool new_path_exists = new_path.exists() ;

	if (new_path_exists) {
		
		if (!exists_ok) runtime_error("attempt to overwrite path with specifying exists_ok = true") ;
		if (new_path.is_dir()) throw runtime_error("cannot rename to an existant directory") ;
	}

	bool this_exists = this->exists() ;

	if (this_exists) { 

		std::rename(this->abs_path_str.c_str(), new_path.abs_path_str.c_str()) ;

	} else if (new_path_exists) {

		new_path.remove_file() ;
	}
 
	this->abs_path_str = new_path.abs_path_str ;
}

void Path::rename(const string& new_path_str, bool exists_ok) { this->rename(Path(new_path_str), exists_ok) ; }
void Path::rename(const Path new_path) { this->rename(new_path, false) ; }
void Path::rename(const string& new_path_str) { this->rename(Path(new_path_str), false) ; }

// dir operations 

vector<string> Path::get_dir_list() {

	if (!this->is_dir()) throw runtime_error ("attempt to read a non-directory path, " + this->abs_path_str) ;

	DIR * dir;
	struct dirent * ent;
	vector<string> ret_vect ;

	if ((dir = opendir(this->abs_path_str.c_str())) != NULL) {

		/* collect all the files and directories within directory */

		while ((ent = readdir (dir)) != NULL) {

			string file_name = string(ent->d_name) ;

			if (file_name != "." && file_name != "..") ret_vect.push_back(this->abs_path_str + "/" + string(ent->d_name)) ;

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

	int rval = mkdir(this->abs_path_str.c_str(),  0777) ;
	if (rval) throw runtime_error("Could not make directory: " + this->abs_path_str) ;
}

void Path::remove_dir_recursively() {

	if (this->abs_path_str == "/") throw runtime_error("why whould you do this? rm /??") ;
	if (!this->is_dir()) throw runtime_error("attempt to recursively remove a non-directory path, " + this->abs_path_str) ;

	for (string sub_path_str : this->get_dir_list()) {
		Path sub_path = Path(sub_path_str) ;
		if (sub_path.is_dir()) { sub_path.remove_dir_recursively() ; }
		else { sub_path.remove_file() ; }
	}

	int result = rmdir(this->abs_path_str.c_str()) ;
	if (result) throw runtime_error("could not remove the directory, " + this->abs_path_str) ;
}

// path operations

string Path::to_relative_path_string() { 

	// tvars
	string token ;
	istringstream token_stream ;

	char buf [PATH_MAX] ;
	getcwd(buf, PATH_MAX) ;
	Path cwd_path = Path(string(buf)) ;

	// tokenize cwd
	vector<string> cwd_tokens ; 
	istringstream token_stream (cwd_path.abs_path_str) ;
	while (getline(token_stream, token, '/')) cwd_tokens.push_back(token) ;

	// tokenize abs path str
	vector<string> this_tokens ; 
	token_stream = istringstream(this->abs_path_str) ;
	while (getline(token_stream, token, '/')) this_tokens.push_back(token) ;

	// create a relative path of tokens
	vector<string> relative_path_tokens ;

	// get the number of shared tokens ?
	int shared_tokens = 0 ;
	for (int i = 0; i < cwd_tokens.size() && i < this_tokens.size(); i++) {
		if (cwd_tokens.at(i) == this_tokens.at(i)) shared_tokens++ ;
		else { break ; }
	}

	// add .. tokens
	for (int i = shared_tokens; i < cwd_tokens.size(); i++) {
		relative_path_tokens.push_back("..") ;
	}

	// add this tokens
	for (int i = shared_tokens; i < this_tokens.size(); i++) {
		relative_path_tokens.push_back(this_tokens.at(i)) ;
	}

	if (relative_path_tokens.size() == 0) return "." ;

	// create the relative path string from tokens
	stringstream ss;
	ss << relative_path_tokens.at(0) ;
	for (int i = 1; i < relative_path_tokens.size(); i++) {
		ss << "/" << relative_path_tokens.at(i) ;
	}

	return ss.str() ;
}

Path Path::get_parent_path() {

	int last_sep = this->abs_path_str.rfind('/') ;

	if (last_sep == string::npos) throw runtime_error("an absolute path should have a /") ;
	if (last_sep == 0) throw runtime_error("there is no parent for the root directory") ;

	return Path(this->abs_path_str.substr(0, last_sep)) ; 
}

string Path::get_filename() {

	int last_sep = this->abs_path_str.rfind('/') ;

	if (last_sep == string::npos) throw runtime_error("an absolute path should have a /") ;
	if (last_sep == 0) throw runtime_error("there is no filename for the root directory") ;

	return this->abs_path_str.substr(last_sep + 1) ;
}

string Path::get_filename_stem() {

	string filename = this->get_filename() ;

	int first_dot = filename.find('.') ;

	if (first_dot == string::npos) { return filename ; }

	return filename.substr(0, first_dot) ;
}

Path Path::join(const string& s) { 

	if (s.at(0) == '/') throw runtime_error("cannot append an absolute path") ;
	return Path(this->abs_path_str + "/" + s) ;
}
