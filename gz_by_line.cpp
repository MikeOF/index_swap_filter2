#include <zlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
using namespace std ;

struct Lines {
	vector<string> line_vect ;
	string last_line_fragment ;
} ;

void get_lines(char * buffer, Lines * lines, int length) {

	int bytes_parsed = 0 ;
	char * prev_new_line_start = buffer ;
	char * new_line_ptr ;
	while(true) {

		new_line_ptr = strchr(prev_new_line_start, '\n') ;

		if (new_line_ptr == NULL) break ;

		bytes_parsed += new_line_ptr - prev_new_line_start + 1 ;

		new_line_ptr[0] = '\0' ; 

		lines->line_vect.push_back(string(prev_new_line_start)) ;

		prev_new_line_start = new_line_ptr + 1 ; 
	}

	if (bytes_parsed < length) {
		lines->last_line_fragment = string(prev_new_line_start) ;
	} else {
		lines->last_line_fragment = "" ;
	}
}

int main(int argc, char ** argv) {

	int ret, flush, check, amount_read;

	gzFile gzfile = gzopen (argv[1], "r") ;

	check = gzbuffer(gzfile, 128000) ; if (check) throw "could not set the zlib buffer\n" ;

	Lines lines ;

	int read_amount = 64000 ;
	int buffer_size = (read_amount * 2) + 1 ;
	int load_from = 0 ;
	char buffer [buffer_size] ;
	buffer[read_amount + 1] = '\0' ;
	while (true) {

		amount_read = gzread(gzfile, &buffer, read_amount) ;

		if (amount_read == 0 && gzeof(gzfile)) { break ; }

		buffer[amount_read + 1] = '\0' ;

		get_lines(buffer, &lines, read_amount) ;

		for (string line : lines.line_vect) {
			std::cout << line << std::endl ;
		}

		lines.line_vect.clear() ;

		if (!lines.last_line_fragment.empty()) {
			strcpy(buffer, lines.last_line_fragment.c_str()) ; 
			load_from = lines.last_line_fragment.size() ;
			lines.last_line_fragment = "" ;
		}

	}
}