#ifndef FILTER_FASTQS_H
#define FILTER_FASTQS_H

#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>

#include "path.h"
#include "gzfile.h"
#include "task_pool.h"

using namespace std ;

void filter_fastqs(int, string&, string&, unordered_set<string>&) ;

struct Filter_fastq_set_args {
	unordered_set<string> fastq_path_set ;
	unordered_set<string> * read_ids_to_exclude_set_ptr ;
	string output_dir_path ;
};

int filter_fastq_set_task_func(Task<int, Filter_fastq_set_args>) ;

#endif