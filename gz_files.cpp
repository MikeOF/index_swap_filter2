#include <zlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "gz_files.h"

using namespace std ;

void gz_read_to_buffer(Gzins * gzins) {

	// read in new chars to buffer
	int bytes_read = gzread(gzins->gzfile, gzins->buffer, gzf_read_amount_bytes) ;

	gzins->buffer[bytes_read] = '\0' ; // add terminal char
}

void gz_parse_lines(Gzins * gzins) {

	if (! gzins->line_vect.empty()) throw "attempted to parse lines without an empty line_vect\n" ;

	char * line_start = gzins->buffer ;
	char * line_end = strchr(line_start, '\n') ;

	// get first line
	if (line_end != NULL) line_end[0] = '\0' ;
	string first_line = gzins->last_line_fragment + string(line_start) ; gzins->last_line_fragment = "" ;

	// return if we found nothing, store the first line and continue if we found something
	if (first_line.empty()) return ;
	gzins->line_vect.push_back(first_line) ;

	// parse more lines if necessary
	if (line_end == NULL) return ;
	line_start = line_end + 1 ;

	while(true) {

		// search for the next newline char
		line_end = strchr(line_start, '\n') ;

		if (line_end == NULL) {  // didn't find a new line char

			gzins->last_line_fragment = string(line_start) ;
			break ;
		}

		line_end[0] = '\0' ; 

		gzins->line_vect.push_back(string(line_start)) ;

		line_start = line_end + 1 ; 
	}
}

void gz_read_lines(Gzins * gzins) {

	gzins->line_vect.clear() ;  // clear out any old lines

	if (gzins->finished) return ;

	// begin the gz read process if necessary
	if (! gzins->begun) {

		gzins->begun = true ;

		// open the file
		gzins->gzfile = gzopen(gzins->file_path.c_str(), "r") ;
		if (gzins->gzfile == NULL) throw "could not open gzipped file, " + gzins->file_path + "\n" ;

		// set the internal gz buffer
		bool check = gzbuffer(gzins->gzfile, gzf_internal_buffer_bytes) ;
		if (check) throw "could not set the zlib buffer\n" ;
	} 

	gz_read_to_buffer(gzins) ;

	gz_parse_lines(gzins) ;

	// determine if the gz file is finished
	if (gzeof(gzins->gzfile)) {

		gzins->finished = true ; 
		gzclose(gzins->gzfile) ;
	}
}

void gz_write_lines(Gzouts * gzouts) {

	// begin the gz read process if necessary
	if (! gzouts->begun) {

		gzouts->begun = true ;

		// open the file
		gzouts->gzfile = gzopen(gzouts->file_path.c_str(), "w") ;
		if (gzouts->gzfile == NULL) throw "could not open gzipped file, " + gzouts->file_path + "\n" ;

		// set the internal gz buffer
		bool check = gzbuffer(gzouts->gzfile, gzf_internal_buffer_bytes) ;
		if (check) throw "could not set the zlib buffer\n" ;

		// set the buffer terminal char
		gzouts->buffer_last_byte = gzouts->buffer + gzf_char_buffer_size_bytes - 1 ;
		gzouts->buffer_last_byte[0] = '\0' ;
		gzouts->buffer_position = gzouts->buffer ;
	} 

	// fill the out buffer
	for (string line : gzouts->line_vect) {

		cout << "hello line size: " << line.size() << endl ;

		int remaining_bytes = gzouts->buffer_last_byte - gzouts->buffer_position ;
		cout << "remaining byes: " << remaining_bytes << endl ;

		// does this line fit in the buffer
		if (remaining_bytes < line.size() + 1) {

			// fill remainder of buffer
			strncpy(gzouts->buffer_position, line.c_str(), remaining_bytes) ; 

			// write out the buffer
			gzwrite(gzouts->gzfile, gzouts->buffer, gzf_char_buffer_size_bytes) ;

			line = line.substr(remaining_bytes) ;

			gzouts->buffer_position = gzouts->buffer ;
		}

		strcpy(gzouts->buffer_position, line.c_str()) ;
		gzouts->buffer_position[line.size()] = '\n' ;
		gzouts->buffer_position += line.size() + 1 ;
	}

	// clear out processed lines
	gzouts->line_vect.clear() ;

	// add a terminating null
	gzouts->buffer_position[0] = '\0' ;
}

void gz_flush_close(Gzouts * gzouts) {

	// begin the gz read process if necessary
	if (! gzouts->begun) throw "attempted to flush and close an unopen gzouts\n" ;

	// flush the line vect
	gz_write_lines(gzouts) ;

	// flush the buffer
	int bytes_to_write ;
	char * terminal_char_ptr = strchr(gzouts->buffer, '\0') ;
	if (terminal_char_ptr == NULL) bytes_to_write = gzf_char_buffer_size_bytes ;
	else bytes_to_write = terminal_char_ptr - gzouts->buffer ;
	gzwrite(gzouts->gzfile, gzouts->buffer, bytes_to_write) ;

	// finally flush and close the gzstream
	gzclose(gzouts->gzfile) ;
}

int main(int argc, char ** argv) {

	Gzins gzins ;
	gzins.file_path = string(argv[1]) ;

	Gzouts gzouts ;
	gzouts.file_path = string(argv[2]) ;

	while (true) {

		gz_read_lines(&gzins) ;

		for (string line : gzins.line_vect) {
			gzouts.line_vect.push_back(line) ;
		}

		gz_write_lines(&gzouts) ;

		if (gzins.finished) break ;
	}

	gz_flush_close(&gzouts) ;
}