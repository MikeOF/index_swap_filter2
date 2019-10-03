#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <exception>

#include "path.h"
#include "sample.h"
#include "workdir.h"
#include "whitelist.h"
#include "task_pool.h"
#include "barcode_read_ids.h"
#include "suspect_barcodes.h"

using namespace std ;

void show_usage(string name) {
    cerr << "Usage: " 
    << name 
    << " threads work_dir_path sample_def sample_def [sample_def ...]" << endl
    << "\tsample_def: <project_name>,<sample_name>,<whitelist_path>,<fastq_dir_path>,<star_reference_path>,<barcode_key>,<barcode_fq_pattern>,<sequence_fq_pattern>" << endl
    << "\t\tbarcode_key: [N ...]C[C ...][N ...]U[U ...][N ...] or [N ...]U[U ...][N ...]C[C ...][N ...]"
    << endl ;
}

int main(int argc, char ** argv) {

	// parse and validate arguments
	if (argc < 5) { show_usage(argv[0]) ; exit(1) ; }

	// threads to run with 
	int threads ;
	try {
		threads = stoi(argv[1]) ;
	} catch (const char* msg) {
		cerr << msg << endl ;
		show_usage(argv[0]) ;
		exit(1) ; 
	}

	// work dir path
	Path work_dir_path = Path(argv[2]) ;
	if (work_dir_path.exists()) {
		cerr << "work dir path, " << work_dir_path.to_string()
		<< ", already exists, will not overwrite" << endl ;
		show_usage(argv[0]) ;
		exit(1) ; 
	}
	if (!work_dir_path.is_dir()) {
		cerr << "work dir path, " << work_dir_path.to_string()
		<< ", does not have a directory for a parent" << endl ;
		show_usage(argv[0]) ;
		exit(1) ; 
	}

	// each sample
	unordered_map<string, Sample> samples ;
	vector<string> sample_keys ;
	try {
		for (int i = 3; i < argc; i++) { 

			// create the sample
			Sample sample (argv[i]) ;

			// make sure there are no duplicate samples
			if (samples.count(sample.get_key()) > 0) {
				cerr << "Duplicate Sample Defs for "<< sample.get_project_name() 
				<< " " << sample.get_sample_name() << endl ;
				show_usage(argv[0]) ;
				exit(1) ; 
			}

			string key = sample.get_key() ;

			// add it to the map
			samples.insert({{key,sample}}) ;

			// keep key
			sample_keys.push_back(key) ;
		}

	} catch (exception& e) {

		cerr << e.what() << endl ;
		show_usage(argv[0]) ;
		exit(1) ; 
	}

	// print off samples
	cout << endl << "Samples\n-------" << endl << endl ;
	for (auto it = samples.begin(); it != samples.end(); ++it) {
		cout << it->second.to_string() << endl << endl ;
	}

	// create the workdir 
	work_dir_path.make_dir() ;
	Workdir workdir = Workdir(work_dir_path, samples) ;

	// create tasks to read read-id-barcodes for each sample
	stack<Task<tuple<string, vector<string>>, Extract_barcode_read_ids_args>> task_stack_1 ;
	for (string key : sample_keys) {

		for (string bc_fq_path : samples.at(key).get_barcode_fastq_paths()) {

			// create the task
			Task<tuple<string, vector<string>>, Extract_barcode_read_ids_args> task ; 
			task.func = extract_barcode_read_ids ; 
			task.args.barcode_read_id_chunks_path = workdir.get_barcode_read_id_chunks_path(key, bc_fq_path) ;
			task.args.fastq_path = bc_fq_path ;
			task.args.sample_ptr = &samples.at(key) ;

			task_stack_1.push(task) ;
		}
	}

	// Barcode Read ID
	// collect barcode read id chunks for each sample
	stack<tuple<string, vector<string>>> barcode_read_id_chunk_stack = run_tasks(threads, task_stack_1) ;
	unordered_map<string, vector<string>> chunk_files_by_sample_key ;
	while (!barcode_read_id_chunk_stack.empty()) {

		tuple<string, vector<string>> barcode_read_id_chunk = barcode_read_id_chunk_stack.top() ; 
		barcode_read_id_chunk_stack.pop() ;

		string sample_key = get<0>(barcode_read_id_chunk) ;
		vector<string> chunk_files = get<1>(barcode_read_id_chunk) ;

		if (chunk_files_by_sample_key.find(sample_key) == chunk_files_by_sample_key.end()) {

			chunk_files_by_sample_key.insert(pair<string, vector<string>>(sample_key, chunk_files)) ;
		} else {
			vector<string>&  file_vect = chunk_files_by_sample_key.at(sample_key) ;
			file_vect.insert(file_vect.end(), chunk_files.begin(), chunk_files.end()) ;
		}
	}

	// create the task stack
	stack<Task<int, Collect_barcode_read_ids_args>> task_stack_2;
	for (unordered_map<string, vector<string>>::iterator it = chunk_files_by_sample_key.begin(); 
		it != chunk_files_by_sample_key.end(); ++it) {

		// create the collection task
		Task<int, Collect_barcode_read_ids_args> task ;
		task.func = collect_barcode_read_ids ;
		task.args.barcode_read_id_chunk_paths = it->second;
		task.args.read_ids_path = workdir.get_barcode_read_ids_path(it->first) ;
		task.args.sample_ptr = &samples.at(it->first) ;

		task_stack_2.push(task) ;
	}

	run_tasks(threads, task_stack_2) ;

	// Get Suspect Read IDs
	// create suspect task
	Task<int, Write_out_suspect_barcodes_args> get_suspect_read_ids_task ;
	get_suspect_read_ids_task.func = write_out_suspect_barcodes ;
	get_suspect_read_ids_task.args.key_getter = get_bc_key ;

	for (string sample_key : sample_keys) {

		string suspect_barcodes_path = workdir.get_suspect_read_ids_path(sample_key) ;

		string sorted_barcode_read_ids_path = workdir.get_barcode_read_ids_path(sample_key) ;

		Sample& sample = samples.at(sample_key) ;
		tuple<string, string> sample_name_project_name = tuple<string, string>(sample.get_sample_name(), sample.get_project_name()) ;

		get_suspect_read_ids_task.args.suspect_barcodes_path_by_sample_key.insert(pair<string, string>(sample_key, suspect_barcodes_path)) ;
		get_suspect_read_ids_task.args.sorted_barcode_read_ids_path_by_sample_key.insert(pair<string, string>(sample_key, sorted_barcode_read_ids_path)) ; 
		get_suspect_read_ids_task.args.sample_name_project_name_by_sample_key.insert(pair<string, tuple<string, string>> (sample_key, sample_name_project_name))  ;
	}
	stack<Task<int, Write_out_suspect_barcodes_args>> task_stack_3 ; task_stack_3.push(get_suspect_read_ids_task) ;	

	run_tasks(threads, task_stack_3) ;

	return 0;
}
