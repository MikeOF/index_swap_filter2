#include "workdir.h"

Workdir::Workdir (std::string base_dir_path_arg, 
	std::unordered_map<std::string, Sample> samples) {

	// check the base dir
	if (!is_dir(base_dir_path_arg)) {
		throw "Workdir path: " + base_dir_path_arg + ", is not a directory" ;
	}
	if (!dir_is_empty(base_dir_path_arg)) {
		throw "Workdir path: " + base_dir_path_arg + ", is not empty" ;
	}

	base_dir_path = remove_trailing_slashes(base_dir_path_arg) ;

	for (auto it = samples.begin(); it != samples.end(); ++it) {

		// parse sample iterator
		std::string key = it->first ;
		Sample sample = it->second ;

		// store key
		sample_keys.push_back(key) ;

		// create sample work dir
		std::string sample_dir_path = base_dir_path + "/" ;
		sample_dir_path += key ;

		make_dir(sample_dir_path) ;

		sample_dir_by_sample_key.insert({{key, sample_dir_path}}) ;

		// create paths to files for swap detection
		std::string suspect_read_ids_path = sample_dir_path + "/" ;
		std::string swapped_in_read_ids_path = sample_dir_path + "/" ;

		suspect_read_ids_path += sample.get_sample_name() + "_suspect_read_ids.gz" ;
		swapped_in_read_ids_path += sample.get_sample_name() + "_index_swap_contaminant_read_ids.txt.gz" ;

		suspect_read_ids_path_by_sample_key.insert({{key, suspect_read_ids_path}}) ;
		swapped_in_read_ids_path_by_sample_key.insert({{key, swapped_in_read_ids_path}}) ;

		read_id_barcodes_path_by_fastq_path_by_sample_key.insert({{key, std::unordered_map<std::string, std::string>()}}) ;
		for (std::string bc_fq_path_str : sample.get_barcode_fastq_paths()) {

			// get the barcode fastq stem
			std::string file_stem = get_file_stem(bc_fq_path_str) ;

			// get the output file path
			std::string read_id_barcodes_path = sample_dir_path + "/" ;
			read_id_barcodes_path += file_stem + "_read_id_barcodes.txt.gz" ;

			// store 
			read_id_barcodes_path_by_fastq_path_by_sample_key.at(key).insert({
				{bc_fq_path_str, read_id_barcodes_path}
			}) ;
		}
	}
}

std::string Workdir::get_read_id_barcodes_path(std::string sample_key, std::string barcode_fastq_key) {
	return std::string(read_id_barcodes_path_by_fastq_path_by_sample_key.at(sample_key).at(barcode_fastq_key)) ;
}

std::string Workdir::get_suspect_read_ids_path(std::string key) {
	return std::string(suspect_read_ids_path_by_sample_key.at(key)) ;
}

std::string Workdir::get_swapped_in_read_ids_path(std::string key) {
	return std::string(swapped_in_read_ids_path_by_sample_key.at(key)) ;
}
