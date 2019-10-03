#ifndef READ_ID_BARCODES_H
#define READ_ID_BARCODES_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <tuple>

#include "sample.h"
#include "whitelist.h"
#include "workdir.h"
#include "task_pool.h"
#include "gzfiles.h"
#include "path.h"
#include "gzchunksort.h"

using namespace std ;

string get_bc_key(const string&) ;

struct Extract_barcode_read_ids_args {
	string barcode_read_id_chunks_path ;
	string fastq_path ;
	Sample * sample_ptr ;
};

tuple<string, vector<string>> extract_barcode_read_ids(Task<tuple<string, vector<string>>, Extract_barcode_read_ids_args>) ;

struct Collect_barcode_read_ids_args {
	vector<string> barcode_read_id_chunk_paths ;
	string read_ids_path ;
	Sample * sample_ptr ;
};

int collect_barcode_read_ids(Task<int, Collect_barcode_read_ids_args>) ;

#endif