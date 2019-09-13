#include "files.h"

#define LENGTH 0x1000

int main(int argc, char ** argv) {

	// parse and validate arguments

	if (argc != 2) { std::cout << "your should pass 1 path argument\n" ; return 1 ; }

	struct stat buf ;

	stat(argv[1], &buf);

	mode_t mode = buf.st_mode ;

	std::cout << mode << std::endl ;

	std::cout << argv[1] << std::endl ;

	if (S_ISDIR(mode)) { std::cout << "it is a dir" << std::endl ; }
	if (S_ISREG(mode)) { 
		std::cout << "it is a regular file" << std::endl ;

		std::string fpath = std::string(argv[1]) ;

		if (fpath.find(".gz") != std::string::npos) {

		    gzFile file ;
		    file = gzopen (fpath.c_str(), "r") ;

			while (1) {

				int err;                    
				int bytes_read ;
				unsigned char buffer[LENGTH] ;
				bytes_read = gzread (file, buffer, LENGTH - 1) ;
				buffer[bytes_read] = '\0' ;
				printf ("%s", buffer) ;

				if (bytes_read < LENGTH - 1) {
					if (gzeof (file)) {
						break;
					} else {
						const char * error_string;
						error_string = gzerror (file, & err);
						if (err) {
							fprintf (stderr, "Error: %s.\n", error_string);
							exit (EXIT_FAILURE);
						}
					}
			}
		}

		gzclose (file) ;

		}
	}
	if (S_ISLNK(mode)) { std::cout << "it is a link" << std::endl ; }

	return 0;
}