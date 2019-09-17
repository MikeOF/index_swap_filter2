#ifndef FILES_H
#define FILES_H

#include <zlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

using namespace std ;

const int gzf_read_amount_bytes = 128000 ;
const int gzf_char_buffer_size_bytes = gzf_read_amount_bytes + 1 ;
const int gzf_internal_buffer_bytes = 256000 ;

struct Gzins {
	string file_path ;
	gzFile gzfile ;
	bool begun = false ;
	bool finished = false ;
	vector<string> line_vect ;
	string last_line_fragment = "" ;
	char buffer [gzf_char_buffer_size_bytes] ;
} ;

struct Gzouts {
	string file_path ;
	gzFile gzfile ;
	bool begun = false ;
	bool finished = false ;
	vector<string> line_vect ;
	char buffer [gzf_char_buffer_size_bytes] ;
	char * buffer_position ;
} ;

void gz_read_lines(Gzins *) ;
void gz_write_lines(Gzouts *) ;
void gz_flush_close(Gzouts *) ;

#endif