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

	// make the CUG label dir path
	this->cug_label_base_dir_path = this->base_dir_path.join("cell_umi_gene_labels") ;
	this->cug_label_base_dir_path.make_dir() ;


	unordered_set<string> alignment_dir_path_set ;
	for (auto it = samples.begin(); it != samples.end(); ++it) {

		// parse sample iterator
		string sample_key = it->first ;
		const Sample& sample = it->second ;

		// create alignment dir / cug file to star ref mapping if necessary
		string star_reference_path = sample.get_star_reference_path() ; 

		if (this->alignment_dir_path_by_star_reference_path.find(star_reference_path) 
			== this->alignment_dir_path_by_star_reference_path.end()) {

			// get count tag and star reference "name"
			int dir_name_cnt = 1 ;
			string star_reference_name = Path(star_reference_path).get_filename() ;

			// get the alignment dir path for this star reference
			Path alignment_dir_path = this->cug_label_base_dir_path.join(
				star_reference_name + "-" + to_string(dir_name_cnt)) ;

			// make sure the alignment dir path is unique
			while(alignment_dir_path_set.find(alignment_dir_path.to_string()) != alignment_dir_path_set.end()) {

				alignment_dir_path = this->cug_label_base_dir_path.join(
					star_reference_name + "-" + to_string(++dir_name_cnt)) ;
			}
			alignment_dir_path_set.insert(alignment_dir_path.to_string()) ;

			// make the alignment dir
			alignment_dir_path.make_dir() ;

			// get annotation gtf path
			Path annotation_dir_path = Path(star_reference_path).join("annotation") ;
			if (!annotation_dir_path.is_dir()) {
				throw runtime_error("too many gtf files found in star reference dir, " + star_reference_path) ;
			}
			
			string annotation_gtf_path ;
			int gtfs_found = 0 ;
			for (string path : annotation_dir_path.get_dir_list()) {
				if (path.substr(path.size()-4) == ".gtf") {
					annotation_gtf_path = path ;
					gtfs_found++;
				}
			}
			if (gtfs_found > 1) throw runtime_error(
				"too many gtf files found in star reference dir, " + star_reference_path) ;
			if (gtfs_found == 0) throw runtime_error(
				"could not find a gtf file in star reference dir, " + star_reference_path) ;


			// create cug label path & chunks path
			Path cug_label_path = alignment_dir_path.join("cug_label.txt.gz") ;
			Path cug_label_chunks_path = alignment_dir_path.join("cug_label_chunks") ;

			// create swaps path
			Path called_swaps_path = alignment_dir_path.join("called_swaps.txt.gz") ;

			// add reference path & cug label path / alignment dir to map
			this->alignment_dir_path_by_star_reference_path.insert(
				make_pair(star_reference_path, alignment_dir_path.to_string())) ;
			this->cug_label_path_by_star_reference_path.insert(
				make_pair(star_reference_path, cug_label_path.to_string())) ;
			this->cug_label_chunks_path_by_star_reference_path.insert(
				make_pair(star_reference_path, cug_label_chunks_path.to_string())) ;
			this->annotation_gtf_path_by_star_reference_path.insert(
				make_pair(star_reference_path, annotation_gtf_path)) ;
			this->called_swaps_path_by_star_reference_path.insert(
				make_pair(star_reference_path, called_swaps_path.to_string())) ;
		}

		// store sample_key
		this->sample_keys.push_back(sample_key) ;

		// create sample work dir
		Path sample_dir_path = this->base_dir_path.join(sample_key) ;
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

unordered_set<string> Workdir::get_star_reference_paths() {
	unordered_set<string> star_ref_path_set ;
	for (auto it = this->alignment_dir_path_by_star_reference_path.begin(); 
		it != alignment_dir_path_by_star_reference_path.end(); ++it) {

		star_ref_path_set.insert(it->first) ;
	}
	return star_ref_path_set ;
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

string Workdir::get_alignment_dir_path(string star_ref_path) {
	return this->alignment_dir_path_by_star_reference_path.at(star_ref_path) ;
}

string Workdir::get_annotation_dir_path(string star_ref_path) {
	return this->annotation_gtf_path_by_star_reference_path.at(star_ref_path) ;
}

string Workdir::get_cug_label_path(string star_ref_path) {
	return this->cug_label_path_by_star_reference_path.at(star_ref_path) ;
}

string Workdir::get_cug_label_chunks_path(string star_ref_path) {
	return this->cug_label_chunks_path_by_star_reference_path.at(star_ref_path) ;
}

string  Workdir::get_called_swaps_path(string star_ref_path) {
	return this->called_swaps_path_by_star_reference_path.at(star_ref_path) ;
}

string Workdir::get_swapped_in_read_ids_path(string sample_key) {
	return this->swapped_in_read_ids_path_by_sample_key.at(sample_key) ;
}
