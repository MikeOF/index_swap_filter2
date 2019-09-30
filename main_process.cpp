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
	cout << endl << "Samples:" << endl << endl ;
	for (auto it = samples.begin(); it != samples.end(); ++it) {
		cout << it->second.to_string() << endl << endl ;
	}

	// create the workdir 
	work_dir_path.make_dir() ;
	Workdir workdir = Workdir(work_dir_path, samples) ;

	// create tasks to read read-id-barcodes for each sample
	stack<Task<int, Extract_barcode_read_ids_args>> task_stack;
	for (string key : sample_keys) {

		for (string bc_fq_path : samples.at(key).get_barcode_fastq_paths()) {

			// create the arguments
			Extract_barcode_read_ids_args args ;
			args.barcode_read_ids_path = workdir.get_barcode_read_ids_path(key, bc_fq_path) ;
			args.fastq_path = bc_fq_path ;
			args.sample_ptr = &samples.at(key) ;

			// create the task
			Task<int, Extract_barcode_read_ids_args> task ; 
			task.func = extract_barcode_read_ids ; 
			task.args = args ;

			task_stack.emplace(task) ;
		}
	}

	run_tasks(threads, task_stack) ;

	return 0;
}
