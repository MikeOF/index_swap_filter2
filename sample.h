#ifndef SAMPLE_H
#define SAMPLE_H

#include <string>
#include <stack>
#include <sstream>
#include <vector>
#include <iostream>
#include <boost/filesystem.hpp>

class Sample {

int cell_bc_start , cell_bc_len ;
int umi_start , umi_len ;
std::string project_name , sample_name ;
boost::filesystem::path whitelist_path, fastq_dir_path, star_reference_path ;

public:

	Sample (std::string) ;

	std::string get_project_name() { return project_name.substr() ; }
	std::string get_sample_name() { return sample_name.substr() ; }
	boost::filesystem::path get_whitelist_path() { 
		return boost::filesystem::path(whitelist_path) ; 
	}
	boost::filesystem::path get_fastq_dir_path() { 
		return boost::filesystem::path(fastq_dir_path) ; 
	}
	boost::filesystem::path get_star_reference_path() { 
		return boost::filesystem::path(star_reference_path) ; 
	}
	std::string get_key() { return sample_name + "_" + project_name ; }

	std::string parse_cell_barcode(std::string) ;
	std::string parse_umi(std::string) ;
	std::stack<boost::filesystem::path> get_barcode_fastq_paths() ;
	std::string to_string() ;

};

#endif
