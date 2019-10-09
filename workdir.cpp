#include "workdir.h"

using namespace std ;

Workdir::Workdir (Path base_dir_path, const unordered_map<string, Sample>& samples) {

	this->base_dir_path = base_dir_path ;

	// check the base dir
	if (!this->base_dir_path.is_dir()) {
		throw runtime_error("Workdir path: " + this->base_dir_path.to_string() + ", is not a directory") ;
	}
	if (!this->base_dir_path.dir_is_empty()) {
		throw runtime_error("Workdir path: " + this->base_dir_path.to_string() + ", is not empty") ;
	}

	// global paths
	this->suspect_read_fastq_path = this->base_dir_path.join("global_suspect_read.fastq.gz").to_string() ;

	for (auto it = samples.begin(); it != samples.end(); ++it) {

		// parse sample iterator
		string sample_key = it->first ;
		const Sample& sample = it->second ;

		// store sample_key
		this->sample_keys.push_back(sample_key) ;

		// create sample work dir
		Path sample_dir_path = base_dir_path.join(sample_key) ;
		sample_dir_path.make_dir() ;
		this->sample_dir_by_sample_key.insert(make_pair(sample_key, sample_dir_path)) ;

		// create paths to files for swap detection

		// create paths for reading in barcode seq-num read ids
		Path bcsnrid_path = sample_dir_path.join(sample.get_sample_name() + "_bcsnrid.txt.gz") ;
		this->bcsnrid_path_by_sample_key.insert(make_pair(sample_key, bcsnrid_path.to_string())) ;

		this->bcsnrid_chunks_path_by_bc_fastq_path_by_sample_key.insert(make_pair(sample_key, unordered_map<string, string>())) ;
		for (string bc_fq_path_str : sample.get_barcode_fastq_paths()) {

			// get the barcode fastq stem
			string file_stem = Path(bc_fq_path_str).get_filename_stem() ;

			// get the output file path
			Path bcsnrid_chunks_path = sample_dir_path.join(file_stem + "_bcsnrid_chunks") ;

			// store 
			this->bcsnrid_chunks_path_by_bc_fastq_path_by_sample_key.at(sample_key).insert(
				make_pair(bc_fq_path_str, bcsnrid_chunks_path.to_string())) ;
		}

		// create paths for suspects
		Path suspect_bcsnrid_path = sample_dir_path.join(sample.get_sample_name() + "_suspect_bcsnrid.txt.gz") ;
		this->suspect_bcsnrid_path_by_sample_key.insert(make_pair(sample_key, suspect_bcsnrid_path.to_string())) ;

		Path suspect_bcsnrid_chunks_path = sample_dir_path.join(sample.get_sample_name() + "_suspect_bcsnrid_chunks") ;
		this->suspect_bcsnrid_chunks_path_by_sample_key.insert(make_pair(sample_key, suspect_bcsnrid_chunks_path.to_string())) ;

		// path for suspect read fastqs
		Path suspect_read_fastq_path = sample_dir_path.join(sample.get_sample_name() + "_suspect_read.fastq.gz") ;
		this->suspect_read_fastq_path_by_sample_key.insert(make_pair(sample_key, suspect_read_fastq_path.to_string())) ;

		// path for swaps		
		Path swapped_in_read_ids_path = sample_dir_path.join(sample.get_sample_name() + "_INDEX_SWAP_CONTAMINANT_READ_IDS.txt.gz") ;
		this->swapped_in_read_ids_path_by_sample_key.insert(make_pair(sample_key, swapped_in_read_ids_path.to_string())) ;
	}
}

string Workdir::get_bcsnrid_path(string sample_key) {
	return this->bcsnrid_path_by_sample_key.at(sample_key) ;
}

string Workdir::get_bcsnrid_chunks_path(string sample_key, string barcode_fastq_key) {
	return this->bcsnrid_chunks_path_by_bc_fastq_path_by_sample_key.at(sample_key).at(barcode_fastq_key) ;
}

string Workdir::get_suspect_bcsnrid_path(string sample_key) {
	return this->suspect_bcsnrid_path_by_sample_key.at(sample_key) ;
}

string Workdir::get_suspect_bcsnrid_chunks_path(string sample_key) {
	return this->suspect_bcsnrid_chunks_path_by_sample_key.at(sample_key) ;
}
string Workdir::get_suspect_read_fastq_path(string sample_key) {
	return this->suspect_read_fastq_path_by_sample_key.at(sample_key) ;
}

string Workdir::get_swapped_in_read_ids_path(string sample_key) {
	return this->swapped_in_read_ids_path_by_sample_key.at(sample_key) ;
}
