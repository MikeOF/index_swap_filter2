#include "check_cell_overlap.h"

using namespace std ;

void check_cell_barcode_overlap(int threads, unordered_map<string, Sample>& samples, Workdir& workdir) {

	// run tasks to read in swapped in read ids
	unordered_map<string, unordered_set<string>> read_ids_to_exclude_by_sample_key ;
	stack<Task<int, Read_in_swapped_in_read_ids_args>> read_swap_rids_task_stack ;
	for (string sample_key : workdir.get_sample_keys()) {

		// make the read ids set
		read_ids_to_exclude_by_sample_key.insert(make_pair(sample_key, unordered_set<string>())) ;

		// make the task
		Task<int, Read_in_swapped_in_read_ids_args> task ;
		task.func = read_in_swapped_in_read_ids_task_func ;
		task.args.swapped_in_read_ids_path = workdir.get_swapped_in_read_ids_path(sample_key) ;
		task.args.read_ids_to_exclude_set_ptr = &(read_ids_to_exclude_by_sample_key.at(sample_key));
		task.args.sample_ptr = &(samples.at(sample_key));

		read_swap_rids_task_stack.push(task) ;
	}
	run_tasks(threads, read_swap_rids_task_stack) ;

	// run tasks to read in cell barcode maps
	unordered_map<string, set<string>> unfiltered_cellb_set_by_sample_key ;
	unordered_map<string, set<string>> filtered_cellb_set_by_sample_key ;

	stack<Task<int, Read_in_cell_barcode_counts_args>> read_cellb_task_stack ;
	for (string sample_key : workdir.get_sample_keys()) {

		// make the cell barcode sets
		unfiltered_cellb_set_by_sample_key.insert(make_pair(sample_key, set<string>())) ;
		filtered_cellb_set_by_sample_key.insert(make_pair(sample_key, set<string>())) ;

		// make the task
		Task<int, Read_in_cell_barcode_counts_args> task ;
		task.func = read_in_cell_barcode_counts_task_func ;
		task.args.bcsnrid_path =  workdir.get_bcsnrid_path(sample_key) ;
		task.args.read_ids_to_exclude_set_ptr = &(read_ids_to_exclude_by_sample_key.at(sample_key)) ;
		task.args.unfiltered_cell_barcode_set_ptr = &(unfiltered_cellb_set_by_sample_key.at(sample_key)) ; 
		task.args.filtered_cell_barcode_set_ptr = &(filtered_cellb_set_by_sample_key.at(sample_key)) ;
		task.args.sample_ptr = &(samples.at(sample_key));

		read_cellb_task_stack.push(task) ;
	}
	run_tasks(threads, read_cellb_task_stack) ;

	// now write cell overlap
	unordered_map<string, string> sample_name_by_sample_key ;
	vector<string> sample_keys_vect ;
	for (string sample_key : workdir.get_sample_keys()) {
		sample_name_by_sample_key.insert(make_pair(sample_key, samples.at(sample_key).get_sample_name())) ;
		sample_keys_vect.push_back(sample_key) ;
	}

	write_cell_overlap(workdir.get_unfiltered_overlap_path(), workdir.get_filtered_overlap_path(), 
		sample_keys_vect, sample_name_by_sample_key, 
		unfiltered_cellb_set_by_sample_key, filtered_cellb_set_by_sample_key) ;
}

int read_in_swapped_in_read_ids_task_func(Task<int, Read_in_swapped_in_read_ids_args> task) {

	// parse arguments 
	string& swapped_in_read_ids_path = task.args.swapped_in_read_ids_path ;
	unordered_set<string>& read_ids_to_exclude_set = *(task.args.read_ids_to_exclude_set_ptr) ;
	Sample& sample = *(task.args.sample_ptr) ;

	// logging
	stringstream ss ;
	string log_header = get_sample_log_header(sample) ;

	// log beginning 
	ss.str("") ;
	ss << log_header << "reading swapped in read ids from " ;
	ss << Path(swapped_in_read_ids_path).to_relative_path_string() << endl ;
	log_message(ss.str()) ;

	// read in the read ids to exclude
	Gzin gzin (swapped_in_read_ids_path) ;
	while (gzin.has_next_line()) {

		read_ids_to_exclude_set.insert(gzin.read_line()) ;
	}

	// log end
	ss.str("") ;
	ss << log_header << to_string(read_ids_to_exclude_set.size()) ;
	ss << " swapped in read ids read" << endl ;
	log_message(ss.str()) ;

	return 0 ;
}

int read_in_cell_barcode_counts_task_func(Task<int, Read_in_cell_barcode_counts_args> task) {

	// parse arguments
	string& bcsnrid_path = task.args.bcsnrid_path ;
	unordered_set<string>& read_ids_to_exclude_set = *(task.args.read_ids_to_exclude_set_ptr) ;
	set<string>& unfiltered_cell_barcode_set = *(task.args.unfiltered_cell_barcode_set_ptr) ;
	set<string>& filtered_cell_barcode_set = *(task.args.filtered_cell_barcode_set_ptr) ;
	Sample& sample = *(task.args.sample_ptr) ;

	// logging
	stringstream ss ;
	string log_header = get_sample_log_header(sample) ;

	// log beginning 
	ss.str("") ;
	ss << log_header << "reading cell barcode umi counts from " ;
	ss << Path(bcsnrid_path).to_relative_path_string() << endl ;
	log_message(ss.str()) ;

	// create count maps
	unordered_map<string, int> unfiltered_umi_count_by_cell_barcode ;
	unordered_map<string, int> filtered_umi_count_by_cell_barcode ;

	// fill maps
	Gzin gzin (bcsnrid_path) ;
	string current_cell_barcode = "" ;
	int cell_barcode_len = sample.get_cell_barcode_len() ;
	string current_barcode = "" ;
	int barcode_len = sample.get_umi_len() + cell_barcode_len ;
	while(gzin.has_next_line()) {

		string bcsnrid_line = gzin.read_line() ;

		// check barcode
		string barcode = bcsnrid_line.substr(0, barcode_len) ;

		// continue if this is another read for the current barcode
		if (barcode != current_barcode) {
			current_barcode = barcode ;
			continue ;
		}

		// parse line some more
		string cell_barcode = bcsnrid_line.substr(0, cell_barcode_len) ;
		string read_id = bcsnrid_line.substr(bcsnrid_line.rfind('\t') + 1) ;

		// count in unfiltered map
		if (unfiltered_umi_count_by_cell_barcode.count(cell_barcode) == 0) {
			unfiltered_umi_count_by_cell_barcode.insert(make_pair(cell_barcode, 0)) ;
		}
		unfiltered_umi_count_by_cell_barcode.at(cell_barcode)++ ;

		// count in filtered map if necessary
		if (read_ids_to_exclude_set.count(read_id) == 0) {

			if (filtered_umi_count_by_cell_barcode.count(cell_barcode) == 0) {
				filtered_umi_count_by_cell_barcode.insert(make_pair(cell_barcode, 0)) ;
			}
			filtered_umi_count_by_cell_barcode.at(cell_barcode)++ ;
		}
	}

	// create cell barcode sets
	auto it = unfiltered_umi_count_by_cell_barcode.begin() ;
	while (it != unfiltered_umi_count_by_cell_barcode.end()) {

		if (it->second > CELL_UMI_COUNT_THRESHOLD) {
			unfiltered_cell_barcode_set.insert(it->first) ;
		}
		it = unfiltered_umi_count_by_cell_barcode.erase(it) ;
	}

	it = filtered_umi_count_by_cell_barcode.begin() ;
	while (it != filtered_umi_count_by_cell_barcode.end()) {

		if (it->second > CELL_UMI_COUNT_THRESHOLD) {
			filtered_cell_barcode_set.insert(it->first) ;
		}
		it = filtered_umi_count_by_cell_barcode.erase(it) ;
	}


	// log ending
	ss.str("") ;
	ss << log_header << to_string(unfiltered_cell_barcode_set.size()) ;
	ss << " unfiltered cell barcodes taken " ;
	ss << to_string(filtered_cell_barcode_set.size()) ;
	ss << " filtered cell barcodes taken" << endl ;
	log_message(ss.str()) ;

	return 0 ;
}

void write_cell_overlap(string unfiltered_overlap_path, string filtered_overlap_path, 
	vector<string>& sample_keys, unordered_map<string, string>& sample_name_by_sample_key,
	unordered_map<string, set<string>>& unfiltered_cellb_set_by_sample_key, 
	unordered_map<string, set<string>>& filtered_cellb_set_by_sample_key) {

	// logging
	stringstream ss ;

	// log beginning
	ss << GLOBAL_LOG_HEADER << "comparing cell overlap between samples" << endl ;
	log_message(ss.str()) ;

	unordered_map<string, unordered_map<string, string>> unfiltered_sample_key_cell_overlap_by_sample_key ;
	unordered_map<string, unordered_map<string, string>> filtered_sample_key_cell_overlap_by_sample_key ;
	for (string sample_key : sample_keys) {

		// add to overlap maps
		unfiltered_sample_key_cell_overlap_by_sample_key.insert(
			make_pair(sample_key, unordered_map<string, string>())) ;
		filtered_sample_key_cell_overlap_by_sample_key.insert(
			make_pair(sample_key, unordered_map<string, string>())) ;

		// get sets
		set<string>& unfiltered_set = unfiltered_cellb_set_by_sample_key.at(sample_key) ;
		set<string>& filtered_set = filtered_cellb_set_by_sample_key.at(sample_key) ;	

		for (string o_sample_key : sample_keys) {

			// get other sets
			set<string>& o_unfiltered_set = unfiltered_cellb_set_by_sample_key.at(o_sample_key) ;
			set<string>& o_filtered_set = filtered_cellb_set_by_sample_key.at(o_sample_key) ;	

			// get the intersections
			set<string> unfiltered_intersection ;

			set_intersection(unfiltered_set.begin(), unfiltered_set.end(), 
				o_unfiltered_set.begin(), o_unfiltered_set.end(), 
				inserter (unfiltered_intersection, unfiltered_intersection.begin())) ;

			set<string> filtered_intersection ;

			set_intersection(filtered_set.begin(), filtered_set.end(), 
				o_filtered_set.begin(), o_filtered_set.end(), 
				inserter (filtered_intersection, filtered_intersection.begin())) ;

			// compute overlap
			float unfiltered_overlap = 0.0 ;
			if (unfiltered_set.size() != 0) {
				unfiltered_overlap = 100 * unfiltered_intersection.size() / unfiltered_set.size() ;
			} 

			float filtered_overlap = 0.0 ;
			if (filtered_set.size() != 0) {
				filtered_overlap = 100 * filtered_intersection.size() / filtered_set.size() ;
			}

			// format overlap
			stringstream unf_oss ;
			stringstream f_oss ;
			unf_oss << fixed << setprecision(2) << unfiltered_overlap ;
			f_oss << fixed << setprecision(2) << filtered_overlap ;

			// add to overlap maps
			unfiltered_sample_key_cell_overlap_by_sample_key.at(sample_key).insert(
				make_pair(o_sample_key, unf_oss.str() + "%")) ;
			filtered_sample_key_cell_overlap_by_sample_key.at(sample_key).insert(
				make_pair(o_sample_key, f_oss.str() + "%")) ;
		}
	}

	// write out tables
	ofstream unf_ofile (unfiltered_overlap_path) ;
	ofstream f_ofile (filtered_overlap_path) ;

	// get header line
	stringstream header_ss ; 
	for (string sample_key : sample_keys) { 
		header_ss << "\t" << sample_name_by_sample_key.at(sample_key) ;
	}
	header_ss << endl ;
	string header_line = header_ss.str() ;

	// write header
	unf_ofile.write(header_line.c_str(), header_line.size()) ;
	f_ofile.write(header_line.c_str(), header_line.size()) ;

	for (string sample_key : sample_keys) {

		stringstream unf_line_ss, f_line_ss ;
		unf_line_ss << sample_name_by_sample_key.at(sample_key) ;
		f_line_ss << sample_name_by_sample_key.at(sample_key) ;

		for (string o_sample_key : sample_keys) {

			// compose line
			unf_line_ss << "\t" ;
			f_line_ss << "\t" ;

			unf_line_ss << unfiltered_sample_key_cell_overlap_by_sample_key.at(sample_key).at(o_sample_key) ;
			f_line_ss << filtered_sample_key_cell_overlap_by_sample_key.at(sample_key).at(o_sample_key) ;
		}
		unf_line_ss << endl ;
		f_line_ss << endl ;

		// write lines
		string unf_line = unf_line_ss.str() ;
		string f_line = f_line_ss.str() ;

		unf_ofile.write(unf_line.c_str(), unf_line.size()) ;
		f_ofile.write(f_line.c_str(), f_line.size()) ;
	}
}
