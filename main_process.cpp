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
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "sample.h"
#include "workdir.h"
#include "whitelist.h"
#include "task_pool.h"
#include "read_id_barcodes.h"

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

	if (argc < 5) { show_usage(argv[0]) ; return 1 ; }

	// threads to run with
	int threads ;
	try {
		threads = std::stoi(argv[1]) ;
	} catch (const char* msg) {
		std::cerr << msg << std::endl ;
		show_usage(argv[0]) ;
		return 1;
	}

	// work dir path
	boost::filesystem::path work_dir_path = boost::filesystem::path(argv[2]) ;
	if (boost::filesystem::exists(work_dir_path)) {
		std::cerr << "work dir path, " << work_dir_path.generic_string() 
		<< ", already exists, will not overwrite" << std::endl ;
		show_usage(argv[0]) ;
		return 1;
	}
	if (!boost::filesystem::is_directory(work_dir_path.parent_path())) {
		std::cerr << "work dir path, " << work_dir_path.generic_string() 
		<< ", does not have a directory for a parent" << std::endl ;
		show_usage(argv[0]) ;
		return 1;
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
				return 1;
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
		return 1;
	}

	// print off samples
	std::cout << std::endl << "Samples:" << std::endl << std::endl ;
	for (auto it = samples.begin(); it != samples.end(); ++it) {
		std::cout << it->second.to_string() << std::endl << std::endl ;
	}

	// create the workdir
	boost::filesystem::create_directory(work_dir_path) ;
	Workdir workdir = Workdir(work_dir_path, samples) ;

	// create tasks to read read-id-barcodes for each sample
	std::stack<Task<int>> task_stack;
	for (std::string key : sample_keys) {

		for (std::string bc_fq_path : samples.at(key).get_barcode_fastq_paths()) {

			std::string rid_barcodes_path = workdir.get_read_id_barcodes_path(key, bc_fq_path) ;

			std::unordered_map<std::string, std::string> string_args ;

			string_args.insert({
				{"read_id_barcodes_path", rid_barcodes_path}, 
				{"fastq_path", bc_fq_path}
			}) ;

			// create the task
			Task<int> task ; 
			task.func = read_id_barcodes ; 
			task.string_args = string_args ;
			task.sample_ptr = &samples.at(key); 
			task.workdir_ptr = &workdir;

			task_stack.push(task) ;
		}
	}


	run_tasks(threads, task_stack) ;

	// while (true) std::cout << "done\n" ; std::this_thread::sleep_for(span) ;

	return 0;
}
