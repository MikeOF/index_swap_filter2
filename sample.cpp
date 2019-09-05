#include "sample.h"

Sample::Sample (std::string sample_def) {

	std::vector<std::string> tokens ;
	std::string token ;
	std::istringstream token_stream(sample_def) ;
	while (std::getline(token_stream, token, ',')) {
	  tokens.push_back(token) ;
	}

	if (tokens.size() != 6) {
		throw "could not parse sample definition: " + sample_def ;
	}

	project_name = tokens[0] ;
	sample_name = tokens[1] ;
	whitelist_path = boost::filesystem::path(tokens[2]) ;
	fastq_dir_path = boost::filesystem::path(tokens[3]) ;
	star_reference_path = boost::filesystem::path(tokens[4]) ;

	// check paths
	if (!boost::filesystem::is_regular_file(whitelist_path)) {
		throw "For Sample Def: " + sample_def 
		+ "\n\tWhitelist Path: " + whitelist_path.generic_string() + ", is not a file" ;
	}
	if (!boost::filesystem::is_directory(fastq_dir_path)) {
		throw "For Sample Def: " + sample_def 
		+ "\n\tFastq Dir Path: " + fastq_dir_path.generic_string() + ", is not a directory" ;
	}
	if (!boost::filesystem::is_directory(star_reference_path)) {
		throw "For Sample Def: " + sample_def 
		+ "\n\tStar Reference Path: " + star_reference_path.generic_string() + ", is not a directory" ;
	}

	std::string barcode_key = tokens[5] ;

	// make sure that only C and U are present in the barcode_key
	for (int i = 0; i < barcode_key.size(); i++) {
		if ((barcode_key[i] != 'C') && (barcode_key[i] != 'U')) {
			throw "could not parse barcode key: " + barcode_key ;
		}
	}

	cell_bc_start = barcode_key.find('C') ;
	cell_bc_len = barcode_key.rfind('C') - cell_bc_start + 1 ;
	umi_start = barcode_key.find('U'); 
	umi_len = barcode_key.rfind('U') - umi_start + 1;

	if (cell_bc_len + umi_len > barcode_key.size()) {
		throw "could not parse barcode key: " + barcode_key ;
	}
}

std::string Sample::parse_cell_barcode(std::string barcode) {
	return barcode.substr(cell_bc_start, cell_bc_len) ;
}

std::string Sample::parse_umi(std::string barcode) {
	return barcode.substr(umi_start, umi_len) ;
}

std::stack<boost::filesystem::path> Sample::get_barcode_fastq_paths() {

	std::stack<boost::filesystem::path> fastq_stack ;

	boost::filesystem::directory_iterator dit = boost::filesystem::directory_iterator(fastq_dir_path) ;

	for (boost::filesystem::directory_entry& entry : dit) {

		if (boost::filesystem::is_regular_file(entry.path())) {

			boost::filesystem::path file_path = entry.path() ;
			std::string file_name = file_path.filename().generic_string() ;

			if (file_name.size() > 9) {

				if (file_name.substr(file_name.size() - 9) == ".fastq.gz") {

					fastq_stack.push(file_path) ;
				}	
			}
		}
	}
	return fastq_stack ;
}

std::string Sample::to_string() {
	std::string sample_str = "Project Name: " + project_name ;
	sample_str +=  "\nSample Name: " + sample_name ;
	sample_str += "\n\tWhitelist Path: " + whitelist_path.generic_string() ;
	sample_str += "\n\tFastq Dir Path: " + fastq_dir_path.generic_string() ;
	sample_str += "\n\tStar Reference Path: " + star_reference_path.generic_string() ;
	sample_str += "\n\tCell Barcode Start: " + std::to_string(cell_bc_start)  ;
	sample_str += "\tCell Barcode Len: " + std::to_string(cell_bc_len) ;
	sample_str += "\n\tUMI Start: " + std::to_string(umi_start) ;
	sample_str += "\tUMI Len: " + std::to_string(umi_len) ;

	return sample_str ;
}