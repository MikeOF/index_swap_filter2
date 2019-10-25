#include "write_suspect_fastq.h"

using namespace std ;

void write_suspect_read_fastqs(int threads, unordered_map<string, Sample>& samples, Workdir& workdir) {

	// write out suspect read fastqs for each sample

	// create task stack 
	stack<Task<int, Write_suspect_read_fastqs_args>> task_stack ;
	for (string sample_key : workdir.get_sample_keys()) {

		// create task
		Task<int, Write_suspect_read_fastqs_args> task ;
		task.func = write_suspect_read_fastqs_task_func ;
		task.args.suspect_bcsnrid_path = workdir.get_suspect_bcsnrid_path(sample_key) ;
		task.args.suspect_read_fastq_path = workdir.get_suspect_read_fastq_path(sample_key) ;
		task.args.sample_ptr = &samples.at(sample_key) ;

		task_stack.push(task) ;
	}
	run_tasks(threads, task_stack) ;

	// write out the global suspect read fastq

	// create the task
	Task<int, Collect_suspect_read_fastqs_args> collect_task ;
	collect_task.func = collect_suspect_read_fastqs_task_func ;
	collect_task.args.global_suspect_read_fastq_path = workdir.get_suspect_read_fastq_path() ;

	for (string sample_key : workdir.get_sample_keys()) { 

		collect_task.args.suspect_read_fastq_paths.push_back(
			workdir.get_suspect_read_fastq_path(sample_key)) ;
	}

	// add to a stack
	stack<Task<int, Collect_suspect_read_fastqs_args>> collect_task_stack ;
	collect_task_stack.push(collect_task) ;

	// run tasks
	run_tasks(1, collect_task_stack) ;
}

int write_suspect_read_fastqs_task_func(Task<int, Write_suspect_read_fastqs_args> task) {

	// logging stream
	stringstream ss ;

	// parse
	string& suspect_bcsnrid_path = task.args.suspect_bcsnrid_path ;
	string& suspect_read_fastq_path = task.args.suspect_read_fastq_path ;
	Sample& sample = *(task.args.sample_ptr) ;
	string sample_key = sample.get_key() ;

	// log strings
    string log_header = get_sample_log_header(sample) ;
    string suspect_read_fastq_relative_path_string = Path(suspect_read_fastq_path).to_relative_path_string() ;

    // log beginning
    ss.str("") ;
    ss << log_header << "writing suspect read fastqs to " ;
    ss << suspect_read_fastq_relative_path_string << endl ;
    log_message(ss.str()) ;

	// get fastq data structures
	unordered_set<string> fastq_name_set ;
	unordered_map<string, Gzin*> gzin_ptr_by_fastq_name ;
	unordered_map<string, int> seq_cnt_by_fastq_name ;
	unordered_map<string, vector<string>> seq_lines_by_fastq_name ;

	// for each read fastq create entry in each data structure
	for (string fastq_path : sample.get_sequence_fastq_paths()) {

		string fastq_name = Path(fastq_path).get_filename() ;
		fastq_name_set.insert(fastq_name) ;

		Gzin* gzin_ptr = new Gzin(fastq_path) ;
		gzin_ptr_by_fastq_name.insert(make_pair(fastq_name, gzin_ptr)) ;
		seq_cnt_by_fastq_name.insert(make_pair(fastq_name, 1)) ;
		seq_lines_by_fastq_name.insert(make_pair(fastq_name, vector<string>())) ;

		if (!gzin_ptr->has_next_line()) throw runtime_error("found empty sequence fastq path") ;

		for (int i = 0; i < 4; i++) {
			seq_lines_by_fastq_name.at(fastq_name).push_back(gzin_ptr->read_line()) ; 
		}
	}

	// read in each suspect line, write out suspect fastqs
	Gzin suspect_bcsnrid_gzin (suspect_bcsnrid_path) ;
	Gzout suspect_fastq_gzout (suspect_read_fastq_path) ;

	int write_cnt = 0 ;
	while(suspect_bcsnrid_gzin.has_next_line()) {

		string bcsnrid_line = suspect_bcsnrid_gzin.read_line() ;

		int suspect_line_number = get_seqnum_from_bcsnrid_line(bcsnrid_line) ;
		string read_id = get_read_id_from_bcsnrid_line(bcsnrid_line) ;
		string barcode = get_barcode_from_bcsnrid_line(bcsnrid_line) ;

		// read up to suspect line number
		stack<string> to_evict ;
		for (string fastq_name : fastq_name_set) {
			while (seq_cnt_by_fastq_name.at(fastq_name) < suspect_line_number) {

				// first check to see if this seq fastq has more lines to read
				if (!gzin_ptr_by_fastq_name.at(fastq_name)->has_next_line()) {
					to_evict.push(fastq_name) ; 
					break ;
				}

				vector<string>& seq_lines = seq_lines_by_fastq_name.at(fastq_name) ;
				for (int i = 0; i < 4; i++) {
					seq_lines.at(i) = gzin_ptr_by_fastq_name.at(fastq_name)->read_line() ; 
				}
				seq_cnt_by_fastq_name.at(fastq_name)++ ;
			}
		}

        // evict finished fastqs
        while (!to_evict.empty()) { fastq_name_set.erase(to_evict.top()) ; to_evict.pop() ; }

        // write out sequence fastqs that match the read id
        for (string fastq_name : fastq_name_set) {

        	// get read id of fastq
        	string& read_id_line = seq_lines_by_fastq_name.at(fastq_name).at(0) ;
        	string fastq_read_id = read_id_line.substr(1, read_id_line.find_first_of(" \t") - 1) ;

        	if (read_id == fastq_read_id) {

        		// update read id
        		stringstream ss ;
        		ss << "@" << read_id << SUSPECT_FASTQ_READ_ID_DELIM << sample_key ;
        		ss << SUSPECT_FASTQ_READ_ID_DELIM << barcode ;
         		seq_lines_by_fastq_name.at(fastq_name).at(0) = ss.str() ;

        		// write out each suspect line
        		for (string line : seq_lines_by_fastq_name.at(fastq_name)) {
        			suspect_fastq_gzout.write_line(line) ;
        		}
        		write_cnt++ ;
        		break ;
        	}
		}
	}

	// close out
	suspect_fastq_gzout.flush_close() ;
	for (auto it = gzin_ptr_by_fastq_name.begin(); it != gzin_ptr_by_fastq_name.end(); ++it) {
		delete it->second ;
	}

	// remove suspect bcsnrid path
	Path(suspect_bcsnrid_path).remove_file() ;

	// log ending
	ss.str("") ;
	ss << log_header << to_string(write_cnt) << " suspect reads written to " ;
    ss << suspect_read_fastq_relative_path_string << endl ;
    log_message(ss.str()) ;

	return 0 ;
}

int collect_suspect_read_fastqs_task_func(Task<int, Collect_suspect_read_fastqs_args> task) {

	// logging stream
	stringstream ss ;

	// parse
	vector<string>& suspect_read_fastq_paths = task.args.suspect_read_fastq_paths ; 
	string& global_suspect_read_fastq_path = task.args.global_suspect_read_fastq_path ;

	// log activity
	ss.str("") ; 
	ss << GLOBAL_LOG_HEADER << "collecting suspect read fastqs into " ;
	ss << Path(global_suspect_read_fastq_path).to_relative_path_string() << endl ;
	log_message(ss.str()) ;

	combine_gz_files(global_suspect_read_fastq_path, suspect_read_fastq_paths) ;

	// remove sample specific suspect read fastqs
	for (string sample_suspect_read_fastq_path : suspect_read_fastq_paths) {
		Path(sample_suspect_read_fastq_path).remove_file() ;
	}

	return 0 ;
}
