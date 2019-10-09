#ifndef SUSPECT_READ_FASTQS_H
#define SUSPECT_READ_FASTQS_H

#include <iostream>
#include <stack>
#include <sstream>

#include "gzfiles.h"
#include "task_pool.h"
#include "gzchunksort.h"
#include "barcode_seqnum_read_ids.h"

using namespace std ;

const string SUSPECT_FASTQ_READ_ID_DELIM = "_-_" ;

void write_suspect_read_fastqs(int, unordered_map<string, Sample>&, Workdir&) ;

struct Write_suspect_read_fastqs_args {
	string suspect_bcsnrid_path ;
	string suspect_read_fastq_path ;
	Sample* sample_ptr ;
} ;

int write_suspect_read_fastqs_task_func(Task<int, Write_suspect_read_fastqs_args>) ;

struct Collect_suspect_read_fastqs_args {
	vector<string> suspect_read_fastq_paths ;
	string global_suspect_read_fastq_path ;
} ;

int collect_suspect_read_fastqs_task_func(Task<int, Collect_suspect_read_fastqs_args>) ;


#endif