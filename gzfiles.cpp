#include "gzfiles.h"

using namespace std ;

Gzin::Gzin (const string& file_path) {

	// check the file path 
	this->file_path = string(file_path) ;
	if (!Path(this->file_path).is_file()) runtime_error("file not found, " + this->file_path) ;

	// open the file
	this->gzfile  = gzopen(this->file_path.c_str(), "r") ;
	if (this->gzfile == NULL) {
		throw runtime_error("could not open gzipped file, " + this->file_path) ;
	}

	// set the internal gz buffer
	bool check = gzbuffer(this->gzfile, Gzin::internal_buffer_bytes) ;
	if (check) throw runtime_error("could not set the internal gzFile buffer") ;
	
	// initialize control variables
	this->finished = false ;
	this->has_next = false ;

	this->line_index = 0 ;
	this->last_line_fragment = "" ;

	this->buffer_bytes = 0 ;

	// initialize the buffer
	this->buffer = new char[Gzin::char_buffer_size_bytes] ;

	// read in the first set of bytes
	this->read_to_buffer() ;

	// read the first set of lines
	this->read_lines() ;

	// determine if we have lines
	if (this->line_vect.size() > 1 || !this->last_line_fragment.empty()) {
		this->has_next = true ;
	}
}

Gzin::~Gzin () { 

	if (!this->finished) gzclose(this->gzfile) ;
	delete this->buffer ; 
}

void Gzin::read_to_buffer() {

	if (this->finished) throw runtime_error("attempted to read from a finished Gzins") ;

	// read in new chars to buffer
	this->buffer_bytes = gzread(this->gzfile, this->buffer, Gzin::read_amount_bytes) ;

	if (this->buffer_bytes < 0) throw runtime_error("error reading from gz file, " + this->file_path) ;

	// add terminal char
	this->buffer[this->buffer_bytes] = '\0' ;

	// determine if the gz file is finished
	if (gzeof(this->gzfile)) { 
		this->finished = true ; 
		gzclose(this->gzfile) ; 
	}
}

void Gzin::parse_lines() {

	// check state of the line vect
	if (!this->line_vect.empty()) {
		throw runtime_error("attempted to parse lines without an empty line_vect") ;
	}

	// record how many bytes we expect to parse here
	int bytes_to_parse = this->buffer_bytes ;

	// set the buffer bytes to 0 since we're going to parse them
	this->buffer_bytes = 0 ;

	// initialize the line start and end to first line values
	char * line_start = this->buffer ;
	char * line_end = strchr(line_start, '\n') ;

	// mark the end of the first line if necessary ;
	if (line_end != NULL) line_end[0] = '\0' ;

	// parse first set of bytes
	string first_string = string(line_start) ; 

	// return if we didn't find any new characters
	if (first_string.empty() && bytes_to_parse < 2) {

		if (!this->last_line_fragment.empty()) {
			this->line_vect.push_back(this->last_line_fragment) ;
			this->last_line_fragment = "" ;
		}
		return ;
	}

	// return if we didn't find a newline character
	if (line_end == NULL) {
		this->last_line_fragment = this->last_line_fragment + first_string ;
		return ;
	}

	// At this point we have found a new line
	// store the first line and continue
	this->line_vect.push_back(this->last_line_fragment + first_string) ;

	// parse more lines
	line_start = line_end + 1 ;
	while(true) {

		// search for the next newline char
		line_end = strchr(line_start, '\n') ;

		if (line_end == NULL) {  // didn't find a new line char
			this->last_line_fragment = string(line_start) ;
			break ;
		}

		line_end[0] = '\0' ; 

		this->line_vect.emplace_back(line_start) ;

		line_start = line_end + 1 ; 
	}
}

void Gzin::read_lines() {

	this->line_vect.clear() ;  // clear out any old lines

	// parse the lines from the buffer
	this->parse_lines() ;

	// fill local character buffer
	if (!this->finished) this->read_to_buffer() ;
}

string Gzin::read_line() {

	// check the status
	if (!this->has_next) throw runtime_error("attempted to read from a Gzin without more lines") ;

	// if we need to fetch more lines into the line vect, do so here
	while (this->line_index == this->line_vect.size()) {

		this->read_lines() ; this->line_index = 0 ;
	}

	this->line_index++ ;

	// if the file is finished and this is the last line, mark that we don't have a next line
	if (this->finished 
		&& this->line_index == this->line_vect.size() 
		&& this->last_line_fragment.empty()
		&& this->buffer_bytes == 0) {
		
		this->has_next = false ;
	}

	return this->line_vect.at(this->line_index - 1) ;
}

Gzout::Gzout(const string& file_path) {

	this->file_path = string(file_path) ;

	// open the file
	this->gzfile = gzopen(this->file_path.c_str(), "w") ;
	if (this->gzfile == NULL) throw runtime_error("could not open gzipped file, " + this->file_path) ;

	// set the internal gz file buffer
	bool check = gzbuffer(this->gzfile, Gzout::internal_buffer_bytes) ;
	if (check) throw runtime_error("could not set the zlib buffer") ;
} 

void Gzout::write_line(const string& line) {

	// check the status
	if (this->finished) throw runtime_error("attempted to write with a finished Gzouts") ;

	// send line the the gz file
	gzwrite(this->gzfile, line.c_str(), line.size()) ;

	// add the newline character
	char newline = '\n' ; gzwrite(this->gzfile, &newline, 1) ;
}

void Gzout::flush_close() {

	// begin the gz read process if necessary
	if (this->finished) throw runtime_error("attempted to flush a finished Gzouts") ;

	// flush and close the gzstream
	gzclose(this->gzfile) ;

	// mark the gzouts finished
	this->finished = true ;
}
