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
#include "barcode_seqnum_read_ids.h"
#include "suspect_barcodes.h"

using namespace std ;

void show_usage(string name) {
    cerr << "Usage: " 
    << name 
    << " threads work_dir_path sample_def sample_def [sample_def ...]" << endl
    << "\tsample_def: <project_name>,<sample_name>,<whitelist_path>,<fastq_dir_path>,"
    << "<star_reference_path>,<barcode_key>,<barcode_fq_pattern>,<sequence_fq_pattern>" << endl
    << "\t\tbarcode_key: [N ...]C[C ...][N ...]U[U ...][N ...] "
    <<"or [N ...]U[U ...][N ...]C[C ...][N ...]"
    << endl ;
}

int main(int argc, char ** argv) {

	// parse and validate arguments
	if (argc < 5) { show_usage(argv[0]) ; exit(1) ; }

	// ----------------------------
	//   Threads
	// --------------------------

	int threads ;
	try { 
		threads = stoi(argv[1]) ; 

	} catch (const char* msg) { 
		cerr << "could not parse threads argument to and int, " << argv[1] << endl ;
		show_usage(argv[0]) ; 
		exit(1) ; 
	}

	if (threads < 1 || threads > 20) {
		cerr << "threads argument must be between 1 and 20 inclusive" << endl ;
		show_usage(argv[0]) ;
		exit(1) ;
	}

	// ----------------------------
	//   Samples
	// --------------------------

	// each sample
	unordered_map<string, Sample> samples ;
	try {
		for (int i = 3; i < argc; i++) { 

			// create the sample
			Sample sample (argv[i]) ;

			// make sure there are no duplicate samples
			if (samples.count(sample.get_key()) > 0) {
				cerr << "Duplicate Sample Defs for "<< sample.get_project_name() 
				<< " " << sample.get_sample_name() << endl ;
				show_usage(argv[0]) ; exit(1) ; 
			}

			// add it to the map
			samples.insert(make_pair(sample.get_key(), sample)) ;
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

	// ----------------------------
	//   Workdir
	// --------------------------

	// work dir path
	Path work_dir_path = Path(argv[2]) ;
	if (work_dir_path.exists()) {
		cerr << "work dir path, " << work_dir_path.to_string() 
		<< ", already exists, will not overwrite" << endl ;
		show_usage(argv[0]) ; exit(1) ; 
	}
	if (!work_dir_path.is_dir()) {
		cerr << "work dir path, " << work_dir_path.to_string() 
		<< ", does not have a directory for a parent" << endl ;
		show_usage(argv[0]) ; exit(1) ; 
	}

	// create the workdir 
	work_dir_path.make_dir() ;
	Workdir workdir = Workdir(work_dir_path, samples) ;

	// ----------------------------
	//   Run Tasks
	// --------------------------

	// read in barcode seq-num read ids into the workdir
	read_in_bcsnrid_lines(threads, samples, workdir) ;

	// write out the suspect read ids
	write_out_suspect_bcsnrid_lines(threads, samples, workdir) ;

	return 0;
}
