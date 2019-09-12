#include "suspect_barcodes.h"

void write_out_suspect_barcodes(std::vector<std::string> sample_keys, 
	std::unordered_map<std::string, Sample> samples, Workdir workdir) {

	// create tasks to read in barcodes for each sample
	std::stack<Task<Suspect_barcodes, Get_all_barcodes_args>> task_stack;
	for (std::string key : sample_keys) {

		// create the arguments
		Get_all_barcodes_args args ;
		args.sample_ptr = &samples.at(key); 
		args.workdir_ptr = &workdir;

		// create the task
		Task<Suspect_barcodes, Get_all_barcodes_args> task ; 
		task.func = get_all_barcodes ; 
		task.args = args ;

		task_stack.push(task) ;
	}
}

Suspect_barcodes get_all_barcodes(Task<Suspect_barcodes, Get_all_barcodes_args> task) {

	// parse task
	Sample sample = *task.args.sample_ptr ;
	Workdir workdir = *task.args.workdir_ptr ;

	std::unordered_set<std::string> barcodes = std::unordered_set<std::string> () ;

	// log the activity
	std::string msg = sample.get_project_name() + " - " + sample.get_sample_name() + " :\n" ;
	msg += "\treading in all barcodes\n" ;
	std::cout << msg ;

	int cnt = 0;
	for (std::string bc_fq_path : sample.get_barcode_fastq_paths()) {

		std::string rid_barcodes_path = workdir.get_read_id_barcodes_path(sample.get_key(), bc_fq_path) ;

		// create the input reader
		std::ifstream in_file(rid_barcodes_path, std::ios_base::in | std::ios_base::binary);
	    boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
	    inbuf.push(boost::iostreams::gzip_decompressor());
	    inbuf.push(in_file);

	    //Convert streambuf to istream
	    std::istream instream(&inbuf);

	    std::string line ;

        while (std::getline(instream, line)) {

        	cnt++ ;
        	if (cnt % 10000 == 0) { std::cout << std::to_string(cnt) + " barcodes read\n" << std::endl ; }

        	// grab the cell-umi barcode
        	barcodes.insert(line.substr(0, line.find('\t') - 1)) ;  
        }
	}

	// create return structure
	Suspect_barcodes suspect_barcodes;
	suspect_barcodes.sample_key = sample.get_key() ;
	suspect_barcodes.barcodes_ptr = &barcodes ;

	return suspect_barcodes ;
}
