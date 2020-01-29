#include "workdir.h"

using namespace std ;

Workdir::Workdir (Path base_dir_path, string star_ref_list, const unordered_map<string, Sample>& samples) {

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

	// overlap table paths
	this->unfiltered_overlap_path = this->base_dir_path.join("unfiltered_cell_overlap.tsv").to_string() ;
	this->filtered_overlap_path = this->base_dir_path.join("filtered_cell_overlap.tsv").to_string() ;

	// parse star reference paths and annotation paths
	vector<string> tokens ;
	string token ;
	istringstream token_stream (star_ref_list) ;
	while (getline(token_stream, token, ',')) tokens.push_back(token) ;

	if (tokens.size() % 2 != 0 || tokens.size() < 2) {
		throw runtime_error("could not parse star ref list, " + star_ref_list) ;
	}

	// check and store each reference and its relevant paths
	int adir_name_cnt = 1 ;
	for (int i = 0; i < tokens.size(); i += 2) {
		string star_ref_path = tokens[i] ;
		string annotation_gtf_path = tokens[i+1] ;

		// checkreference paths
		if (!Path(star_ref_path).is_dir()) {
			throw runtime_error("star ref is not an existant dir, " + star_ref_path) ;
		}
		if (!Path(annotation_gtf_path).is_file()) {
			throw runtime_error("star annotation gtf is not an existant file, " + annotation_gtf_path) ;
		}

		// store the reference an assign alignment and cug paths
		this->star_reference_paths.insert(star_ref_path) ;
		this->annotation_gtf_path_by_star_reference_path.insert(
			make_pair(star_ref_path, annotation_gtf_path)) ;

		// get count tag and star reference "name"
		string star_reference_name = Path(star_ref_path).get_filename() ;

		// get the alignment dir path for this star reference
		Path alignment_dir_path = this->cug_label_base_dir_path.join(
			star_reference_name + "-" + to_string(adir_name_cnt++)) ;

		// make the alignment dir
		alignment_dir_path.make_dir() ;

		// create the alignment sam path
		Path sam_path = alignment_dir_path.join("star_transcriptome_coordinates.sam") ;

		// create cug label path & chunks path
		Path cug_label_path = alignment_dir_path.join("cug_label.txt.gz") ;
		Path cug_label_chunks_path = alignment_dir_path.join("cug_label_chunks") ;

		// create swaps path
		Path called_swaps_path = alignment_dir_path.join("called_swaps.txt.gz") ;

		// add reference associated paths to maps
		this->alignment_dir_path_by_star_reference_path.insert(
			make_pair(star_ref_path, alignment_dir_path.to_string())) ;

		this->sam_path_by_star_reference_path.insert(
			make_pair(star_ref_path, sam_path.to_string())) ;

		this->cug_label_path_by_star_reference_path.insert(
			make_pair(star_ref_path, cug_label_path.to_string())) ;

		this->cug_label_chunks_path_by_star_reference_path.insert(
			make_pair(star_ref_path, cug_label_chunks_path.to_string())) ;

		this->called_swaps_path_by_star_reference_path.insert(
			make_pair(star_ref_path, called_swaps_path.to_string())) ;
	}

	// get paths for samples
	for (auto it = samples.begin(); it != samples.end(); ++it) {

		// parse sample iterator
		string sample_key = it->first ;
		const Sample& sample = it->second ;

		// store sample_key
		this->sample_keys.insert(sample_key) ;

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

string Workdir::get_suspect_read_fastq_path_list_string() { 

	stringstream ss_suspect_fq_list ;
	bool add_comma = false ;
	for (string sample_key : this->sample_keys) {

		if (add_comma) { ss_suspect_fq_list << "," ; }
		else { add_comma = true ; }

		ss_suspect_fq_list << this->get_suspect_read_fastq_path(sample_key) ;
	}

	return ss_suspect_fq_list.str() ; 
}

string Workdir::get_alignment_dir_path(string star_ref_path) {
	return this->alignment_dir_path_by_star_reference_path.at(star_ref_path) ;
}

string Workdir::get_sam_path(string star_ref_path) {
	return this->sam_path_by_star_reference_path.at(star_ref_path) ;
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
