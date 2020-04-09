#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "log.h"
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
#include "filter_fastqs.h"
#include "check_cell_overlap.h"

using namespace std ;

void detect(int argc, char ** argv) ;
void filter(int argc, char ** argv) ;

void show_main_usage(string name) {
	stringstream ss ;
	ss << "Usage: " 
	<< name 
	<< " <command> <arguments>" << endl << endl  
	<< "\tcommands" << endl << endl
	<< "\t\tdetect - detect index swapped reads in a sequencing run" << endl << endl
	<< "\t\tfilter - filter reads from a set of fastqs" << endl << endl ;
	log_error_message(ss.str()) ;
}

int main(int argc, char ** argv) {

	// call appropriate function or show usage 
	if (argc < 2) { show_main_usage(argv[0]) ; exit(1) ; }

	if (strcmp(argv[1], "detect") == 0) {
		detect(argc, argv) ;
	} else if (strcmp(argv[1], "filter") == 0) {
		filter(argc, argv) ;
	} else {
		show_main_usage(argv[0]) ; exit(1) ;
	}
	return 0 ;
}

void show_detect_usage(string name) {
	stringstream ss ;
	ss << "Usage: " 
    << name 
    << " threads work_dir_path star_reference_list sample_def sample_def [sample_def ...]"
    << endl << endl
    << "\tstar_reference_list: <star ref path>,<gtf path>,[<star ref path>,<gtf path>, ...]"
    << endl << endl
    << "\tsample_def: <project_name>,<sample_name>,<whitelist_path>,<fastq_dir_path>,"
    << "<barcode_key>,<barcode_fq_pattern>,<sequence_fq_pattern>" << endl
    << "\t\tbarcode_key: [N ...]C[C ...][N ...]U[U ...][N ...] "
    <<"or [N ...]U[U ...][N ...]C[C ...][N ...]"
    << endl ;
	log_error_message(ss.str()) ;
}

void detect(int argc, char ** argv) {

	// logging stream
	stringstream ss ;

	// function name
	string function_name = string(argv[0]) + " " + string(argv[1]) ;

	// parse arguments
	if (argc < 7) { show_detect_usage(function_name) ; exit(1) ; }

	string threads_arg = argv[2] ;
	string workdir_path_arg = argv[3] ;
	string star_reference_list_arg = argv[4] ;

	vector<string> sample_def_vect_arg ;
	for (int i = 5; i < argc; i++) { sample_def_vect_arg.push_back(argv[i]) ; }

	// ----------------------------
	//   Threads
	// --------------------------

	int threads ;
	try { 
		threads = stoi(threads_arg) ; 

	} catch (const char* msg) { 
		ss.str("") ;
		ss << "could not parse threads argument to an int, " << threads_arg << endl ;
		log_error_message(ss.str()) ; 
		show_detect_usage(function_name) ; 
		exit(1) ; 
	}

	if (threads < 1 || threads > 32) {
		ss.str("") ;
		ss << "threads argument must be between 1 and 32 inclusive" << endl ;
		log_error_message(ss.str()) ; 
		show_detect_usage(function_name) ;
		exit(1) ;
	}

	// ----------------------------
	//   Samples
	// --------------------------

	// each sample
	unordered_map<string, Sample> samples ;
	try {
		for (string sample_def : sample_def_vect_arg) { 

			// create the sample
			Sample sample (sample_def) ;

			// make sure there are no duplicate samples
			if (samples.count(sample.get_key()) > 0) {
				ss.str("") ;
				ss << "Duplicate Sample Defs for "<< sample.get_project_name() 
				<< " " << sample.get_sample_name() << endl ;
				log_error_message(ss.str()) ; 
				show_detect_usage(function_name) ; exit(1) ; 
			}

			// add it to the map
			samples.insert(make_pair(sample.get_key(), sample)) ;
		}

	} catch (exception& e) {
		ss.str("") ;
		ss << e.what() << endl ; 
		log_error_message(ss.str()) ;
		show_detect_usage(function_name) ; 
		exit(1) ; 
	}

	// print off samples
	ss.str("") ;
	ss << endl << "Samples\n-------" << endl << endl ;
	log_message(ss.str()) ;
	for (auto it = samples.begin(); it != samples.end(); ++it) {
		ss.str("") ;
		ss << it->second.to_string() << endl << endl ;
		log_message(ss.str()) ;
	}

	// ----------------------------
	//   Workdir
	// --------------------------

	// work dir path
	Path work_dir_path (workdir_path_arg) ;

	if (work_dir_path.exists()) {
		ss.str("") ;
		ss << "work dir path, " << work_dir_path.to_string() 
		<< ", already exists, will not overwrite" << endl ;
		log_error_message(ss.str()) ; 
		show_detect_usage(function_name) ; exit(1) ; 
	}

	if (!work_dir_path.get_parent_path().is_dir()) {
		ss.str("") ;
		ss << "work dir path, " << work_dir_path.to_string() 
		<< ", does not have a directory for a parent" << endl ;
		log_error_message(ss.str()) ; 
		show_detect_usage(function_name) ; exit(1) ; 
	}

	// create the workdir 
	work_dir_path.make_dir() ;
	Workdir workdir (work_dir_path, star_reference_list_arg, samples) ;

	// ----------------------------
	//   Run Tasks
	// --------------------------

	try {
		// read in barcode seq-num read ids into the workdir
		write_bcsnrid_lines(threads, samples, workdir) ;
		
		// write out the suspect read ids
		determine_suspect_bcsnrid_lines(threads, samples, workdir) ;

		// write out suspect read fastqs
		write_suspect_read_fastqs(threads, samples, workdir) ;

		// write out suspect labels
		label_suspect_reads(threads, samples, workdir) ;

		// call swaps
		write_swapped_reads(threads, samples, workdir) ;

		// write out cell overlap
		check_cell_barcode_overlap(threads, samples, workdir) ;

	} catch (exception& e) {
		ss.str("") ;
		ss << e.what() << endl ; 
		log_error_message(ss.str()) ;
		exit(1) ; 
	}

	// ----------------------------
	//   Copy IHOP Read IDS to Fastq Dirs
	// --------------------------
	for (string sample_key : workdir.get_sample_keys()) {

		Path fastq_dir_path (samples.at(sample_key).get_fastq_dir_path()) ;
		Path swapped_in_read_ids_path (workdir.get_swapped_in_read_ids_path(sample_key)) ;

		// log activity
		ss.str("") ;
		ss << get_sample_log_header(samples.at(sample_key)) ;
		ss << "copying " << swapped_in_read_ids_path.to_relative_path_string() ;
		ss << " to the fastq dir" << endl ;
		log_message(ss.str()) ;

		Path output_file_path = fastq_dir_path.join(swapped_in_read_ids_path.get_filename()) ;
		copy_gz_file(swapped_in_read_ids_path.to_string(), output_file_path.to_string()) ;
	}
}

void show_filter_usage(string name) {
	stringstream ss ;
	ss << "Usage: " << name 
	<< " threads output_dir_path read_ids_to_exclude_gz_path fastq_gz_path [fastq_gz_path ...] " 
	<< endl << endl ;
	log_error_message(ss.str()) ;
}

void filter(int argc, char ** argv) {

	// logging stream
	stringstream ss ;

	// function name
	string function_name = string(argv[0]) + " " + string(argv[1]) ;

	// parse and validate arguments
	if (argc < 6) { show_filter_usage(function_name) ; exit(1) ; }

	// ----------------------------
	//   Threads
	// --------------------------

	int threads ;
	try { 
		threads = stoi(argv[2]) ; 

	} catch (const char* msg) {
		ss.str("") ;
		ss << "could not parse threads argument to an int, " << argv[2] << endl ;
		log_error_message(ss.str()) ;
		show_filter_usage(function_name) ; 
		exit(1) ; 
	}

	if (threads < 1 || threads > 32) {
		ss.str("") ;
		ss << "threads argument must be between 1 and 32 inclusive" << endl ;
		log_error_message(ss.str()) ;
		show_filter_usage(function_name) ;
		exit(1) ;
	}

	// ----------------------------
	//   Output Dir Path
	// --------------------------

	Path output_dir_path (argv[3]) ;
	if (output_dir_path.exists()) {
		ss.str("") ;
		ss << "output directory, " << output_dir_path.to_string() ;
		ss << ", already exists, will not overwrite" << endl ;
		log_error_message(ss.str()) ;
		show_filter_usage(function_name) ;
		exit(1) ;
	}
	if (!output_dir_path.get_parent_path().is_dir()) {
		ss.str("") ;
		ss << "output directory, " << output_dir_path.to_string() ;
		ss << ", does not have an existant parent directory" << endl ;
		log_error_message(ss.str()) ;
		show_filter_usage(function_name) ;
		exit(1) ;
	}

	// ----------------------------
	//   Exclude Read IDs Path
	// --------------------------

	Path read_ids_to_exclude_path (argv[4]) ;
	string read_ids_to_exclude_path_str = read_ids_to_exclude_path.to_string() ;
	string read_ids_to_exclude_filename =  read_ids_to_exclude_path.get_filename() ;

	if (!read_ids_to_exclude_path.is_file()) {
		ss.str("") ;
		ss << "read ids to exlude path, " << read_ids_to_exclude_path_str ;
		ss << ", is not an existant file" << endl ;
		log_error_message(ss.str()) ;
		show_filter_usage(function_name) ;
		exit(1) ;
	}
	if (read_ids_to_exclude_filename.find(".gz") != read_ids_to_exclude_filename.size() - 3) {
		ss.str("") ;
		ss << "read ids to exlude path, " << read_ids_to_exclude_path_str ;
		ss << ", does not look gzipped" << endl ;
		log_error_message(ss.str()) ;
		show_filter_usage(function_name) ;
		exit(1) ;
	}

	// ----------------------------
	//   Fastqs
	// --------------------------

	unordered_set<string> fastq_path_set ;
	for (int i = 5; i < argc; i++) {

		Path fastq_path (argv[i]) ;
		string fastq_path_str = fastq_path.to_string() ;
		string fastq_filename = fastq_path.get_filename() ;

		if (!fastq_path.is_file()) {
			ss.str("") ;
			ss << "fastq path, " << fastq_path_str ;
			ss << ", is not an existant file" << endl ;
			log_error_message(ss.str()) ;
			show_filter_usage(function_name) ;
			exit(1) ;
		}

		if (fastq_filename.find(".fastq.gz") != fastq_filename.size() - 9) {
			ss.str("") ;
			ss << "passed fastq path, " << fastq_path_str ; 
			ss << ", does not appear to be a fastq file" << endl ;
			log_error_message(ss.str()) ;
			show_filter_usage(function_name) ;
			exit(1) ;
		}

		fastq_path_set.insert(fastq_path_str) ;
	}

	// now filter the fastqs
	filter_fastqs(threads, output_dir_path.to_string(), read_ids_to_exclude_path_str, fastq_path_set) ;
}
