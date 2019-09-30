#include "barcode_read_ids.h"

using namespace std ;

int extract_barcode_read_ids(Task<int, Extract_barcode_read_ids_args> task) {

    // parse arguments
	string barcode_read_ids_path = task.args.barcode_read_ids_path ;
	string fastq_path = task.args.fastq_path ;
	Sample& sample = * task.args.sample_ptr ; 

    // create the whitelist
	Whitelist wlist = Whitelist(sample.get_whitelist_path()) ;

	// log activity
    string log_header = sample.get_project_name() + " - " + sample.get_sample_name() + " : " ;
	cout << log_header + "reading read-ids and barcodes from " + fastq_path + "\n";

    // create the output writer
    Gzout gzout = Gzout(barcode_read_ids_path) ;

	// create the input reader
    Gzin gzin (fastq_path) ;

    // lines
    string lines[4] ;
    int idx = 0 ;
    int seq_cnt = 0 ;

    while (gzin.has_next_line()) {

        lines[idx++] = gzin.read_line() ;

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

                gzout.write_line(ss.str()) ;
            }
        }
    }

	gzout.flush_close() ;
    cout << log_header + to_string(seq_cnt) + " sequences read from " + fastq_path + "\n" ;

    return 0 ;
}
