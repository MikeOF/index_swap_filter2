#ifndef SAMPLE_H
#define SAMPLE_H

#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "path.h"

using namespace std ;

class Sample {

	string barcode_fq_pattern, sequence_fq_pattern ; 
	int cell_bc_start, cell_bc_len ;
	int umi_start, umi_len ;
	string project_name, sample_name, sample_key ;
	string whitelist_path, fastq_dir_path, star_reference_path ;
	vector<string> barcode_fastq_paths ;
	vector<string> sequence_fastq_paths ;

	public:

		Sample (const string&) ;

		string get_project_name() const { return project_name ; }
		string get_sample_name() const { return sample_name ; }
		string get_whitelist_path() const { return whitelist_path ; }
		string get_fastq_dir_path() const { return fastq_dir_path ; }
		string get_star_reference_path() const { return star_reference_path ; }
		string get_key() const { return sample_name + "_" + project_name ; }
		vector<string> get_barcode_fastq_paths() const { return barcode_fastq_paths ; } 
		vector<string> get_sequence_fastq_paths() const { return sequence_fastq_paths ; } 

		string parse_cell_barcode(const string& barcode) { return barcode.substr(cell_bc_start, cell_bc_len) ; }
		string parse_umi(const string& barcode) { return barcode.substr(umi_start, umi_len) ; }
		string to_string() ;
};

#endif
