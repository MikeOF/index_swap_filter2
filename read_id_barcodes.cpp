#include "read_id_barcodes.h"

void read_id_barcodes(Sample sample) {

	// Whitelist wlist = Whitelist(sample.get_whitelist_path().generic_string()) ;

	std::stack<boost::filesystem::path> fq_stack = sample.get_barcode_fastq_paths() ;

	while (!fq_stack.empty()) {
		boost::filesystem::path fq_path = fq_stack.top() ; fq_stack.pop() ;
		std::cout << fq_path.generic_string() << std::endl ;
	}
}
