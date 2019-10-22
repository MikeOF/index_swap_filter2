#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <sstream>
#include <string>

#include "sample.h"

using namespace std ;

const string GLOBAL_LOG_HEADER = "GLOBAL : " ;

inline string get_sample_log_header(Sample& sample) {
	stringstream ss ;
	ss << sample.get_project_name() ;
	ss << " - " ;
	ss << sample.get_sample_name() ;
	ss << " : " ;
	return ss.str() ;
}

inline void log_message(const string& msg) { cout << msg << flush ; }
inline void log_error_message(const string& msg) { cerr << msg << flush ; }

#endif