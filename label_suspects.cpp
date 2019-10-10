#include "label_suspects.h"

using namespace std ;

void label_suspect_reads(int threads, unordered_map<string, Sample>& samples, Workdir& workdir) {

	// run alignment tasks
	for (string star_reference_path : workdir.get_star_reference_paths()) {

		Task<string, Align_suspect_reads_args> task ;
		task.func = align_suspect_reads_task_func ;
		task.args.string star_reference_path = star_reference_path ;
		task.args.alignment_dir_path = workdir.get_alignment_dir_path(star_reference_path) ;
		task.args.cug_label_path = workdir.get_cug_label_path(star_reference_path) ;
		task.args.suspect_reads_fastq = workdir.get_suspect_read_fastq_path() ;

		stack<Task<string, Align_suspect_reads_args>> task_stack ;
		task_stack.push(task) ;

		run_tasks(1, task_stack) ;
	}
}

string align_suspect_reads_task_func(Task<string, Align_suspect_reads_args> task) {

	return "null" ;
}

int parse_sorted_suspect_labels_task_func(Task<int, Parse_sorted_suspect_labels_args> task) {

	return 0 ;
}
