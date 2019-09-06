#ifndef WORKDIR_H
#define WORKDIR_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <boost/filesystem.hpp>

#include "sample.h"

class Workdir {

	boost::filesystem::path base_dir_path ;
	std::unordered_map<std::string, std::string> sample_dir_by_sample_key ;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> read_id_barcodes_path_by_fastq_path_by_sample_key ;
	std::unordered_map<std::string, std::string> suspect_read_ids_path_by_sample_key ;
	std::unordered_map<std::string, std::string> swapped_in_read_ids_path_by_sample_key ;

	public:

		Workdir (boost::filesystem::path, std::unordered_map<std::string, Sample>) ;

		std::string get_read_id_barcodes_path(std::string, std::string) ;
		std::string get_suspect_read_ids_path(std::string) ;
		std::string get_swapped_in_read_ids_path(std::string) ;
};

#endif
