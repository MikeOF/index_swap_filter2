#include "filter_fastqs.h"

using namespace std ;

void filter_fastqs(int threads, string output_dir_path, 
	string read_ids_to_exclude_path, unordered_set<string> fastq_path_set) {
	
}

struct Filter_fastq_set_args {
	unordered_set<string> fastq_set ;
	unordered_set<string> * read_ids_to_exclude ;
	string output_dir_path ;
};

int filter_fastq_set_task_func(Task<int, Filter_fastq_set_args>) ;
