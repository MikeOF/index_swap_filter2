#ifndef GZFILE_H
#define GZFILE_H

#include <zlib.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <cstring>
#include <unordered_set>
#include <stdexcept>

#include "path.h"

using namespace std ;

class Gzin {

	static const int read_amount_bytes = 256000 ;
	static const int char_buffer_size_bytes = 256000 + 1 ;
	static const int internal_buffer_bytes = 512000 ;

	string file_path ;
	gzFile gzfile ;

	bool finished ;
	bool has_next ;

	int line_index ;
	vector<string> line_vect ;
	string last_line_fragment ;

	int buffer_bytes ;
	char * buffer ;

	void read_to_buffer() ;
	void parse_lines() ;
	void read_lines() ;

	public:

	    Gzin(const string&) ;

	    Gzin(const Gzin&) = delete ; // disable copy construction

	    ~Gzin() ;

		string read_line() ;
		bool has_next_line() const { return has_next ; }
};

class Gzout {

	static const int internal_buffer_bytes = 512000 ;

	string file_path ;
	gzFile gzfile ;
	bool finished = false ;

	public:

		Gzout() ;
		Gzout(const string&) ;

		void write_line(const string&) ;
		void flush_close() ;
};

const int GLOBAL_GZFILE_INTERNAL_BUFFER_BYTES = 512000 ;
const int GLOBAL_GZFILE_CHAR_BUFFER_BYTES = 256000 ;
void combine_gz_files(string&, vector<string>&) ;
void copy_gz_file(string, string) ;

#endif