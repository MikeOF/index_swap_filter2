#ifndef SWAPPED_READS_H
#define SWAPPED_READS_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <stack>

#include "gzfiles.h"
#include "sample.h"
#include "workdir.h"
#include "task_pool.h"

using namespace std ;

void write_swapped_reads(int, unordered_map<string, Sample>&, Workdir&) ;

struct Call_swapped_reads_args {
	string cug_label_path ;
	string called_swaps_path ;
} ;

int call_swapped_reads_task_func(Task<int, Call_swapped_reads_args>) ;

struct Collect_swapped_reads_args {
	vector<string> called_swaps_paths ;
	string swapped_in_read_ids_path ;
	string sample_key ;
} ;

int collect_swapped_reads_task_func(Task<int, Collect_swapped_reads_args>) ;

#endif