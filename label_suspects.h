#ifndef LABEL_SUSPECTS_H
#define LABEL_SUSPECTS_H

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <stack>
#include <sstream>
#include <iostream>
#include <fstream>

#include "log.h"
#include "sample.h"
#include "workdir.h"
#include "task_pool.h"
#include "path.h"
#include "gzchunksort.h"
#include "write_suspect_fastq.h"

using namespace std ;

void label_suspect_reads(int, unordered_map<string, Sample>&, Workdir&) ;

struct Align_suspect_reads_args {
	string star_reference_path ;
	string alignment_dir_path ;
	string suspect_reads_fastq_list ;
	int threads ;
} ;

string align_suspect_reads_task_func(Task<string, Align_suspect_reads_args>) ;

struct Parse_sorted_cug_labels_args {
	string sam_path ;
	string cug_label_chunks_path ;
	string cug_label_path ;
	string annotation_gtf_path ;
} ;

int parse_sorted_cug_labels_task_func(Task<int, Parse_sorted_cug_labels_args>) ;

#endif