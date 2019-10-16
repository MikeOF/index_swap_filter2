#ifndef DETERMINE_SUSPECT_BARCODES_H
#define DETERMINE_SUSPECT_BARCODES_H

#include <iostream>
#include <vector>
#include <tuple>
#include <stack>
#include <string>
#include <unordered_map>

#include "gzfile.h"
#include "task_pool.h"
#include "gzchunksort.h"
#include "write_barcode_seqnum_read_ids.h"

using namespace std ;

void write_out_suspect_bcsnrid_lines(int, unordered_map<string, Sample>&, Workdir&) ;

struct Write_out_suspect_bcsnrid_lines_args {
	unordered_map<string, string> suspect_bcsnrid_chunks_path_by_sample_key ;
	unordered_map<string, string> bcsnrid_path_by_sample_key ;
	unordered_map<string, Sample*> sample_ptr_by_sample_key ;
} ;

unordered_map<string,vector<string>> write_out_suspect_bcsnrid_lines_task_func(
	Task<unordered_map<string,vector<string>>, Write_out_suspect_bcsnrid_lines_args>) ;

struct Collect_suspect_bcsnrid_lines_args {
	vector<string> suspect_bcsnrid_chunk_paths ;
	string suspect_bcsnrid_path ;
	Sample* sample_ptr ;
} ;

int collect_suspect_bcsnrid_lines_task_func(Task<int, Collect_suspect_bcsnrid_lines_args>) ;

#endif