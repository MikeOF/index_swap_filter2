#include "barcode_read_ids.h"

using namespace std ;

string get_bc_key(const string& line) { return line.substr(0, line.find('\t')) ; }

tuple<string, vector<string>> extract_barcode_read_ids(Task<tuple<string, vector<string>>, Extract_barcode_read_ids_args> task) {

    // parse arguments
	string barcode_read_id_chunks_path = task.args.barcode_read_id_chunks_path ;
	string fastq_path = task.args.fastq_path ;
	Sample& sample = * task.args.sample_ptr ; 

    // create the whitelist
	Whitelist wlist = Whitelist(sample.get_whitelist_path()) ;

	// log activity
    string log_header = sample.get_project_name() + " - " + sample.get_sample_name() + " : " ;
	cout << log_header + "reading barcode read id lines from " + fastq_path + "\n";

    // create the output writer
    GzChunkSortWriter gszout = GzChunkSortWriter(barcode_read_id_chunks_path) ;

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
                ss << v_cell_bc << umi ;

                string key = ss.str() ;

                ss << '\t' << read_id ;

                gszout.write_line(key, ss.str()) ;
            }
        }
    }

	gszout.flush_close() ;
    cout << log_header + to_string(seq_cnt) + " sequences read from " + fastq_path + "\n" ;

    return tuple<string, vector<string>> (sample.get_key(), gszout.get_files()) ;
}

int collect_barcode_read_ids(Task<int, Collect_barcode_read_ids_args> task) {

    // log activity
    string log_header = task.args.sample_ptr->get_project_name() + " - " + task.args.sample_ptr->get_sample_name() + " : " ;
    cout << log_header + "collecting sorted barcode read id files into " + task.args.read_ids_path + "\n";

    int lines_written = collect_sorted_chunks(task.args.read_ids_path, task.args.barcode_read_id_chunk_paths, get_bc_key) ;

    // log activity
    cout << log_header +  to_string(lines_written) + " barcode read id lines written to " + task.args.read_ids_path + "\n";

    return 0 ;
}
