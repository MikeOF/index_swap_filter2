#include "write_barcode_seqnum_read_ids.h"

using namespace std ;

string get_barcode_from_bcsnrid_line(const string& line) { return line.substr(0, line.find('\t')) ; }

int get_seqnum_from_bcsnrid_line(const string& line) { 

    int start_pos = line.find('\t')+1 ;
    int length = line.find('\t', start_pos) - start_pos ;
    return stoi(line.substr(start_pos, length)) ;
}

string get_read_id_from_bcsnrid_line(const string& line) {
    
    return line.substr(line.find('\t', line.find('\t') + 1) + 1) ;
}

void write_bcsnrid_lines(int threads, unordered_map<string, Sample>& samples, Workdir& workdir) {

    // write out barcode seqnum read id chunks for each sample
    stack<Task<tuple<string, vector<string>>, Extract_bcsnrid_lines_args>> extract_task_stack ;

    // for each sample
    for (string sample_key : workdir.get_sample_keys()) {

        // for each barcode fastq
        for (string bc_fq_path : samples.at(sample_key).get_barcode_fastq_paths()) {

            // create the task
            Task<tuple<string, vector<string>>, Extract_bcsnrid_lines_args> task ; 
            task.func = extract_bcsnrid_lines_task_func ; 
            task.args.bcsnrid_chunks_path = workdir.get_bcsnrid_chunks_path(sample_key, bc_fq_path) ;
            task.args.bc_fastq_path = bc_fq_path ;
            task.args.sample_ptr = &samples.at(sample_key) ;

            extract_task_stack.push(task) ;
        }
    }
    stack<tuple<string, vector<string>>> bcsnrid_chunk_paths_stack = run_tasks(threads, extract_task_stack) ;

    // map bcsnrid chunks by sample
    unordered_map<string, vector<string>> chunk_files_by_sample_key ;
    while (!bcsnrid_chunk_paths_stack.empty()) {

        tuple<string, vector<string>> sample_key_and_bcsnrid_chunk = bcsnrid_chunk_paths_stack.top() ; 
        bcsnrid_chunk_paths_stack.pop() ;

        // parse sample key and vector of chunk file paths
        string sample_key = get<0>(bcsnrid_chunk_paths_stack.top()) ;
        vector<string> chunk_files = get<1>(bcsnrid_chunk_paths_stack.top()) ;

        // map the chunk file paths by the sample key
        if (chunk_files_by_sample_key.find(sample_key) == chunk_files_by_sample_key.end()) {

            chunk_files_by_sample_key.insert(make_pair(sample_key, chunk_files)) ;

        } else {
            chunk_files_by_sample_key.at(sample_key).insert(
                chunk_files_by_sample_key.at(sample_key).end(), chunk_files.begin(), chunk_files.end()) ;
        }
    }

    // create the collection task stack
    stack<Task<int, Collect_bcsnrid_lines_args>> collect_task_stack;
    for (auto it = chunk_files_by_sample_key.begin(); it != chunk_files_by_sample_key.end(); ++it) {

        // get chunk dirs for this sample
        vector<string> bcsnrid_chunk_dir_paths ;
        for (string bc_fq_path : samples.at(it->first).get_barcode_fastq_paths()) {
            bcsnrid_chunk_dir_paths.push_back(workdir.get_bcsnrid_chunks_path(it->first, bc_fq_path)) ;
        }

        // create the task
        Task<int, Collect_bcsnrid_lines_args> task ;
        task.func = collect_bcsnrid_lines_task_func ;
        task.args.bcsnrid_chunk_paths = it->second ;
        task.args.bcsnrid_chunk_dir_paths = bcsnrid_chunk_dir_paths ;
        task.args.bcsnrid_path = workdir.get_bcsnrid_path(it->first) ;
        task.args.sample_ptr = &samples.at(it->first) ;

        collect_task_stack.push(task) ;
    }

    run_tasks(threads, collect_task_stack) ;
}

tuple<string, vector<string>> extract_bcsnrid_lines_task_func(Task<tuple<string, vector<string>>, Extract_bcsnrid_lines_args> task) {

    // parse arguments
	string bcsnrid_chunks_path = task.args.bcsnrid_chunks_path ;
	string bc_fastq_path = task.args.bc_fastq_path ;
	Sample& sample = * task.args.sample_ptr ; 

    // create the whitelist
	Whitelist wlist = Whitelist(sample.get_whitelist_path()) ;

	// log materials
    string log_header = get_sample_log_header(sample) ;
    string bc_fastq_relative_path_string = Path(bc_fastq_path).to_relative_path_string() ;

    // log beginning
	stringstream ss << log_header << "reading barcode seqnum read id lines from " ;
    ss << bc_fastq_relative_path_string << endl ;
    log_message(ss.str()) ;

    // create the output writer
    GzChunkSortWriter<string> gz_csw_out (bcsnrid_chunks_path) ;

	// create the input reader
    Gzin gzin (bc_fastq_path) ;

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

                ss << '\t' << seq_cnt << '\t' << read_id ;

                gz_csw_out.write_line(key, ss.str()) ;
            }
        }
    }

	gz_csw_out.flush_close() ;

    // log ending
    ss.str("") ;
    ss << log_header << to_string(seq_cnt) << " barcode seqnum read id lines read from " ;
    ss << bc_fastq_relative_path_string << endl ;
    log_message(ss.str()) ;

    return tuple<string, vector<string>> (sample.get_key(), gz_csw_out.get_files()) ;
}

int collect_bcsnrid_lines_task_func(Task<int, Collect_bcsnrid_lines_args> task) {

    Sample& sample = *(task.args.sample_ptr) ;

    // log activity
    string log_header = get_sample_log_header(sample) ;
    stringstream ss << log_header << "collecting sorted barcode seqnum read id lines" << endl ;
    log_message(ss.str()) ;

    // collect chunks
    int lines_written = collect_sorted_chunks<string>(task.args.bcsnrid_path, 
        task.args.bcsnrid_chunk_paths, get_barcode_from_bcsnrid_line) ;

    // log activity
    string bcsnrid_relative_path_string = Path(task.args.bcsnrid_path).to_relative_path_string() ;
    if (lines_written >= 0) {
        ss.str("") ;
        ss << log_header << to_string(lines_written) << " barcode seqnum read id lines written to " ;
        ss << bcsnrid_relative_path_string << endl ;
        log_message(ss.str()) ;
    } else {
        ss.str("") ;
        ss << log_header << "barcode seqnum read id lines copied to " << bcsnrid_relative_path_string << endl ;
        log_message(ss.str()) ;
    }

    // remove the chunk dirs
    for (string bcsnrid_chunk_dir_path : task.args.bcsnrid_chunk_dir_paths) {
        Path(bcsnrid_chunk_dir_path).remove_dir_recursively() ;
    }

    return 0 ;
}
