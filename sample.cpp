#include "sample.h"

Sample::Sample (std::string sample_def) {

	std::vector<std::string> tokens ;
	std::string token ;
	std::istringstream token_stream(sample_def) ;
	while (std::getline(token_stream, token, ',')) tokens.push_back(token) ;

	if (tokens.size() != 8) {
		throw std::runtime_error ("could not parse sample definition: " + sample_def) ;
	}

	project_name = tokens[0] ;
	sample_name = tokens[1] ;
	whitelist_path = tokens[2] ;
	fastq_dir_path = tokens[3] ;
	star_reference_path = tokens[4] ;

	// check paths
	if (!is_file(whitelist_path)) {
		throw std::runtime_error ("For Sample Def: " + sample_def 
		+ "\n\tWhitelist Path: " + whitelist_path + ", is not a file") ;
	}
	if (!is_dir(fastq_dir_path)) {
		throw std::runtime_error ("For Sample Def: " + sample_def 
		+ "\n\tFastq Dir Path: " + fastq_dir_path + ", is not a directory") ;
	}
	if (!is_dir(star_reference_path)) {
		throw std::runtime_error ("For Sample Def: " + sample_def 
		+ "\n\tStar Reference Path: " + star_reference_path + ", is not a directory") ;
	}

	// barcode and fastq definitions

	std::string barcode_key = tokens[5] ;
	barcode_fq_pattern = tokens[6] ;
	sequence_fq_pattern = tokens[7] ;

	// make sure that only C, N, and U are present in the barcode_key
	int num_N = 0;
	for (int i = 0; i < barcode_key.size(); i++) {

		if (barcode_key[i] == 'N') { num_N++ ; }

		else {
			if (barcode_key[i] != 'C' && barcode_key[i] != 'U' && barcode_key[i] != 'N') {
				throw std::runtime_error ("could not parse barcode key: " + barcode_key) ;
			}
		}
	}

	cell_bc_start = barcode_key.find('C') ;
	cell_bc_len = barcode_key.rfind('C') - cell_bc_start + 1 ;
	umi_start = barcode_key.find('U'); 
	umi_len = barcode_key.rfind('U') - umi_start + 1;

	if (cell_bc_len + umi_len + num_N != barcode_key.size()) {
		throw std::runtime_error ("could not parse barcode key: " + barcode_key) ;
	}

	// collect barcode and sequence fastqs
	std::vector<std::string> file_paths = get_dir_list(fastq_dir_path) ;

	for (std::string file_path : file_paths) {

		if (is_file(file_path)) {

			std::string file_name = get_filename(file_path) ;

			// check if this is a fastq for this sample
			if (file_name.substr(0, sample_name.size()) != sample_name) continue ;
			if (file_name.find(".fastq.gz") != file_name.size() - 9) continue ;

			// add to the appropriate vector
			if (file_name.find(barcode_fq_pattern) != std::string::npos) {
				barcode_fastq_paths.push_back(file_path) ;

			} else if (file_name.find(sequence_fq_pattern) != std::string::npos) {
				sequence_fastq_paths.push_back(file_path) ;
			}
		}
	}

	// make sure we found the fastqs
	if (barcode_fastq_paths.size() == 0 ) throw std::runtime_error (
		"No barcode fastqs found for " + project_name + " " + sample_name) ;

	if (sequence_fastq_paths.size() == 0) throw std::runtime_error (
		"No sequence fastqs found for " + project_name + " " + sample_name) ;
}

std::string Sample::parse_cell_barcode(std::string barcode) {
	return barcode.substr(cell_bc_start, cell_bc_len) ;
}

std::string Sample::parse_umi(std::string barcode) {
	return barcode.substr(umi_start, umi_len) ;
}

std::string Sample::to_string() {
	std::string sample_str = "Project Name: " + project_name ;
	sample_str +=  "\nSample Name: " + sample_name ;
	sample_str += "\n\tWhitelist Path: " + whitelist_path ;
	sample_str += "\n\tFastq Dir Path: " + fastq_dir_path ;
	sample_str += "\n\tStar Reference Path: " + star_reference_path ;
	sample_str += "\n\tCell Barcode Start: " + std::to_string(cell_bc_start)  ;
	sample_str += "\tCell Barcode Len: " + std::to_string(cell_bc_len) ;
	sample_str += "\n\tUMI Start: " + std::to_string(umi_start) ;
	sample_str += "\tUMI Len: " + std::to_string(umi_len) ;
	sample_str += "\n\tBarcode fastqs: " ;
	for (std::string p : barcode_fastq_paths) {
		sample_str += "\n\t\t" + p ;
	}
	sample_str += "\n\tSequence fastqs: " ;
	for (std::string p : sequence_fastq_paths) {
		sample_str += "\n\t\t" + p ;
	}

	return sample_str ;
}
