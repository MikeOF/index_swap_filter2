#include "whitelist.h"

using namespace std ;

Whitelist::Whitelist (string whitelist_path) {

	string line ;
	ifstream wl_file (whitelist_path) ;

	if (wl_file.is_open()) {

		  while (getline(wl_file, line)) {

		  	if (whitelist_set.count(line) > 0) { 
		  		cout << "collision on " << line << endl ; 
		  		wl_file.close() ; return ; 
		  	}

		  	whitelist_set.insert(line) ;
		  }

		wl_file.close() ;
	}

	else  throw runtime_error("unable to open file: " + whitelist_path) ;
}

string Whitelist::get_valid_barcode(const string& in_query) {

	if (whitelist_set.count(in_query) == 1) { return in_query ; }

    // create a copy since we are using replace 
    string query = in_query ;

	// valid bases
	string bases [] = {"A", "G", "C", "T"} ;

	// to record match results
    int matches = 0 ; string match; 

    int first_N = query.find_first_of('N') ;
    if (first_N != string::npos) {

        if (first_N == query.find_last_of('N')) {  // make sure there is only 1 N

            for (int j = 0; j < 4; j++) {  // cycle through possible bases

                query.replace(first_N, 1, bases[j]) ;

                if (whitelist_set.count(query) == 1) {  // search whitelist
                    match = string(query) ; 
                    matches++ ;
                }
            }
        }

    } else {

        for (int i = 0; i < query.length(); i++) {

            string original = query.substr(i, 1) ;

            for (int j = 0; j < 4; j++) {

            	if (original.compare(bases[j]) != 0) {  // ignore original query
		           	
		           	query.replace(i, 1, bases[j]) ;

	                if (whitelist_set.count(query) == 1) {  // search whitelist
	                    match = string(query) ; 
	                    matches++ ;
	                }
            	}
            }
            query.replace(i, 1, original) ; // reset query for next i position
        }
    }

    if (matches > 1 || matches == 0) { return "" ; }  // didn't find a unique match

    return match ;
}
