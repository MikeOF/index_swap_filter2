#include "label_suspects.h"

using namespace std ;

void label_suspect_reads(int threads, unordered_map<string, Sample>& samples, Workdir& workdir) {

	// run alignment tasks
	unordered_map<string, string> sam_path_by_star_reference_path ;
	for (string star_reference_path : workdir.get_star_reference_paths()) {

		// create task for this star reference
		Task<string, Align_suspect_reads_args> task ;
		task.func = align_suspect_reads_task_func ;
		task.args.star_reference_path = star_reference_path ;
		task.args.alignment_dir_path = workdir.get_alignment_dir_path(star_reference_path) ;
		task.args.suspect_reads_fastq = workdir.get_suspect_read_fastq_path() ;
		task.args.threads = threads ;

		stack<Task<string, Align_suspect_reads_args>> task_stack ;
		task_stack.push(task) ;

		// run alignment task
		stack<string> return_stack = run_tasks(1, task_stack) ;
		string sam_path = return_stack.top() ;

		// map sam path to cug label path
		sam_path_by_star_reference_path.insert(make_pair(star_reference_path, sam_path)) ;
	}

	// create cug parsing tasks
	stack<Task<int, Parse_sorted_cug_labels_args>> parse_task_stack ;
	for (string star_reference_path : workdir.get_star_reference_paths()) {

		// create the task
		Task<int, Parse_sorted_cug_labels_args> task ;
		task.func = parse_sorted_cug_labels_task_func ;
		task.args.sam_path = sam_path_by_star_reference_path.at(star_reference_path) ;
		task.args.cug_label_chunks_path = workdir.get_cug_label_chunks_path(star_reference_path) ;
		task.args.cug_label_path = workdir.get_cug_label_path(star_reference_path) ;
		task.args.annotation_gtf_path = workdir.get_annotation_dir_path(star_reference_path) ;

		parse_task_stack.push(task) ;
	}
	run_tasks(threads, parse_task_stack) ;

}

string align_suspect_reads_task_func(Task<string, Align_suspect_reads_args> task) {

	// Parse arguments
	string& star_reference_path = task.args.star_reference_path ;
	string& alignment_dir_path = task.args.alignment_dir_path ;
	string& suspect_reads_fastq = task.args.suspect_reads_fastq ;
	int threads = task.args.threads ;

	// create STAR command to align suspect reads
    stringstream ss_star_cmd ;
    ss_star_cmd << "STAR " ;
    ss_star_cmd << "--runThreadN " << to_string(threads) << " " ;
    ss_star_cmd << "--genomeDir " << star_reference_path << " " ;
    ss_star_cmd << "--quantMode TranscriptomeSAM " ;
    ss_star_cmd << "--outSAMtype SAM " ;
    ss_star_cmd << "--readFilesIn " << suspect_reads_fastq << " " ;
    ss_star_cmd << "--readFilesCommand zcat " ;
    ss_star_cmd << "--outFileNamePrefix " << alignment_dir_path << "/ " ;
    ss_star_cmd << "--outSAMprimaryFlag AllBestScore " ;
    ss_star_cmd << "--twopassMode Basic " ;
    ss_star_cmd << "--outFilterMultimapScoreRange 0 " ;

    // call STAR
    int result = system(ss_star_cmd.str().c_str()) ;

    // create samtools command to decompress BAM file
    Path alignment_dir_Path (alignment_dir_path) ;
    Path bam_path = alignment_dir_Path.join("Aligned.toTranscriptome.out.bam") ;
    Path sam_path = alignment_dir_Path.join("Aligned.toTranscriptome.out.sam") ;
    stringstream ss_samtools_cmd ;
    ss_samtools_cmd << "samtools view " ;
    ss_samtools_cmd << bam_path.to_string() ;
    ss_samtools_cmd << " > " ;
    ss_samtools_cmd << sam_path.to_string() ;

    // call samtools
    result = system(ss_samtools_cmd.str().c_str()) ;

	return sam_path.to_string() ;
}

string get_cug_key(string line) {
	int first_tab_pos = line.find('\t') ;
	int second_tab_pos = line.find('\t', first_tab_pos + 1) ;
	stringstream ss ;
	ss << line.substr(0, first_tab_pos) ;
	ss << "_" ;
	ss <<  line.substr(first_tab_pos + 1, second_tab_pos - first_tab_pos + 1) ;
	return ss.str() ;
}

int parse_sorted_cug_labels_task_func(Task<int, Parse_sorted_cug_labels_args> task) {

	// Parse arguments
	string& sam_path = task.args.sam_path ;
	string& cug_label_chunks_path = task.args.cug_label_chunks_path ;
	string& cug_label_path = task.args.cug_label_path ;
	string& annotation_gtf_path = task.args.annotation_gtf_path ;

	// read in annotation gtf to map transcripts to genes
	unordered_map<string, string> gene_id_by_transcript_id ;
	string line ;
	ifstream gtf_file (annotation_gtf_path) ;

	if (gtf_file.is_open()) {

		while (getline(gtf_file, line)) {

			// parse transcript lines
			if (line.find("\ttranscript\t") != string::npos) {

				int gene_id_start_pos = line.find("\"", line.find("gene_id \"") + 1) + 1 ;
				int gene_id_end_pos = line.find("\"", gene_id_start_pos) ;
				int transcript_id_start_pos = line.find("\"", line.find("transcript_id \"") + 1) + 1 ;
				int transcript_id_end_pos = line.find("\"", transcript_id_start_pos) ;

				string gene_id = line.substr(gene_id_start_pos, gene_id_end_pos - gene_id_start_pos) ;
				string transcript_id = line.substr(transcript_id_start_pos, 
					transcript_id_end_pos - transcript_id_start_pos) ;

				// map the transcript to the gene id
				if (gene_id_by_transcript_id.find(transcript_id) != gene_id_by_transcript_id.end()) {
					if (gene_id != gene_id_by_transcript_id.at(transcript_id)) {
						stringstream ss ;
						ss << "different gene ids for transcript id: " + transcript_id ;
						ss << " annotation gtf path: " + annotation_gtf_path ;
						ss << " gene_ids: " + gene_id + " " ;
						ss << gene_id_by_transcript_id.at(transcript_id) ;
						throw runtime_error(ss.str()) ; 
					}
				} else {
					gene_id_by_transcript_id.insert(make_pair(transcript_id, gene_id)) ;
				}
			}
		}
		gtf_file.close() ;

	} else { throw runtime_error("unable to open annotation gtf file: " + annotation_gtf_path) ; }

	// write out the cug lines
	ifstream sam_file (sam_path) ;
	if (sam_file.is_open()) {

		GzChunkSortWriter<string> gz_csw_out (cug_label_chunks_path) ;
		string current_read_id ;
		unordered_set<string> gene_id_set ;
		int delim_size = SUSPECT_FASTQ_READ_ID_DELIM.size() ;

		while(getline(sam_file, line)) {

			// get read id
			int read_id_stop = line.find(SUSPECT_FASTQ_READ_ID_DELIM) ;
			string read_id = line.substr(0, read_id_stop) ;

			// refresh current read id tracker
			if (read_id != current_read_id) {
				current_read_id = read_id ;
				gene_id_set.clear() ;
			}

			// get sample key
			int sample_key_start = read_id_stop + delim_size ;
			int sample_key_stop = line.find(SUSPECT_FASTQ_READ_ID_DELIM, sample_key_start) ;
			string sample_key = line.substr(sample_key_start, sample_key_stop - sample_key_start) ;

			// get barcode
			int barcode_start = sample_key_stop + delim_size ;
			int barcode_stop = line.find('\t', barcode_start) ;
			string barcode = line.substr(barcode_start, barcode_stop - barcode_start) ;

			// get transcript id & gene id
			int transcript_id_start = line.find('\t', barcode_stop + 1) + 1 ;
			int transcript_id_stop = line.find('\t', transcript_id_start) ;
			string transcript_id = line.substr(transcript_id_start, transcript_id_stop - transcript_id_start) ;
			string& gene_id = gene_id_by_transcript_id.at(transcript_id) ;

			// don't write out duplicate cug lines
			if (gene_id_set.find(gene_id) != gene_id_set.end()) continue ; 
			gene_id_set.insert(gene_id) ;

			// write cug line
			stringstream ss_key ;
			ss_key << barcode << "_" << gene_id ; 
			stringstream ss_line ;
			ss_line << barcode << '\t' << gene_id << '\t' << sample_key << '\t' << read_id ;

			gz_csw_out.write_line(ss_key.str(), ss_line.str()) ;
		}
		sam_file.close() ;
		gz_csw_out.flush_close();

	} else { throw runtime_error("unable to open sam file: " + sam_path) ; }

	collect_sorted_chunks<string>(cug_label_path, gz_csw_out.get_files(), get_cug_key) ;

	return 0 ;
}
