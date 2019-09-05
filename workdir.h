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
	std::unordered_map<std::string, boost::filesystem::path> sample_dir_by_sample_key ;
	std::unordered_map<std::string, boost::filesystem::path> read_id_barcodes_path_by_sample_key ;
	std::unordered_map<std::string, boost::filesystem::path> suspect_read_ids_path_by_sample_key ;
	std::unordered_map<std::string, boost::filesystem::path> swapped_in_read_ids_path_by_sample_key ;

	public:

		Workdir (boost::filesystem::path, std::unordered_map<std::string, Sample>) ;

		boost::filesystem::path get_read_id_barcodes_path(std::string) ;
		boost::filesystem::path get_suspect_read_ids_path(std::string) ;
		boost::filesystem::path get_swapped_in_read_ids_path(std::string) ;

};

#endif
