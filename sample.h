#ifndef SAMPLE_H
#define SAMPLE_H

#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <iostream>

#include "files.h"

class Sample {

	std::string barcode_fq_pattern, sequence_fq_pattern ; 
	int cell_bc_start, cell_bc_len ;
	int umi_start, umi_len ;
	std::string project_name, sample_name ;
	std::string whitelist_path, fastq_dir_path, star_reference_path ;
	std::vector<std::string> barcode_fastq_paths ;
	std::vector<std::string> sequence_fastq_paths ;

	public:

		Sample (std::string) ;

		std::string get_project_name() { return std::string(project_name) ; }
		std::string get_sample_name() { return std::string(sample_name) ; }
		std::string get_whitelist_path() { return std::string(whitelist_path) ; }
		std::string get_fastq_dir_path() { return std::string(fastq_dir_path) ; }
		std::string get_star_reference_path() { return std::string(star_reference_path) ; }
		std::string get_key() { return sample_name + "_" + project_name ; }
		std::vector<std::string> get_barcode_fastq_paths() { return barcode_fastq_paths ; } 
		std::vector<std::string> get_sequence_fastq_paths() { return sequence_fastq_paths ; } 

		std::string parse_cell_barcode(std::string) ;
		std::string parse_umi(std::string) ;
		std::string to_string() ;

};

#endif
