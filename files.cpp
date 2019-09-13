#include "files.h"

int main(int argc, char ** argv) {

	// parse and validate arguments

	if (argc != 2) { std::cout << "your should pass 1 path argument\n" ; return 1 ; }

	struct stat buf ;

	stat(argv[1], &buf);

	mode_t mode = buf.st_mode ;

	std::cout << mode << std::endl ;

	std::cout << argv[1] << std::endl ;

	if (S_ISDIR(mode)) { std::cout << "it is a dir" << std::endl ; }
	if (S_ISREG(mode)) { std::cout << "it is a regular file" << std::endl ; }
	if (S_ISLNK(mode)) { std::cout << "it is a link" << std::endl ; }

	return 0;
}