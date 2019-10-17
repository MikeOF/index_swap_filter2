#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "path.h"
#include "sample.h"
#include "workdir.h"
#include "whitelist.h"
#include "task_pool.h"
#include "write_barcode_seqnum_read_ids.h"
#include "determine_suspect_barcodes.h"
#include "write_suspect_fastq.h"
#include "label_suspects.h"
#include "call_swapped_reads.h"

using namespace std ;

void detect(int argc, char ** argv) ;
void filter(int argc, char ** argv) ;

void show_main_usage(string name) {
	cerr << "Usage: " 
	<< name 
	<< " <command> <arguments>" << endl << endl  
	<< "\tcommands" << endl << endl
	<< "\t\tdetect - detect index swapped reads in a sequencing run" << endl << endl
	<< "\t\tfilter - filter reads from a set of fastqs" << endl << endl ;
}

int main(int argc, char ** argv) {

	// call appropriate function or show usage 
	if (argc < 2) { show_main_usage(argv[0]) ; exit(1) ; }

	if (argv[1] == 'detect') {
		detect(argc, argv) ;
	} else if (argv[1] == 'filter') {
		filter(argc, argv) ;
	} else {
		show_main_usage(argv[0]) ; exit(1) ;
	}
	return 0 ;
}

void show_detect_usage(string name) {
    cerr << "Usage: " 
    << name 
    << " threads work_dir_path sample_def sample_def [sample_def ...]" << endl
    << "\tsample_def: <project_name>,<sample_name>,<whitelist_path>,<fastq_dir_path>,"
    << "<star_reference_path>,<barcode_key>,<barcode_fq_pattern>,<sequence_fq_pattern>" << endl
    << "\t\tbarcode_key: [N ...]C[C ...][N ...]U[U ...][N ...] "
    <<"or [N ...]U[U ...][N ...]C[C ...][N ...]"
    << endl ;
}

void detect(int argc, char ** argv) {

	// function name
	string function_name = string(argv[0]) + string(argv[1]) ;

	// parse and validate arguments
	if (argc < 6) { show_detect_usage(function_name) ; exit(1) ; }

	// ----------------------------
	//   Threads
	// --------------------------

	int threads ;
	try { 
		threads = stoi(argv[2]) ; 

	} catch (const char* msg) { 
		cerr << "could not parse threads argument to an int, " << argv[2] << endl ;
		show_detect_usage(function_name) ; 
		exit(1) ; 
	}

	if (threads < 1 || threads > 20) {
		cerr << "threads argument must be between 0 and 21" << endl ;
		show_detect_usage(function_name) ;
		exit(1) ;
	}

	// ----------------------------
	//   Samples
	// --------------------------

	// each sample
	unordered_map<string, Sample> samples ;
	try {
		for (int i = 4; i < argc; i++) { 

			// create the sample
			Sample sample (argv[i]) ;

			// make sure there are no duplicate samples
			if (samples.count(sample.get_key()) > 0) {
				cerr << "Duplicate Sample Defs for "<< sample.get_project_name() 
				<< " " << sample.get_sample_name() << endl ;
				show_detect_usage(function_name) ; exit(1) ; 
			}

			// add it to the map
			samples.insert(make_pair(sample.get_key(), sample)) ;
		}

	} catch (exception& e) { 
		cerr << e.what() << endl ; 
		show_detect_usage(function_name) ; 
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
	Path work_dir_path (argv[2]) ;

	if (work_dir_path.exists()) {
		cerr << "work dir path, " << work_dir_path.to_string() 
		<< ", already exists, will not overwrite" << endl ;
		show_detect_usage(function_name) ; exit(1) ; 
	}

	if (!work_dir_path.get_parent_path().is_dir()) {
		cerr << "work dir path, " << work_dir_path.to_string() 
		<< ", does not have a directory for a parent" << endl ;
		show_detect_usage(function_name) ; exit(1) ; 
	}

	// create the workdir 
	work_dir_path.make_dir() ;
	Workdir workdir (work_dir_path, argv[3], samples) ;

	// ----------------------------
	//   Run Tasks
	// --------------------------

	// read in barcode seq-num read ids into the workdir
	read_in_bcsnrid_lines(threads, samples, workdir) ;

	// write out the suspect read ids
	write_out_suspect_bcsnrid_lines(threads, samples, workdir) ;

	// write out suspect read fastqs
	write_suspect_read_fastqs(threads, samples, workdir) ;

	// write out suspect labels
	label_suspect_reads(threads, samples, workdir) ;

	// call swaps
	write_swapped_reads(threads, samples, workdir) ;
}
