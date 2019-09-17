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
#include "gz_files.h"
#include "files.h"

struct Read_id_barcodes_args {
	std::string read_id_barcodes_path ;
	std::string fastq_path ;
	Sample * sample_ptr ;
};

int read_id_barcodes(Task<int, Read_id_barcodes_args>) ;

#endif