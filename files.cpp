#include "files.h"

mode_t get_mode(std::string path) {

	struct stat pstat ;
	stat(path.c_str(), &pstat) ;

	return(pstat.st_mode) ;
}

std::string get_parent_path(std::string path) {

	int last_sep = path.rfind('/') ;

	if (last_sep == std::string::npos) {
		return "." ;
	} else {
		return path.substr(0, last_sep) ;
	}
}

std::string get_absolute_path(std::string path) {

	if (path.find('/') == 0) return path ;

	char buf [PATH_MAX] ;

	getcwd(buf, PATH_MAX) ;

	return std::string(buf) + "/" + path ;
}

bool file_exists(std::string path) {

	struct stat pstat ;
	stat(path.c_str(), &pstat) ;

	return (stat(path.c_str(), &pstat) == 0) ;
}

bool is_file(std::string path) {

	if (!file_exists(path)) return false ;

	mode_t pmode = get_mode(path) ;

	return (S_IFREG & pmode) ;
}

bool is_dir(std::string path) {

	if (!file_exists(path)) return false ;

	mode_t pmode = get_mode(path) ;

	return (S_IFDIR & pmode) ;
}

std::vector<std::string> get_dir_list(std::string path) {

	if (!is_dir(path)) throw std::runtime_error ("attempt to read a non-directory path, " + path) ;
	path = remove_trailing_slashes(path) ;

	DIR * dir;
	struct dirent * ent;
	std::vector<std::string> ret_vect ;

	if ((dir = opendir(path.c_str())) != NULL) {

		/* collect all the files and directories within directory */

		while ((ent = readdir (dir)) != NULL) {

			std::string file_name = std::string(ent->d_name) ;

			if (file_name != "." && file_name != "..") ret_vect.push_back(path + "/" + std::string(ent->d_name)) ;

		}
		closedir (dir);

	} else {

		/* could not open directory */
		perror ("");
	}

	return ret_vect ;
} ;

bool dir_is_empty(std::string path) {

	std::vector<std::string> ret_vect = get_dir_list(path) ;

	return ret_vect.empty() ;

}

std::string get_filename(std::string path) {

	if (!is_file(path)) { throw path + " is not a file" ; }

	int last_sep = path.rfind('/') ;

	if (last_sep == std::string::npos) { return path ; }

	return path.substr(last_sep + 1) ;
}

std::string get_file_stem(std::string path) {

	std::string file_name = get_filename(path) ;

	int first_dot = file_name.find('.') ;

	if (first_dot == std::string::npos) { return file_name ; }

	return file_name.substr(0, first_dot) ;
}

std::string remove_trailing_slashes(std::string path) {

	while (path.at(path.size() - 1) == '/') { path = path.substr(0, path.size() - 1) ; }

	return path ;
}

void make_dir(std::string path) {

	int rval = mkdir(path.c_str(),  0777) ;
	if (rval) throw "Could not make directory: " + path + "\n" ;
}
