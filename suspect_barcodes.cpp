#include "suspect_barcodes.h"

void write_out_suspect_barcodes(std::vector<std::string> sample_keys, 
	std::unordered_map<std::string, Sample> samples, Workdir workdir, int threads) {

	// create the suspect barcodes map
	std::unordered_map<std::string, std::unordered_set<std::string>> suspect_barcodes_by_sample_key ;
	for (std::pair<std::string, Sample> element : samples) {

		std::string key = element.first;
		std::unordered_set<std::string> this_set ;

		suspect_barcodes_by_sample_key.insert({{key, this_set}}) ;
	}

	// create tasks to read in barcodes for each sample
	std::stack<Task<int, Get_all_barcodes_args>> task_stack;
	for (std::string key : sample_keys) {

		// create the arguments
		Get_all_barcodes_args args ;
		args.suspect_barcodes_ptr = &suspect_barcodes_by_sample_key.at(key) ;
		args.sample_ptr = &samples.at(key) ; 
		args.workdir_ptr = &workdir ;

		// create the task
		Task<int, Get_all_barcodes_args> task ; 
		task.func = get_all_barcodes ; 
		task.args = args ;

		task_stack.push(task) ;
	}

	run_tasks(threads, task_stack) ;

	for (std::pair<std::string, std::unordered_set<std::string>> element : suspect_barcodes_by_sample_key) {

		std::string key = element.first;

		std::cout << "sample key: " + key + ", " + std::to_string(element.second.size()) + " barcodes" << std::endl ;
	}
}

int get_all_barcodes(Task<int, Get_all_barcodes_args> task) {

	// parse task
	std::unordered_set<std::string> * suspect_barcodes_ptr = task.args.suspect_barcodes_ptr ;
	Sample sample = *task.args.sample_ptr ;
	Workdir workdir = *task.args.workdir_ptr ;

	// log the activity
	std::string msg = sample.get_project_name() + " - " + sample.get_sample_name() + " : reading in all barcodes\n" ;
	std::cout << msg ;

	int cnt = 0;
	for (std::string bc_fq_path : sample.get_barcode_fastq_paths()) {

		std::string rid_barcodes_path = workdir.get_read_id_barcodes_path(sample.get_key(), bc_fq_path) ;

		// create the input reader
		Gzins gz_input ;
		gz_input.file_path = rid_barcodes_path ;
		gz_begin_gzins(&gz_input) ;

		// get all the barcodes from this file
        while (!gz_input.finished) {

			gz_read_lines(&gz_input) ;

			for (std::string line : gz_input.line_vect) {

				// grab the cell-umi barcode
        		suspect_barcodes_ptr->insert(line.substr(0, line.find('\t') - 1)) ;
			}

			if (gz_input.finished) break ;
		}
	}

	return 0 ;
}
