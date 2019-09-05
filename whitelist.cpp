#include "whitelist.h"

Whitelist::Whitelist (std::string whitelist_path) {

	std::string line ;
	std::ifstream wl_file (whitelist_path) ;

	if (wl_file.is_open()) {

		  while (getline (wl_file, line)) {

		  	if (whitelist_set.count (line) > 0) { 
		  		std::cout << "collision on " << line << std::endl ; 
		  		wl_file.close() ; return ; 
		  	}

		  	whitelist_set.insert ( line ) ;

		  }

		wl_file.close() ;
	}

	else  printf("unable to open file: %s", whitelist_path.c_str()) ;
}

std::string Whitelist::get_valid_barcode(std::string query) {

	if (whitelist_set.count(query) > 0) { return query ; }

	// valid bases
	std::string bases [] = {"A", "G", "C", "T"} ;

	// to record match results
    int matches = 0 ; std::string match; 

    int first_N = query.find_first_of('N') ;
    if (first_N != std::string::npos) {

        if (first_N == query.find_last_of('N')) {  // make sure there is only 1 N

            for (int j = 0; j < 4; j++) {  // cycle through possible bases

                query.replace(first_N, 1, bases[j]) ;

                if (whitelist_set.count(query) == 1) {  // search whitelist
                    match = query.substr() ; 
                    matches++ ;
                }
            }

            query.replace(first_N, 1, "N") ;
        }

    } else {

        for (int i = 0; i < query.length(); i++) {

            std::string original = query.substr(i, 1) ;

            for (int j = 0; j < 4; j++) {

            	if (original.compare(bases[j]) != 0) {  // ignore original query
		           	
		           	query.replace(i, 1, bases[j]) ;

	                if (whitelist_set.count(query) == 1) {  // search whitelist
	                    match = query.substr() ; 
	                    matches++ ;
	                }
            	}
            }

            query.replace(i, 1, original) ;  // reset the query
        }
    }

    if (matches > 1 || matches == 0) { return "" ; }

    return match ;

}
