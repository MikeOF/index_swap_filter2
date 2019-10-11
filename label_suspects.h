#ifndef LABEL_SUSPECTS_H
#define LABEL_SUSPECTS_H

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <stack>
#include <sstream>

#include "sample.h"
#include "whitelist.h"
#include "workdir.h"
#include "task_pool.h"
#include "path.h"

using namespace std ;

void label_suspect_reads(int, unordered_map<string, Sample>&, Workdir&) ;

struct Align_suspect_reads_args {
	string star_reference_path ;
	string alignment_dir_path ;
	string suspect_reads_fastq ;
} ;

int align_suspect_reads_task_func(Task<int, Align_suspect_reads_args>) ;

struct Parse_sorted_suspect_labels_args {
	unordered_map<string, string> sorted_label_path_by_sam_file_path ;
} ;

int parse_sorted_suspect_labels_task_func(Task<int, Parse_sorted_suspect_labels_args>) ;

#endif