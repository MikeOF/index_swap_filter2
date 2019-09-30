#ifndef READ_ID_BARCODES_H
#define READ_ID_BARCODES_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>

#include "sample.h"
#include "whitelist.h"
#include "workdir.h"
#include "task_pool.h"
#include "gzfiles.h"
#include "path.h"

using namespace std ;

struct Extract_barcode_read_ids_args {
	string barcode_read_ids_path ;
	string fastq_path ;
	Sample * sample_ptr ;
};

int extract_barcode_read_ids(Task<int, Extract_barcode_read_ids_args>) ;

#endif