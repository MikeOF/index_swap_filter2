#ifndef GZ_FILES_H
#define GZ_FILES_H

#include <zlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "files.h"

using namespace std ;

const int gzf_read_amount_bytes = 256000 ;
const int gzf_char_buffer_size_bytes = gzf_read_amount_bytes + 1 ;
const int gzf_internal_buffer_bytes = 512000 ;

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
} ;

struct Gzouts {
	string file_path ;
	gzFile gzfile ;
	bool begun = false ;
	bool finished = false ;
} ;

Gzouts gz_get_gzouts(string) ;
void gz_begin_gzouts(Gzouts *) ;
void gz_write_line(Gzouts *, string) ;
void gz_flush_close(Gzouts *) ;

#endif