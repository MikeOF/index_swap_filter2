#include "swapped_reads.h"

using namespace std ;

void write_swapped_reads(int threads, unordered_map<string, Sample>& samples, Workdir& workdir) {

	// create a swap calling task for each CUG path
	stack<Task<int, Call_swapped_reads_args>> call_swaps_task_stack ;
	for (string star_ref_path : workdir.get_star_reference_paths()) {

		Task<int, Call_swapped_reads_args> task ;
		task.func = call_swapped_reads_task_func ;
		task.args.cug_label_path = workdir.get_cug_label_path(star_ref_path) ;
		task.args.called_swaps_path = workdir.get_called_swaps_path(star_ref_path) ;

		call_swaps_task_stack.push(task) ;
	}
	run_tasks(threads, call_swaps_task_stack) ;
}

int call_swapped_reads_task_func(Task<int, Call_swapped_reads_args> task) {

	// parse arg
	string cug_label_path = task.args.cug_label_path ;
	string called_swaps_path = task.args.called_swaps_path ;

	// get file reader and writer
	Gzin gzin (cug_label_path) ;
	Gzout gzout (called_swaps_path) ;

	string current_cug = "" ;
	unordered_map<string, unordered_set<string>> read_ids_by_sample_key ;
	int read_cnt = 0 ;
	while (true) {

		string line = gzin.read_line() ;

		// get barcode
		int barcode_stop = line.find('\t') ;
		string barcode = line.substr(0, barcode_stop) ;

		// get gene id
		int gene_id_start = barcode_stop + 1 ;
		int gene_id_stop = line.find('\t', gene_id_start) ;
		string gene_id = line.substr(gene_id_start, gene_id_stop - gene_id_start) ;

		// get sample key
		int sample_key_start = gene_id_stop + 1 ;
		int sample_key_stop = line.find('\t', sample_key_start) ;
		string sample_key = line.substr(sample_key_start, sample_key_stop - sample_key_start) ;

		// get read id
		string read_id = line.substr(sample_key_stop + 1) ;

		string this_cug = barcode + gene_id ;

		// add read if this is the current cug
		if (this_cug == current_cug) {

			// add sample key if necessary
			if (read_ids_by_sample_key.find(sample_key) == read_ids_by_sample_key.end()) {
				read_ids_by_sample_key.insert(make_pair(sample_key, unordered_set<string>())) ;
			}

			read_ids_by_sample_key.at(sample_key).insert(read_id) ;
			read_cnt++ ;
		
		} 

		// write swaps and reset the read ids by sample key map if we are done with the current cug
		bool has_next_line = gzin.has_next_line() ;
		if (this_cug != current_cug || !has_next_line) { // runs on the first pass

			float limit = read_cnt * .8 ;

			// write out reads if the sample is below the count limit
			for (auto it = read_ids_by_sample_key.begin(); it != read_ids_by_sample_key.end(); ++it) {

				if (it->second.size() < limit) {
					for (string swap_read_id : it->second) { gzout.write_line(it->first + swap_read_id) ; }
					cout << "size below limit " + to_string(it->second.size()) + " limit: " + to_string(limit) + "\n" ;
				} else {
					cout << "size above limit " + to_string(it->second.size()) + " limit: " + to_string(limit) + "\n" ;
				}
			}

			// exit the while loop if we are done with the cug file
			if (!has_next_line) break ;

			// reset map and tracking vars
			read_cnt = 1 ;
			read_ids_by_sample_key.clear() ;
			current_cug = this_cug ;
			read_ids_by_sample_key.insert(make_pair(sample_key, unordered_set<string>())) ;
			read_ids_by_sample_key.at(sample_key).insert(read_id) ;
		}
	}
	gzout.flush_close() ;

	return 0 ;
}

