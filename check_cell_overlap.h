#ifndef CHECK_CELL_OVERLAP_H
#define CHECK_CELL_OVERLAP_H

#include <unordered_set>
#include <set>
#include <unordered_map>
#include <string>
#include <stack>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <fstream>

#include "log.h"
#include "sample.h"
#include "workdir.h"
#include "task_pool.h"
#include "gzfile.h"
#include "path.h"
#include "task_pool.h"

using namespace std ;

const int CELL_UMI_COUNT_THRESHOLD = 300 ;

void check_cell_barcode_overlap(int, unordered_map<string, Sample>&, Workdir&) ;

struct Read_in_swapped_in_read_ids_args {
	string swapped_in_read_ids_path ;
	unordered_set<string> * read_ids_to_exclude_set_ptr ;
	Sample * sample_ptr ;
} ;

int read_in_swapped_in_read_ids_task_func(Task<int, Read_in_swapped_in_read_ids_args>) ;

struct Read_in_cell_barcode_counts_args {
	string bcsnrid_path ;
	unordered_set<string> * read_ids_to_exclude_set_ptr ;
	set<string> * unfiltered_cell_barcode_set_ptr ;
	set<string> * filtered_cell_barcode_set_ptr ;
	Sample * sample_ptr ;
} ;

int read_in_cell_barcode_counts_task_func(Task<int, Read_in_cell_barcode_counts_args>) ;

void write_cell_overlap(string, string, vector<string>&, unordered_map<string, string>&, 
	unordered_map<string, set<string>>&, unordered_map<string, set<string>>&) ;

#endif