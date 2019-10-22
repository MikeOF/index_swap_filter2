#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <sstream>

using namespace std ;

const string GLOBAL_LOG_HEADER = "GLOBAL : " ;

string get_sample_log_header(Sample& sample) {
	stringstream ss << sample.get_project_name() ;
	ss << " - " ;
	ss << sample.get_sample_name() ;
	ss << " : " ;
	return ss.str() ;
}

void log_message(string& msg) { cout << msg << flush ; }
void log_error_message(string& msg) { cerr << msg << flush ; }

#endif