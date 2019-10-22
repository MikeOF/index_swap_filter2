#ifndef LOG_H
#define LOG_H

#include <iostream>

using namespace std ;

const string GLOBAL_LOG_HEADER = "GLOBAL : " ;

log_message(string& msg) { cout << msg << flush ; }
log_error_message(string& msg) { cerr << msg << flush ; }

#endif