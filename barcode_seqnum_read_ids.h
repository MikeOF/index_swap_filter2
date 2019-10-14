#ifndef BARCODE_SEQNUM_READ_IDS_H
#define BARCODE_SEQNUM_READ_IDS_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <tuple>
#include <stdexcept>

#include "sample.h"
#include "whitelist.h"
#include "workdir.h"
#include "task_pool.h"
#include "gzfiles.h"
#include "path.h"
#include "gzchunksort.h"

using namespace std ;

string get_barcode_from_bcsnrid_line(const string&) ;
int get_seqnum_from_bcsnrid_line(const string&) ;
string get_read_id_from_bcsnrid_line(const string&) ;

void read_in_bcsnrid_lines(int, unordered_map<string, Sample>&, Workdir&) ;

struct Extract_bcsnrid_lines_args {
	string bcsnrid_chunks_path ;
	string bc_fastq_path ;
	Sample * sample_ptr ;
};

tuple<string, vector<string>> extract_bcsnrid_lines_task_func(Task<tuple<string, vector<string>>, Extract_bcsnrid_lines_args>) ;

struct Collect_bcsnrid_lines_args {
	vector<string> bcsnrid_chunk_paths ;
	string bcsnrid_path ;
	Sample * sample_ptr ;
};

int collect_bcsnrid_lines_task_func(Task<int, Collect_bcsnrid_lines_args>) ;

#endif