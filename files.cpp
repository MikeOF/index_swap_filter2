#include "files.h"

#define GZBUFFER_LENGTH 0x1000

int get_next(char * buffer, char c, int pos, int length) {
	while (pos < length) {
		std::cout << "hello\n" ; 
		if (buffer[pos] == c) {
			return pos ;
		}
		pos++ ;
	}
	return -1 ;
}

int main(int argc, char ** argv) {

	// parse and validate arguments

	if (argc != 2) { std::cout << "your should pass 1 path argument\n" ; return 1 ; }

	struct stat buf ;

	stat(argv[1], &buf);

	mode_t mode = buf.st_mode ;

	std::cout << mode << std::endl ;

	std::cout << argv[1] << std::endl ;

	// std::cout << "next - " << get_next(argv[1], '.', 0, 20) << std::endl ;

	if (S_ISDIR(mode)) { std::cout << "it is a dir" << std::endl ; }
	if (S_ISREG(mode)) { 
		std::cout << "it is a regular file" << std::endl ;

		std::string fpath = std::string(argv[1]) ;

		if (fpath.find(".gz") != std::string::npos) {

	        gzFile file ; 
	        int err, bytes_read ; 
	        unsigned char in_buffer[GZBUFFER_LENGTH] ; 

	        file = gzopen(fpath.c_str(),"r") ;


    		char ** lines[4] ; 
    		int line_cnt = 0 ;
    		int buffer_start = 0 ;
	        bool more_to_read = true ;
	        while(more_to_read) {

	        	bytes_read = gzread (file, in_buffer, GZBUFFER_LENGTH - 1 - buffer_start) ;
				in_buffer[bytes_read] = '\0' ;
				more_to_read = ! gzeof (file) ;



				printf("%s", in_buffer) ;

				more_to_read = ! gzeof (file) ;


	        }

			// while (1) {

			// 	int err;                    
			// 	int bytes_read ; 
			// 	unsigned char buffer[GZBUFFER_LENGTH] ;
			// 	bytes_read = gzread (file, buffer, GZBUFFER_LENGTH - 1) ;
			// 	buffer[bytes_read] = '\0' ;
			// 	printf ("%s", buffer) ;

			// 	if (bytes_read < GZBUFFER_LENGTH - 1) {
			// 		if (gzeof (file)) {
			// 			break;
			// 		} else {
			// 			const char * error_string;
			// 			error_string = gzerror (file, & err);
			// 			if (err) {
			// 				fprintf (stderr, "Error: %s.\n", error_string);
			// 				exit (EXIT_FAILURE);
			// 			}
			// 		}
			// }

			gzclose (file) ;
		}

		
	}
	if (S_ISLNK(mode)) { std::cout << "it is a link" << std::endl ; }

	return 0;
}