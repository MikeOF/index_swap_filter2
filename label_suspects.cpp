#include "label_suspects.h"

using namespace std ;

void label_suspect_reads(int threads, unordered_map<string, Sample>& samples, Workdir& workdir) {

	// run alignment tasks
	for (string star_reference_path : workdir.get_star_reference_paths()) {

		Task<int, Align_suspect_reads_args> task ;
		task.func = align_suspect_reads_task_func ;
		task.args.star_reference_path = star_reference_path ;
		task.args.alignment_dir_path = workdir.get_alignment_dir_path(star_reference_path) ;
		task.args.suspect_reads_fastq = workdir.get_suspect_read_fastq_path() ;
		task.args.threads = threads ;

		stack<Task<int, Align_suspect_reads_args>> task_stack ;
		task_stack.push(task) ;

		run_tasks(1, task_stack) ;
	}
}

int align_suspect_reads_task_func(Task<int, Align_suspect_reads_args> task) {

	// Parse arguments
	string& star_reference_path = task.args.star_reference_path ;
	string& alignment_dir_path = task.args.alignment_dir_path ;
	string& suspect_reads_fastq = task.args.suspect_reads_fastq ;
	int threads = task.args.threads ;

	// create STAR command
	// self.command = " ".join([
    //        'STAR --runThreadN', str(self.threads),
    //        '--outSAMattributes NH NM AS nM',
    //        '--genomeDir', self.ref_dir,
    //        '--quantMode TranscriptomeSAM',
    //        '--outSAMtype BAM Unsorted',
    //        '--readFilesIn', self.read_file,
    //        '--readFilesCommand zcat ',
    //        '--outFileNamePrefix', self.out_prefix,
    //        '--outSAMprimaryFlag AllBestScore'
    //    ])
    stringstream ss ;
    ss << "STAR " ;
    ss << "--runThreadN " << to_string(threads) << " " ;
    ss << "--genomeDir " << star_reference_path << " " ;
    ss << "--quantMode TranscriptomeSAM " ;
    ss << "--outSAMtype SAM " ;
    ss << "--readFilesIn " << suspect_reads_fastq << " " ;
    ss << "--readFilesCommand zcat " ;
    ss << "--outFileNamePrefix " << alignment_dir_path << "/ " ;
    ss << "--outSAMprimaryFlag AllBestScore " ;

    // call STAR
    int result = system(ss.str().c_str()) ;

	return 0 ;
}

int parse_sorted_suspect_labels_task_func(Task<int, Parse_sorted_suspect_labels_args> task) {

	return 0 ;
}
