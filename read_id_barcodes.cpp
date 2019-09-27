#include "read_id_barcodes.h"

using namespace std ;

int read_id_barcodes(Task<int, Read_id_barcodes_args> task) {

    // parse arguments
	string read_id_barcodes_path = task.args.read_id_barcodes_path ;
	string fastq_path = task.args.fastq_path ;
	Sample sample = * task.args.sample_ptr ; 

    // create the whitelist
	Whitelist wlist = Whitelist(sample.get_whitelist_path()) ;

	// create the output writer
    Gzouts gz_output ;
    gz_output.file_path = read_id_barcodes_path ;
    gz_begin_gzouts(&gz_output) ;

	// log activity
    string log_header = sample.get_project_name() + " - " + sample.get_sample_name() + " : " ;
	cout << log_header + "reading read-ids and barcodes from " + fastq_path + "\n";

	// create the input reader
    Gzins gz_input ;
    gz_input.file_path = fastq_path ;
    gz_begin_gzins(&gz_input) ;

    // lines
    string lines[4] ;
    int idx = 0 ;
    int seq_cnt = 0 ;

    while (gz_input.has_next) {

        string line = gz_read_line(&gz_input) ;


        lines[idx++] = line ;

        if (idx == 4) { // then we have a complete sequence
            idx = 0;
            seq_cnt++ ;

            string read_id = lines[0].substr(0, lines[0].find_first_of(' ')) ;
            string cell_bc = sample.parse_cell_barcode(lines[1]) ;
            string umi = sample.parse_umi(lines[1]) ;

            // validate umi
            if (umi.find_first_of('N') != string::npos) continue ;

            // get validated cell barcode
            string v_cell_bc = wlist.get_valid_barcode(cell_bc) ;
            
            if (!v_cell_bc.empty()) {

                stringstream ss;
                ss << v_cell_bc << umi << '\t' << to_string(seq_cnt) << '\t' << read_id ;

                gz_write_line(&gz_output, ss.str()) ;
            }
        }
    }

	gz_flush_close(&gz_output) ;
    cout << log_header + to_string(seq_cnt) + " sequences read from " + fastq_path + "\n" ;

    return 0 ;
}
