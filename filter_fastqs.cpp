#include "filter_fastqs.h"

using namespace std ;

void filter_fastqs(int threads, string& output_dir_path, 
	string& read_ids_to_exclude_path, unordered_set<string>& fastq_path_set) {

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
}

int filter_fastq_set_task_func(Task<int, Filter_fastq_set_args>) ;
