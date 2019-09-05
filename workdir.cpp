#include "workdir.h"

Workdir::Workdir (boost::filesystem::path base_dir_path_arg, 
	std::unordered_map<std::string, Sample> samples) {

	// check the base dir
	if (!boost::filesystem::is_directory(base_dir_path_arg)) {
		throw "Workdir path: " + base_dir_path_arg.generic_string() + ", is not a directory" ;
	}
	if (!boost::filesystem::is_empty(base_dir_path_arg)) {
		throw "Workdir path: " + base_dir_path_arg.generic_string() + ", is not empty" ;
	}

	base_dir_path = base_dir_path_arg ;

	for (auto it = samples.begin(); it != samples.end(); ++it) {

		// parse sample iterator
		std::string key = it->first ;
		Sample sample = it->second ;

		// create sample work dir
		boost::filesystem::path sample_dir_path = boost::filesystem::path(base_dir_path) ;
		sample_dir_path /= key ;

		boost::filesystem::create_directory(sample_dir_path) ;

		sample_dir_by_sample_key.insert({{key, sample_dir_path}}) ;

		// create paths to files for swap detection
		boost::filesystem::path read_id_barcodes_path = boost::filesystem::path(sample_dir_path) ;
		boost::filesystem::path suspect_read_ids_path = boost::filesystem::path(sample_dir_path) ;
		boost::filesystem::path swapped_in_read_ids_path = boost::filesystem::path(sample_dir_path) ;

		read_id_barcodes_path /= sample.get_sample_name() + "_read_id_barcodes.txt" ;
		suspect_read_ids_path /= sample.get_sample_name() + "_suspect_read_ids.txt" ;
		swapped_in_read_ids_path /= sample.get_sample_name() + "_swapped_in_read_ids.gz" ;

		read_id_barcodes_path_by_sample_key.insert({{key, read_id_barcodes_path}}) ;
		suspect_read_ids_path_by_sample_key.insert({{key, suspect_read_ids_path}}) ;
		swapped_in_read_ids_path_by_sample_key.insert({{key, swapped_in_read_ids_path}}) ;
	}
}

boost::filesystem::path Workdir::get_read_id_barcodes_path(std::string key) {
	return boost::filesystem::path(read_id_barcodes_path_by_sample_key.at(key)) ;
}

boost::filesystem::path Workdir::get_suspect_read_ids_path(std::string key) {
	return boost::filesystem::path(suspect_read_ids_path_by_sample_key.at(key)) ;
}

boost::filesystem::path Workdir::get_swapped_in_read_ids_path(std::string key) {
	return boost::filesystem::path(swapped_in_read_ids_path_by_sample_key.at(key)) ;
}
