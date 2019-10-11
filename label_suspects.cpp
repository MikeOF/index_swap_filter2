#include "label_suspects.h"

using namespace std ;

void label_suspect_reads(int threads, unordered_map<string, Sample>& samples, Workdir& workdir) {

	// run alignment tasks
	unordered_map<string, string> sam_path_by_cug_label_path ;
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
		sam_path_by_cug_label_path.insert(
			make_pair(workdir.get_cug_label_path(star_reference_path), sam_path)) ;
	}

	// create cug parsing tasks
	for (string star_reference_path : workdir.get_star_reference_paths()) {

		// create the task
		
		string annotation_gtf_path = workdir.get_annotation_dir_path(star_reference_path) ;

	}

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

int parse_sorted_cug_labels_task_func(Task<int, Parse_sorted_cug_labels_args> task) {



	return 0 ;
}
