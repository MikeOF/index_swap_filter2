#ifndef READ_ID_BARCODES_H
#define READ_ID_BARCODES_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "sample.h"
#include "whitelist.h"
#include "workdir.h"
#include "task_pool.h"

struct Read_id_barcodes_args {
	std::string read_id_barcodes_path ;
	std::string fastq_path ;
	Sample * sample_ptr ;
};

int read_id_barcodes(Task<int, Read_id_barcodes_args>) ;

#endif