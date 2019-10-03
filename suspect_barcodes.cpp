#include "suspect_barcodes.h"

int write_out_suspect_barcodes(Task<int, Write_out_suspect_barcodes_args> task) {

	// log activity
	cout << "GLOBAL : writing out suspect read ids\n" ;

	// parse args
	unordered_map<string, string>& suspect_barcodes_path_by_sample_key = task.args.suspect_barcodes_path_by_sample_key ;
	unordered_map<string, string>& sorted_barcode_read_ids_path_by_sample_key = task.args.sorted_barcode_read_ids_path_by_sample_key ;
	unordered_map<string, tuple<string, string>>& sample_name_project_name_by_sample_key = task.args.sample_name_project_name_by_sample_key ;
	function<string (string)> key_getter = task.args.key_getter ;

	// get sample keys
	vector<string> sample_key_vect ;
	for (unordered_map<string, string>::iterator it = suspect_barcodes_path_by_sample_key.begin(); 
		it != suspect_barcodes_path_by_sample_key.end(); ++it) {

		sample_key_vect.push_back(it->first) ;
	}

	// get ins, outs, keys, lines, and suspect counters
	unordered_map<string, Gzout*> gzout_ptr_by_sample_key ;
	unordered_map<string, Gzin*> gzin_ptr_by_sample_key ;
	unordered_map<string, string> key_by_sample_key ;
	unordered_map<string, string> line_by_sample_key ;
	unordered_map<string, int> line_counter_by_sample_key ;
	for (string sample_key : sample_key_vect) {

		// Gzin and Gzout
		Gzout * gzout_ptr = new Gzout(suspect_barcodes_path_by_sample_key.at(sample_key)) ;
		Gzin * gzin_ptr = new Gzin(sorted_barcode_read_ids_path_by_sample_key.at(sample_key)) ;

		if (!gzin_ptr->has_next_line()) throw runtime_error("passed an empty sorted barcode read ids file") ;

		// key and line
		string line = gzin_ptr->read_line() ;
		string key = key_getter(line) ;

		// make maps
		gzout_ptr_by_sample_key.insert(pair<string, Gzout*>(sample_key, gzout_ptr)) ;
		gzin_ptr_by_sample_key.insert(pair<string, Gzin*>(sample_key, gzin_ptr)) ;
		key_by_sample_key.insert(pair<string, string>(sample_key, key)) ;
		line_by_sample_key.insert(pair<string, string>(sample_key, line)) ;
		line_counter_by_sample_key.insert(pair<string, int>(sample_key, 0)) ;
	}

	while (sample_key_vect.size() > 1) {

		// get low key
		string low_key = key_by_sample_key.at(sample_key_vect.at(0)) ;
        for (string sample_key : sample_key_vect) { 
        	if (key_by_sample_key.at(sample_key) < low_key) { 
        		low_key = string(key_by_sample_key.at(sample_key)) ;
        	} 
        }

        // get samples
        vector<string> low_key_samples ;
		for (string sample_key : sample_key_vect) { 
        	if (key_by_sample_key.at(sample_key) == low_key) low_key_samples.push_back(sample_key) ;
        }

        // samples to remove from the sample_key_vect
        stack<string> to_evict ;

        // if there is only one sample get the next key for that sample and continue
        if (low_key_samples.size() == 1) {

        	string& sample_key = low_key_samples.at(0) ;

        	while (key_by_sample_key.at(sample_key) == low_key) {

        		// check if there are more lines for this sample
        		if (gzin_ptr_by_sample_key.at(sample_key)->has_next_line()) {

        			// get a new line
        			line_by_sample_key.at(sample_key) = gzin_ptr_by_sample_key.at(sample_key)->read_line() ;
					key_by_sample_key.at(sample_key) = key_getter(line_by_sample_key.at(sample_key)) ;

        		} else { 

        			// evict the sample
        			to_evict.push(sample_key) ;
        			break ;
        		}
        	}

        } else {

	        // print off suspects
	        for (int i = 0; i < low_key_samples.size(); i++) {

	        	string& sample_key = low_key_samples.at(i) ;

	        	while (key_by_sample_key.at(sample_key) == low_key) {

	        		// write the line
	        		gzout_ptr_by_sample_key.at(sample_key)->write_line(line_by_sample_key.at(sample_key)) ;
					line_counter_by_sample_key.at(sample_key)++ ;

	        		if (gzin_ptr_by_sample_key.at(sample_key)->has_next_line()) {

	        			// get a new line
	        			line_by_sample_key.at(sample_key) = gzin_ptr_by_sample_key.at(sample_key)->read_line() ;
						key_by_sample_key.at(sample_key) = key_getter(line_by_sample_key.at(sample_key)) ;

	        		} else { 

	        			// evict the sample
	        			to_evict.push(sample_key) ;
	        			break ;
	        		}
	        	}
	        }
        }

        // evict finished samples
        while (!to_evict.empty()) {

        	for (int i = 0; i < sample_key_vect.size(); i++) {

        		if (sample_key_vect.at(i) == to_evict.top()) {

        			sample_key_vect.erase(sample_key_vect.begin() + i) ;
        			break ;
        		}
        	}
        	to_evict.pop() ;
        }
	}

	// close gzouts, delete gzouts and gzins
	for (auto it = gzout_ptr_by_sample_key.begin(); it != gzout_ptr_by_sample_key.end(); ++it) {

		it->second->flush_close() ; delete it->second ;
	}
	for (auto it = gzin_ptr_by_sample_key.begin(); it != gzin_ptr_by_sample_key.end(); ++it) {

		delete it->second ;
	}

	// log number of suspect read ids found
	for (auto it = sample_name_project_name_by_sample_key.begin(); it != sample_name_project_name_by_sample_key.end(); ++it) {
		string log_msg = get<1>(it->second) + " - " + get<0>(it->second) + " : " ;
		log_msg += to_string(line_counter_by_sample_key.at(it->first)) + " suspect read ids written to " ;
		log_msg += suspect_barcodes_path_by_sample_key.at(it->first) + "\n" ;
		cout << log_msg ;
	}

	return 0 ;
}
