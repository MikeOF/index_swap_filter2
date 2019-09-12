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

		// store key
		sample_keys.push_back(key) ;

		// create sample work dir
		boost::filesystem::path sample_dir_path = boost::filesystem::path(base_dir_path) ;
		sample_dir_path /= key ;

		boost::filesystem::create_directory(sample_dir_path) ;

		sample_dir_by_sample_key.insert({{key, sample_dir_path.generic_string()}}) ;

		// create paths to files for swap detection
		boost::filesystem::path suspect_read_ids_path = boost::filesystem::path(sample_dir_path) ;
		boost::filesystem::path swapped_in_read_ids_path = boost::filesystem::path(sample_dir_path) ;

		suspect_read_ids_path /= sample.get_sample_name() + "_suspect_read_ids.gz" ;
		swapped_in_read_ids_path /= sample.get_sample_name() + "_index_swap_contaminant_read_ids.txt.gz" ;

		suspect_read_ids_path_by_sample_key.insert({{key, suspect_read_ids_path.generic_string()}}) ;
		swapped_in_read_ids_path_by_sample_key.insert({{key, swapped_in_read_ids_path.generic_string()}}) ;

		read_id_barcodes_path_by_fastq_path_by_sample_key.insert({{key, std::unordered_map<std::string, std::string>()}}) ;
		for (std::string bc_fq_path_str : sample.get_barcode_fastq_paths()) {

			// get the barcode fastq stem
			boost::filesystem::path bc_fq_path = boost::filesystem::path(bc_fq_path_str) ;
			boost::filesystem::path file_stem = bc_fq_path.filename() ;
			while (!file_stem.extension().empty()) { file_stem = file_stem.stem() ; }

			// get the output file path
			boost::filesystem::path read_id_barcodes_path = boost::filesystem::path(sample_dir_path) ;
			read_id_barcodes_path /= file_stem.generic_string() + "_read_id_barcodes.txt.gz" ;

			// store 
			read_id_barcodes_path_by_fastq_path_by_sample_key.at(key).insert({
				{bc_fq_path_str, read_id_barcodes_path.generic_string()}
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
