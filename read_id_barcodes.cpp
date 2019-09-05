#include "read_id_barcodes.h"

void read_id_barcodes(Sample sample, Workdir workdir) {

	Whitelist wlist = Whitelist(sample.get_whitelist_path().generic_string()) ;

	std::vector<std::string> barcode_fastq_paths = sample.get_barcode_fastq_paths() ;

	// create the output writer
	std::ofstream out_file (workdir.get_read_id_barcodes_path(sample.get_key()).generic_string(), std::ios_base::out | std::ios_base::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::output> outbuf;
	outbuf.push(boost::iostreams::gzip_compressor());
	outbuf.push(out_file) ;
	std::ostream outstream(&outbuf) ;

	for (std::string fq_path : barcode_fastq_paths) {

		// log activity
		std::string msg = sample.get_project_name() + " - " + sample.get_sample_name() + " :\n" ;
		msg += "\treading read-ids and barcodes from " + fq_path + "\n" ;

		std::cout << msg ;

		// create the input reader
		std::ifstream file(fq_path, std::ios_base::in | std::ios_base::binary);
	    boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
	    inbuf.push(boost::iostreams::gzip_decompressor());
	    inbuf.push(file);

	    //Convert streambuf to istream
	    std::istream instream(&inbuf);

	    // lines
	    std::string lines[4];
	    int cnt = 0;

        while (std::getline(instream, lines[cnt])) {

	    	cnt++;

	    	if (cnt == 4) { // then we have a complete sequence
    			cnt = 0;

				std::string read_id = lines[0].substr(0, lines[0].find_first_of(' ') + 1) ;
	    		std::string cell_bc = sample.parse_cell_barcode(lines[1]) ;
	    		std::string umi = sample.parse_umi(lines[1]) ;

	            // validat umi
	            if (umi.find_first_of('N') != std::string::npos) continue ;

	    		// get validated cell barcode
	    		std::string v_cell_bc = wlist.get_valid_barcode(cell_bc) ;
	    		
	    		if (!v_cell_bc.empty()) {
		    		outstream << v_cell_bc << umi << '\t' << read_id << std::endl ;
	    		}
	    	}
	    }

	    file.close() ;
	}
	out_file.close() ;
}
