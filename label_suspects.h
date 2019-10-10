#ifndef BARCODE_SEQNUM_READ_IDS_H
#define BARCODE_SEQNUM_READ_IDS_H

#include <unordered_set>

using namespace std ;

void label_suspect_reads(int, unordered_map<string, Sample>&, Workdir&) ;

struct Align_suspect_reads_args {
	string star_reference_path ;
	string alignment_dir_path ;
	string cug_label_path ;
	string suspect_reads_fastq ;
} ;

unordered_map<string, string> align_suspect_reads_task_func(
	Task<unordered_map<string, string>, Align_suspect_reads_args>) ;

struct Parse_sorted_suspect_labels_args {
	unordered_map<string, string> sorted_label_path_by_sam_file_path ;
} ;

int parse_sorted_suspect_labels_task_func(Task<int, Parse_sorted_suspect_labels_args>) ;

#endif