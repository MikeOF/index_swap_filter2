#include "gz_files.h"

using namespace std ;

void gz_begin_gzins(Gzins * gzins) {

	// check the status
	if (gzins->begun) throw runtime_error("attempted to begin a Gzins twice") ;

	gzins->begun = true ;

	// open the file
	gzins->gzfile = gzopen(gzins->file_path.c_str(), "r") ;
	if (gzins->gzfile == NULL) throw runtime_error("could not open gzipped file, " + gzins->file_path) ;

	// set the internal gz buffer
	bool check = gzbuffer(gzins->gzfile, gzf_internal_buffer_bytes) ;
	if (check) throw runtime_error("could not set the internal gzFile buffer") ;

	// read in the first set of lines
	gz_read_lines(gzins) ;

	// determine if we have lines
	if (gzins->line_vect.size() > 1 || gzins->last_line_fragment.size() > 1) {
		gzins->has_next = true ;
	}
}

void gz_read_to_buffer(Gzins * gzins) {

	// read in new chars to buffer
	int bytes_read = gzread(gzins->gzfile, gzins->buffer, gzf_read_amount_bytes) ;

	// add terminal char
	gzins->buffer[bytes_read] = '\0' ;
}

void gz_parse_lines(Gzins * gzins) {

	// check state of the line vect
	if (!gzins->line_vect.empty()) {
		throw runtime_error("attempted to parse lines without an empty line_vect") ;
	}

	// initialize the line start and end to first line values
	char * line_start = gzins->buffer ;
	char * line_end = strchr(line_start, '\n') ;

	// mark the end of the first line if necessary ;
	if (line_end != NULL) line_end[0] = '\0' ;

	// parse first set of bytes
	string first_string = string(line_start) ; 

	// return if we didn't find any new characters
	if (first_string.empty()) {

		gzins->line_vect.push_back(gzins->last_line_fragment) ;
		gzins->last_line_fragment = "" ;
		return ;
	}

	// return if we didn't find a newline character
	if (line_end == NULL) {
		gzins->last_line_fragment = gzins->last_line_fragment + first_string ; 
		return ;
	}

	//  store the first line and continue
	gzins->line_vect.push_back(gzins->last_line_fragment + first_string) ;

	// parse more lines
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

	// check the status
	if (!gzins->begun) throw runtime_error("attempted to read from an un-begun Gzins") ;
	if (gzins->finished) throw runtime_error("attempted to read from a finished Gzins") ;

	gzins->line_vect.clear() ;  // clear out any old lines

	// fill local character buffer
	gz_read_to_buffer(gzins) ;

	// parse the lines from the buffer
	gz_parse_lines(gzins) ;

	// determine if the gz file is finished
	if (gzeof(gzins->gzfile)) {

		gzins->finished = true ; 
		gzclose(gzins->gzfile) ;
		break ;
	}
}

string& gz_read_line(Gzins * gzins) {

	// check the status
	if (!gzins->begun) throw runtime_error("attempted to read from an un-begun Gzins") ;
	if (!gzins->has_next) throw runtime_error("attempted to read from a Gzins without more lines") ;

	while (gzins->line_index == gzins->line_vect.size()) {

		if (gzins->finished) gzins->has_next = false ; return ;

		gz_read_lines(Gzins * gzins) ;
		gzins->line_index = 0 ;
	}

	gzins->line_index++ ;

	return gzins->line_vect.at(gzins->line_index - 1) ;
}

Gzouts gz_get_gzouts(string file_path) {
	Gzouts gzouts ;
	gzouts.file_path = file_path ;
	return gzouts ;
}

void gz_begin_gzouts(Gzouts * gzouts) {

	// check the status
	if (gzouts->begun) throw runtime_error("attempted to begin a Gzouts twice") ;

	gzouts->begun = true ;

	// open the file
	gzouts->gzfile = gzopen(gzouts->file_path.c_str(), "w") ;
	if (gzouts->gzfile == NULL) throw "could not open gzipped file, " + gzouts->file_path + "\n" ;

	// set the internal gz file buffer
	bool check = gzbuffer(gzouts->gzfile, gzf_internal_buffer_bytes) ;
	if (check) throw "could not set the zlib buffer\n" ;
}

void gz_write_line(Gzouts * gzouts, string line) {

	// check the status
	if (!gzouts->begun) throw runtime_error("attempted to write to an un-begun Gzouts") ;
	if (gzouts->finished) throw runtime_error("attempted to write with a finished Gzouts") ;

	// send line the the gz file
	gzwrite(gzouts->gzfile, line.c_str(), line.size()) ;

	// add the newline character
	char newline = '\n' ;
	gzwrite(gzouts->gzfile, &newline, 1) ;
}

void gz_flush_close(Gzouts * gzouts) {

	// begin the gz read process if necessary
	if (!gzouts->begun) throw runtime_error("attempted to flush and close an unopen Gzouts") ;
	if (gzouts->finished) throw runtime_error("attempted to flush a finished Gzouts") ;

	// flush and close the gzstream
	gzclose(gzouts->gzfile) ;

	// mark the gzouts finished
	gzouts->finished = true ;
}
