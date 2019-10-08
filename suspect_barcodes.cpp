#include "suspect_barcodes.h"

void write_out_suspect_barcodes(unordered_map<string, Sample>& samples, Workdir& workdir) {

	// create suspect task
	Task<unordered_map<string,vector<string>>, Write_out_suspect_bcsnrid_lines_args> write_out_suspect_bcsnrid_lines_task ;
	write_out_suspect_bcsnrid_lines_task.func = write_out_suspect_bcsnrid_lines_task_func ;

	// for each sample key
	for (string sample_key : workdir.get_sample_keys()) {

		// get the dir to write suspect chunks to
		write_out_suspect_bcsnrid_lines_task.args.suspect_bcsnrid_chunks_path_by_sample_key.insert(
			make_pair(sample_key, workdir.get_suspect_bcsnrid_chunks_path(sample_key))) ;

		// get the sorted barcode read ids path to read from
		write_out_suspect_bcsnrid_lines_task.args.bcsnrid_path_by_sample_key.insert(
			make_pair(sample_key, workdir.get_bcsnrid_path(sample_key))) ; 

		// get the sample pointer
		write_out_suspect_bcsnrid_lines_task.args.sample_ptr_by_sample_key.insert(
			make_pair(sample_key, &(samples.at(sample_key))))  ;
	}
	stack<Task<unordered_map<string,vector<string>>, Write_out_suspect_bcsnrid_lines_args>> write_task_stack ; 
	write_task_stack.push(write_out_suspect_bcsnrid_lines_task) ;	

	stack<unordered_map<string,vector<string>>> supect_bcsnrid_chunk_paths_by_sample_key_stack  = run_tasks(1, write_task_stack) ;
	unordered_map<string,vector<string>> supect_bcsnrid_chunk_paths_by_sample_key = supect_bcsnrid_chunk_paths_by_sample_key_stack.top();

	// create suspect collection task stack
	stack<Task<int, Collect_suspect_bcsnrid_lines_args>> collection_task_stack ;
	for (auto it = supect_bcsnrid_chunk_paths_by_sample_key.begin(); 
		it != supect_bcsnrid_chunk_paths_by_sample_key.end(); ++it) {

		Task<int, Collect_suspect_bcsnrid_lines_args> task ;
		task.func = collect_suspect_bcsnrid_lines_task_func ;
		task.args.suspect_bcsnrid_chunk_paths = it->second ;
		task.args.suspect_bcsnrid_path = workdir.get_suspect_bcsnrid_path(it->first);
		task.args.sample_ptr = &(samples.at(it->first));
	}
	run_tasks(1, collection_task_stack) ;
}

unordered_map<string,vector<string>> write_out_suspect_bcsnrid_lines_task_func(
	Task<unordered_map<string,vector<string>>, Write_out_suspect_bcsnrid_lines_args> task) {

	// log activity
	cout << "GLOBAL : writing out suspect barcode seq-num read ids\n" ;

	// get argument references
	unordered_map<string, string>& suspect_bcsnrid_chunks_path_by_sample_key = task.args.suspect_bcsnrid_chunks_path_by_sample_key ;
	unordered_map<string, string>& bcsnrid_path_by_sample_key = task.args.bcsnrid_path_by_sample_key ;
	unordered_map<string, Sample*>& sample_ptr_by_sample_key = task.args.sample_ptr_by_sample_key ;

	// get sample keys
	vector<string> sample_key_vect ;
	for (auto it = sample_ptr_by_sample_key.begin(); it != sample_ptr_by_sample_key.end(); ++it) {
		sample_key_vect.push_back(it->first) ;
	}

	// get ins, outs, barcodes, lines, and suspect counters
	unordered_map<string, GzChunkSortWriter<int>*> gzcsout_ptr_by_sample_key ;
	unordered_map<string, Gzin*> gzin_ptr_by_sample_key ;
	unordered_map<string, string> barcode_by_sample_key ;
	unordered_map<string, string> line_by_sample_key ;
	unordered_map<string, int> line_counter_by_sample_key ;
	for (string sample_key : sample_key_vect) {

		// Gzin and GzChunkSortWriter
		GzChunkSortWriter<int> * gzcsout_ptr = new GzChunkSortWriter<int>(suspect_bcsnrid_chunks_path_by_sample_key.at(sample_key)) ;
		Gzin * gzin_ptr = new Gzin(bcsnrid_path_by_sample_key.at(sample_key)) ;

		if (!gzin_ptr->has_next_line()) throw runtime_error("passed an empty sorted barcode read ids file") ;

		// barcode and line
		string line = gzin_ptr->read_line() ;
		string barcode = get_barcode_from_bcsnrid_line(line) ;

		// make maps
		gzcsout_ptr_by_sample_key.insert(make_pair(sample_key, gzcsout_ptr)) ;
		gzin_ptr_by_sample_key.insert(make_pair(sample_key, gzin_ptr)) ;
		barcode_by_sample_key.insert(make_pair(sample_key, barcode)) ;
		line_by_sample_key.insert(make_pair(sample_key, line)) ;
		line_counter_by_sample_key.insert(make_pair(sample_key, 0)) ;
	}

	while (sample_key_vect.size() > 1) {

		// get low barcode
		string low_barcode = barcode_by_sample_key.at(sample_key_vect.at(0)) ;
        for (string sample_key : sample_key_vect) { 
        	if (barcode_by_sample_key.at(sample_key) < low_barcode) { 
        		low_barcode = string(barcode_by_sample_key.at(sample_key)) ;
        	} 
        }

        // get samples
        vector<string> low_barcode_samples ;
		for (string sample_key : sample_key_vect) { 
        	if (barcode_by_sample_key.at(sample_key) == low_barcode) low_barcode_samples.push_back(sample_key) ;
        }

        // samples to remove from the sample_key_vect
        stack<string> to_evict ;

        // if there is only one sample get the next key for that sample and continue
        if (low_barcode_samples.size() == 1) {

        	string& sample_key = low_barcode_samples.at(0) ;

        	while (barcode_by_sample_key.at(sample_key) == low_barcode) {

        		// check if there are more lines for this sample
        		if (gzin_ptr_by_sample_key.at(sample_key)->has_next_line()) {

        			// get a new line
        			line_by_sample_key.at(sample_key) = gzin_ptr_by_sample_key.at(sample_key)->read_line() ;
					barcode_by_sample_key.at(sample_key) = get_barcode_from_bcsnrid_line(line_by_sample_key.at(sample_key)) ;

        		} else { 

        			// evict the sample
        			to_evict.push(sample_key) ;
        			break ;
        		}
        	}

        } else {
        	// we have "suspects" so print off all suspects for each sample with them

	        // print off suspects
	        for (int i = 0; i < low_barcode_samples.size(); i++) {

	        	string& sample_key = low_barcode_samples.at(i) ;

	        	while (barcode_by_sample_key.at(sample_key) == low_barcode) {

	        		// write the line
	        		gzcsout_ptr_by_sample_key.at(sample_key)->write_line(
	        			get_seqnum_from_bcsnrid_line(line_by_sample_key.at(sample_key)), 
	        			line_by_sample_key.at(sample_key)) ;

	        		// count the line
					line_counter_by_sample_key.at(sample_key)++ ;

	        		if (gzin_ptr_by_sample_key.at(sample_key)->has_next_line()) {

	        			// get a new line
	        			line_by_sample_key.at(sample_key) = gzin_ptr_by_sample_key.at(sample_key)->read_line() ;
						barcode_by_sample_key.at(sample_key) = get_barcode_from_bcsnrid_line(line_by_sample_key.at(sample_key)) ;

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

	// get chunk files, close gzouts, delete gzouts and gzins
	unordered_map<string,vector<string>> supect_chunk_paths_by_sample_key ;
	for (auto it = gzcsout_ptr_by_sample_key.begin(); it != gzcsout_ptr_by_sample_key.end(); ++it) {

		supect_chunk_paths_by_sample_key.insert(make_pair(it->first, it->second->get_files())) ;
		it->second->flush_close() ; delete it->second ;
	}
	for (auto it = gzin_ptr_by_sample_key.begin(); it != gzin_ptr_by_sample_key.end(); ++it) {

		delete it->second ;
	}

	// log number of suspect read ids found
	for (auto it = sample_ptr_by_sample_key.begin(); it != sample_ptr_by_sample_key.end(); ++it) {
		string msg = it->second->get_project_name() + " - " + it->second->get_sample_name() + " : " ;
		msg += to_string(line_counter_by_sample_key.at(it->first)) + " suspect barcode seq-num read ids written \n" ;
		cout << msg ;
	}

	return supect_chunk_paths_by_sample_key ;
}

int collect_suspect_bcsnrid_lines_task_func(Task<int, Collect_suspect_bcsnrid_lines_args> task) {

	// get argument references
	vector<string>& suspect_bcsnrid_chunk_paths = task.args.suspect_bcsnrid_chunk_paths ;
	string& suspect_bcsnrid_path = task.args.suspect_bcsnrid_path ;
	Sample* sample_ptr = task.args.sample_ptr ;

	// log activity
	string log_header = sample->get_project_name() + " - " + sample->get_sample_name() + " : " ;
	cout << log_header + "collecting suspect barcode seq-num read ids\n" ;

	int lines_written = collect_sorted_chunks<int>(suspect_bcsnrid_path, suspect_bcsnrid_chunk_paths, get_seqnum_from_bcsnrid_line) ;

	// log result
	string msg = log_header ;
	if (lines_written < 0) {

		msg += "suspect barcode seq-num read ids copied to" 
		msg += Path(suspect_bcsnrid_path).to_relative_path_string() + "\n" ;
	} else {

		msg += to_string(lines_written) 
		msg += " suspect barcode seq-num read ids written to" 
		msg += Path(suspect_bcsnrid_path).to_relative_path_string() + "\n" ;
	}	
	cout << msg ;

	return 0 ;
}
