#include "sample.h"

using namespace std ;

Sample::Sample (const string& sample_def) {

	// parse sample definition into tokens
	vector<string> tokens ;
	string token ;
	istringstream token_stream(sample_def) ;
	while (getline(token_stream, token, ',')) tokens.push_back(token) ;

	if (tokens.size() != 7) {
		throw runtime_error ("could not parse sample definition: " + sample_def) ;
	}

	project_name = tokens[0] ;
	sample_name = tokens[1] ;
	whitelist_path = tokens[2] ;
	fastq_dir_path = tokens[3] ;

	// check paths
	if (!Path(whitelist_path).is_file()) {
		throw runtime_error ("For Sample Def: " + sample_def 
		+ "\n\tWhitelist Path: " + whitelist_path + ", is not a file") ;
	}
	if (!Path(fastq_dir_path).is_dir()) {
		throw runtime_error ("For Sample Def: " + sample_def 
		+ "\n\tFastq Dir Path: " + fastq_dir_path + ", is not a directory") ;
	}

	// barcode and fastq definitions

	string barcode_key = tokens[4] ;
	barcode_fq_pattern = tokens[5] ;
	sequence_fq_pattern = tokens[6] ;

	// make sure that only C, N, and U are present in the barcode_key
	int num_N = 0;
	for (int i = 0; i < barcode_key.size(); i++) {

		if (barcode_key[i] == 'N') { num_N++ ; }

		else {
			if (barcode_key[i] != 'C' && barcode_key[i] != 'U' && barcode_key[i] != 'N') {
				throw runtime_error ("could not parse barcode key: " + barcode_key) ;
			}
		}
	}

	cell_bc_start = barcode_key.find('C') ;
	cell_bc_len = barcode_key.rfind('C') - cell_bc_start + 1 ;
	umi_start = barcode_key.find('U'); 
	umi_len = barcode_key.rfind('U') - umi_start + 1;

	if (cell_bc_len + umi_len + num_N != barcode_key.size()) {
		throw runtime_error ("could not parse barcode key: " + barcode_key) ;
	}

	// collect barcode and sequence fastqs
	vector<string> file_paths = Path(fastq_dir_path).get_dir_list() ;

	for (string file_path_str : file_paths) {

		Path file_path = Path(file_path_str) ;

		if (file_path.is_file()) {

			string filename = file_path.get_filename() ;

			// check if this is a fastq for this sample
			if (filename.substr(0, sample_name.size()) != sample_name) continue ;
			if (filename.find(".fastq.gz") != filename.size() - 9) continue ;

			// add to the appropriate vector
			if (filename.find(barcode_fq_pattern) != string::npos) {
				barcode_fastq_paths.push_back(file_path_str) ;

			} else if (filename.find(sequence_fq_pattern) != string::npos) {
				sequence_fastq_paths.push_back(file_path_str) ;
			}
		}
	}

	// make sure we found the fastqs
	if (barcode_fastq_paths.size() == 0 ) throw runtime_error (
		"No barcode fastqs found for " + project_name + " " + sample_name) ;

	if (sequence_fastq_paths.size() == 0) throw runtime_error (
		"No sequence fastqs found for " + project_name + " " + sample_name) ;

	// make sure we found the same number of barcode and sequence fastqs
	if (barcode_fastq_paths.size() != sequence_fastq_paths.size()) throw runtime_error(
		"different numbers of barcode and sequence fastqs were found for " + project_name + " " + sample_name) ;
}

string Sample::to_string() {
	string sample_str = "Project Name: " + project_name ;
	sample_str +=  "\nSample Name: " + sample_name ;
	sample_str += "\n\tWhitelist Path: " + whitelist_path ;
	sample_str += "\n\tFastq Dir Path: " + fastq_dir_path ;
	sample_str += "\n\tCell Barcode Start: " + ::to_string(cell_bc_start)  ;
	sample_str += "\tCell Barcode Len: " + ::to_string(cell_bc_len) ;
	sample_str += "\n\tUMI Start: " + ::to_string(umi_start) ;
	sample_str += "\tUMI Len: " + ::to_string(umi_len) ;
	sample_str += "\n\tBarcode fastqs: " ;
	for (string p : barcode_fastq_paths) {
		sample_str += "\n\t\t" + p ;
	}
	sample_str += "\n\tSequence fastqs: " ;
	for (string p : sequence_fastq_paths) {
		sample_str += "\n\t\t" + p ;
	}

	return sample_str ;
}
