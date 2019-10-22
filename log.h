#ifndef LOG_H
#define LOG_H

#include <iostream>

using namespace std ;

const string GLOBAL_LOG_HEADER = "GLOBAL : " ;

string get_sample_log_header(Sample& sample) {
	return sample.get_project_name() + " - " + sample.get_sample_name() + " : " ;
}

log_message(string& msg) { cout << msg << flush ; }
log_error_message(string& msg) { cerr << msg << flush ; }

#endif