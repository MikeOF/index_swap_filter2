#include "filter_fastqs.h"

using namespace std ;

void filter_fastqs(int threads, string& output_dir_path, 
	string& read_ids_to_exclude_path, unordered_set<string>& fastq_path_set) {

	// log spacing
	stringstream ss << endl << "Filtration Beginning" << endl ;
	log_message(ss.str()) ;

	// get groups of corresponding fastqs
	unordered_map<string, unordered_set<string>> fastq_path_by_first_read_id ;
	for (string fastq_path : fastq_path_set) {

		Gzin gzin (fastq_path) ;

		if (!gzin.has_next_line()) throw runtime_error("passed an empty fastq file, " + fastq_path) ;

		string line = gzin.read_line() ;
		string read_id = line.substr(0, line.find_first_of(" \t")) ;

		if (fastq_path_by_first_read_id.count(read_id) == 0) {
			fastq_path_by_first_read_id.insert(make_pair(read_id, unordered_set<string>())) ;
		}

		fastq_path_by_first_read_id.at(read_id).insert(fastq_path) ;
	}

	// read in all read ids to exclude
	unordered_set<string> read_ids_to_exclude_set ;
	Gzin gzin (read_ids_to_exclude_path) ;
	while(gzin.has_next_line()) {

		string read_id = gzin.read_line() ;

		// trim any whitespace
		int first = read_id.find_first_not_of(" \t") ;
		if (first == string::npos) throw runtime_error("a blank read id was found the the to-exclude set") ;

		int last = read_id.find_last_not_of(" \t")

		read_id = read_id.substr(first, last - first + 1) ;

		read_ids_to_exclude_set.insert(read_id) ;
	}

	// create and run filtration tasks
	stack<Task<int, Filter_fastq_set_args>> filter_task_stack ;
	for (auto it = fastq_path_by_first_read_id.begin(); it != fastq_path_by_first_read_id.end(); ++it) {

		Task<int, Filter_fastq_set_args> task ;
		task.func = filter_fastq_set_task_func ;
		task.args.fastq_path_set = it->second ;
		task.args.read_ids_to_exclude_set_ptr = &read_ids_to_exclude_set ;
		task.args.output_dir_path = output_dir_path ;

		filter_task_stack.push(task) ;
	}
	run_tasks(threads, filter_task_stack) ;

	// log ending
	ss.str("") ;
	ss << endl << "Filtration Complete" << endl ;
	log_message(ss.str()) ;
}

int filter_fastq_set_task_func(Task<int, Filter_fastq_set_args> task) {

	// parse args
	unordered_set<string>& fastq_path_set = task.args.fastq_path_set ;
	unordered_set<string>& read_ids_to_exclude_set = *(task.args.read_ids_to_exclude_set_ptr) ;
	Path output_dir_path (task.args.output_dir_path) ;

	// log group beginning
	stringstream ss << endl << "Fastq Group:" << endl ;
	for (string& fastq_path : fastq_path_set) {
		ss << "\t" << fastq_path << endl ;
	} 
	ss << "\t" << "beginning" << end ;
	log_message(ss.str()) ;

	// make the output directory
	output_dir_path.make_dir() ;

	// get maps of gzins and gzouts
	string anchor_fastq_path ;
	unordered_set<string> other_fastq_path_set ;
	unordered_map<string, Gzin *> gzin_ptr_by_fastq_path ;
	unordered_map<string, Gzout *> gzout_ptr_by_fastq_path ;
	int write_cnt = 0 ;
	int filtered_cnt = 0 ;
	for (string fastq_path : fastq_path_set) {

		// store fastq path conveniently
		if (anchor_fastq_path.empty()) {
			anchor_fastq_path = fastq_path ;
		} else {
			other_fastq_path_set.insert(fastq_path) ;
		}

		// make gzin
		gzin_ptr_by_fastq_path.insert(make_pair(fastq_path, new Gzin(fastq_path))) ;

		// make gzout
		Path fasq_out_Path = output_dir_path.join(Path(fastq_path).get_filename())
		gzout_ptr_by_fastq_path.insert(make_pair(fastq_path, new Gzout(fasq_out_Path.to_string()))) ;
	}

	// print out filtered fastqs
	while(gzin_ptr_by_fastq_path.at(anchor_fastq_path)->has_next_line()) {

		// read in the anchor fastq's seq
		string anchor_line = gzin_ptr_by_fastq_path.at(anchor_fastq_path)->read_line() ;

		// get and check read id
		string read_id = anchor_line.substr(0, anchor_line.find_first_of(" \t")) ;

		if (read_ids_to_exclude_set.count(read_id) == 0) {
			// then print out the sequence for each fastq
			write_cnt++ ;

			// print first line of anchor
			gzout_ptr_by_fastq_path.at(anchor_fastq_path)->write_line(anchor_line) ;

			// print rest of anchor for sequence
			for (int i = 0; i < 3; i++) {
				gzout_ptr_by_fastq_path.at(anchor_fastq_path)->write_line(
					gzin_ptr_by_fastq_path.at(anchor_fastq_path)->read_line()) ;
			}

			// print a seq from each of the other fastqs
			for (string other_fastq_path : other_fastq_path_set) {
				for (int i = 0; i < 4; i++) {

					gzout_ptr_by_fastq_path.at(other_fastq_path)->write_line(
						gzin_ptr_by_fastq_path.at(other_fastq_path)->read_line()) ;
				}
			}

		} else {
			// read past this sequence for each fastq
			filtered_cnt++ ;

			for (int i = 0; i < 3; i++) {
				gzin_ptr_by_fastq_path.at(anchor_fastq_path)->read_line() ;
			}

			for (string other_fastq_path : other_fastq_path_set) {
				for (int i = 0; i < 4; i++) {

					gzin_ptr_by_fastq_path.at(other_fastq_path)->read_line() ;
				}
			}
		}
	}

	// log ending
	ss.str("") ;
	ss << endl << "Fastq Group:" << endl ;
	for (string& fastq_path : fastq_path_set) {
		ss << "\t" << fastq_path << endl ;
	}
	ss << "\t" << to_string(write_cnt) << " sequences written" << end ;
	ss << "\t" << to_string(filtered_cnt) << " sequences filtered" << end ;
	log_message(ss.str()) ;
}
