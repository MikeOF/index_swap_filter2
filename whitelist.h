#ifndef WHITELIST_H
#define WHITELIST_H

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
 
class Whitelist {

	// this is our hash set
	std::unordered_set<std::string> whitelist_set ;

	public:

		Whitelist (std::string) ;

		std::string get_valid_barcode(std::string) ;

};
 
#endif
