#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <exception>
#include <array>

#include "files.h"
#include "sample.h"
#include "workdir.h"
#include "whitelist.h"
#include "task_pool.h"
#include "read_id_barcodes.h"
//#include "suspect_barcodes.h"

void show_usage(std::string name) {
    std::cerr << "Usage: " 
    << name 
    << " threads work_dir_path sample_def sample_def [sample_def ...]" << std::endl
    << "\tsample_def: <project_name>,<sample_name>,<whitelist_path>,<fastq_dir_path>,<star_reference_path>,<barcode_key>,<barcode_fq_pattern>,<sequence_fq_pattern>" << std::endl
    << "\t\tbarcode_key: [N ...]C[C ...][N ...]U[U ...][N ...] or [N ...]U[U ...][N ...]C[C ...][N ...]"
    << std::endl ;
}

int main(int argc, char ** argv) {

	// parse and validate arguments

	if (argc < 5) { show_usage(argv[0]) ; exit(1) ; }

	// threads to run with 
	int threads ;
	try {
		threads = std::stoi(argv[1]) ;
	} catch (const char* msg) {
		std::cerr << msg << std::endl ;
		show_usage(argv[0]) ;
		exit(1) ; 
	}

	// work dir path
	std::string work_dir_path (argv[2]) ;
	if (file_exists(work_dir_path)) {
		std::cerr << "work dir path, " << work_dir_path
		<< ", already exists, will not overwrite" << std::endl ;
		show_usage(argv[0]) ;
		exit(1) ; 
	}
	if (!is_dir(get_parent_path(work_dir_path))) {
		std::cerr << "work dir path, " << work_dir_path
		<< ", does not have a directory for a parent" << std::endl ;
		show_usage(argv[0]) ;
		exit(1) ; 
	}

	// each sample
	std::unordered_map<std::string, Sample> samples ;
	std::vector<std::string> sample_keys ;
	try {
		for (int i = 3; i < argc; i++) { 

			// create the sample
			Sample sample = Sample(argv[i]) ;

			// make sure there are no duplicate samples
			if (samples.count(sample.get_key()) > 0) {
				std::cerr << "Duplicate Sample Defs for "<< sample.get_project_name() 
				<< " " << sample.get_sample_name() << std::endl ;
				show_usage(argv[0]) ;
				exit(1) ; 
			}

			std::string key = sample.get_key() ;

			// add it to the map
			samples.insert({{key,sample}}) ;

			// keep key
			sample_keys.push_back(key) ;
		}

	} catch (std::exception& e) {

		std::cerr << e.what() << std::endl ;
		show_usage(argv[0]) ;
		exit(1) ; 
	}

	// print off samples
	std::cout << std::endl << "Samples:" << std::endl << std::endl ;
	for (auto it = samples.begin(); it != samples.end(); ++it) {
		std::cout << it->second.to_string() << std::endl << std::endl ;
	}

	// create the workdir 
	make_dir(work_dir_path) ;
	Workdir workdir = Workdir(work_dir_path, samples) ;

	// create tasks to read read-id-barcodes for each sample
	std::stack<Task<int, Read_id_barcodes_args>> task_stack;
	for (std::string key : sample_keys) {

		for (std::string bc_fq_path : samples.at(key).get_barcode_fastq_paths()) {

			// create the arguments
			Read_id_barcodes_args args ;
			args.read_id_barcodes_path = workdir.get_read_id_barcodes_path(key, bc_fq_path) ;
			args.fastq_path = bc_fq_path ;
			args.sample_ptr = &samples.at(key) ;

			// create the task
			Task<int, Read_id_barcodes_args> task ; 
			task.func = read_id_barcodes ; 
			task.args = args ;

			task_stack.emplace(task) ;
		}
	}

	std::cout << "hello before run_tasks" << std::endl ;

	run_tasks(threads, task_stack) ;

	std::cout << "hello after run_tasks" << std::endl ;

	//write_out_suspect_barcodes(sample_keys, samples, workdir) ;

	while(true) {std::cout << "hi" << std::endl; std::this_thread::sleep_for (std::chrono::seconds (5)) ;}

	// while (true) std::cout << "done\n" ; std::this_thread::sleep_for(span) ;

	return 0;
}
