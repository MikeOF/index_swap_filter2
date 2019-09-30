#include "workdir.h"

using namespace std ;

Workdir::Workdir (Path base_dir_path, const unordered_map<string, Sample>& samples) {

	base_dir_path = base_dir_path ;

	// check the base dir
	if (!base_dir_path.is_dir()) {
		throw runtime_error("Workdir path: " + base_dir_path.to_string() + ", is not a directory") ;
	}
	if (!base_dir_path.dir_is_empty()) {
		throw runtime_error("Workdir path: " + base_dir_path.to_string() + ", is not empty") ;
	}

	for (auto it = samples.begin(); it != samples.end(); ++it) {

		// parse sample iterator
		string key = it->first ;
		const Sample& sample = it->second ;

		// store key
		sample_keys.push_back(key) ;

		// create sample work dir
		Path sample_dir_path = base_dir_path.join(key) ;
		sample_dir_path.make_dir() ;

		sample_dir_by_sample_key.insert({{key, sample_dir_path}}) ;

		// create paths to files for swap detection
		Path suspect_read_ids_path = sample_dir_path.join(sample.get_sample_name() + "_suspect_read_ids.gz") ;
		Path swapped_in_read_ids_path = sample_dir_path.join(sample.get_sample_name() + "_index_swap_contaminant_read_ids.txt.gz") ;

		suspect_read_ids_path_by_sample_key.insert({{key, suspect_read_ids_path.to_string()}}) ;
		swapped_in_read_ids_path_by_sample_key.insert({{key, swapped_in_read_ids_path.to_string()}}) ;

		barcode_read_ids_path_by_fastq_path_by_sample_key.insert({{key, unordered_map<string, string>()}}) ;
		for (string bc_fq_path_str : sample.get_barcode_fastq_paths()) {

			// get the barcode fastq stem
			string file_stem = Path(bc_fq_path_str).get_filename_stem() ;

			// get the output file path
			string read_id_barcodes_path = sample_dir_path.join(file_stem + "_read_id_barcodes.txt.gz").to_string() ;

			// store 
			barcode_read_ids_path_by_fastq_path_by_sample_key.at(key).insert({
				{bc_fq_path_str, read_id_barcodes_path}
			}) ;
		}
	}
}

string Workdir::get_barcode_read_ids_path(string sample_key, string barcode_fastq_key) {
	return barcode_read_ids_path_by_fastq_path_by_sample_key.at(sample_key).at(barcode_fastq_key) ;
}

string Workdir::get_suspect_read_ids_path(string key) {
	return suspect_read_ids_path_by_sample_key.at(key) ;
}

string Workdir::get_swapped_in_read_ids_path(string key) {
	return swapped_in_read_ids_path_by_sample_key.at(key) ;
}
