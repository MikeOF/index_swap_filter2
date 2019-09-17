#include "read_id_barcodes.h"

int read_id_barcodes(Task<int, Read_id_barcodes_args> task) {

    // parse arguments
	std::string read_id_barcodes_path = task.args.read_id_barcodes_path ;
	std::string fastq_path = task.args.fastq_path ;
	Sample sample = * task.args.sample_ptr ; 

    // create the whitelist
	Whitelist wlist = Whitelist(sample.get_whitelist_path()) ;

	// create the output writer
	// std::ofstream out_file (read_id_barcodes_path, std::ios_base::out | std::ios_base::binary);
    // boost::iostreams::filtering_streambuf<boost::iostreams::output> outbuf;
	// outbuf.push(boost::iostreams::gzip_compressor());
	// outbuf.push(out_file) ;
	// std::ostream outstream(&outbuf) ;
    Gzouts gz_output ;
    gz_output.file_path = read_id_barcodes_path;

	// log activity
	std::string msg = sample.get_project_name() + " - " + sample.get_sample_name() + " :\n" ;
	msg += "\treading read-ids and barcodes from " + fastq_path + "\n" ;

	std::cout << msg ;

	// create the input reader
    // std::ifstream in_file(fastq_path, std::ios_base::in | std::ios_base::binary);
    // boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
    // inbuf.push(boost::iostreams::gzip_decompressor());
    // inbuf.push(in_file);
    Gzins gz_input ;
    gz_input.file_path = fastq_path ;

    //Convert streambuf to istream
    //std::istream instream(&inbuf);

    // lines
    std::string lines[4] ;
    int idx = 0 ;
    int seq_cnt = 0 ;

    while (!gz_input.finished) {

        gz_read_lines(&gz_input) ;

        for (std::string line : gz_input.line_vect) {

            lines[idx++] = line ;

            if (idx == 4) { // then we have a complete sequence
                idx = 0;
                seq_cnt++ ;

                std::string read_id = lines[0].substr(0, lines[0].find_first_of(' ')) ;
                std::string cell_bc = sample.parse_cell_barcode(lines[1]) ;
                std::string umi = sample.parse_umi(lines[1]) ;

                // validat umi
                if (umi.find_first_of('N') != std::string::npos) continue ;

                // get validated cell barcode
                std::string v_cell_bc = wlist.get_valid_barcode(cell_bc) ;
                
                if (!v_cell_bc.empty()) {

                    std::stringstream ss;
                    ss << v_cell_bc << umi << '\t' << read_id ;

                    gz_output.line_vect.push_back(ss.str()) ;
                }

                // write lines if we have a bunch
                if (gz_output.line_vect.size() > 50000) gz_write_lines(&gz_output) ;

                if (seq_cnt % 50000 == 0) { std::cout << sample.get_project_name() + " - " + sample.get_sample_name() + " - " + std::to_string(seq_cnt) + " sequences read\n" ; }
            }
        }

        if (gz_input.finished) break ;
    }

	gz_flush_close(&gz_output) ;

    std::cout << sample.get_project_name() + " - " + sample.get_sample_name() + " - finished\n" ; 

    return 0 ;
}
