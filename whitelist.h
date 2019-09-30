#ifndef WHITELIST_H
#define WHITELIST_H

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>

using namespace std ;
 
class Whitelist {

	// this is our hash set
	unordered_set<string> whitelist_set ;

	public:

		Whitelist (string) ;

		string get_valid_barcode(const string&) ;

};
 
#endif
