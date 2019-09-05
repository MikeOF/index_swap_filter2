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

	if (argc < 5) { show_usage(argv[0]) ; }

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

	} catch (const std::string msg) {

		std::cerr << msg << std::endl ;
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

	// spin off threads to read read-id-barcodes for each sample
	std::stack<std::string> sample_key_stack;
	for (int i = 0; i < sample_keys.size(); i++) sample_key_stack.push(sample_keys[i]) ;

	std::vector<std::future<void>> children; 
	std::chrono::milliseconds span (500);

	bool running = true ;
	while (running) {

		// check children
		std::stack<int> to_remove ;
		for (int i = 0; i < children.size(); i++) {

			if (children[i].wait_for(span) == std::future_status::ready) {

				to_remove.push(i) ; children[i].get() ;
			} 
		}

		// remove completed children
		while (!to_remove.empty()) { 

			children.erase(children.begin() + to_remove.top()) ; to_remove.pop() ; 
		}

		// start threads
		int children_to_start = threads - children.size() ;
		if (children_to_start > sample_key_stack.size()) { children_to_start = sample_key_stack.size() ; }

		for (int i = 0; i < children_to_start; i++) {

			std::string key = sample_key_stack.top() ; sample_key_stack.pop() ;

			children.push_back(std::async (read_id_barcodes, samples.at(key), workdir)) ;
		}

		if (children.empty() && sample_key_stack.empty()) { running = false ; }
	}

	// while (true) std::cout << "done\n" ; std::this_thread::sleep_for(span) ;

	return 0;
}
