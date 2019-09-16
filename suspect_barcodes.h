#ifndef SUSPECT_BARCODES_H
#define SUSPECT_BARCODES_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filter/gzip.hpp"

#include "sample.h"
#include "workdir.h"
#include "task_pool.h"

struct Suspect_barcodes {
	std::string sample_key ;
	std::unordered_set<std::string> * barcodes_ptr ; 
} ;

struct Get_all_barcodes_args {
	Sample * sample_ptr ;
	Workdir * workdir_ptr ;
} ;

void write_out_suspect_barcodes(std::vector<std::string>, std::unordered_map<std::string, Sample>, Workdir) ;

Suspect_barcodes get_all_barcodes(Task<Suspect_barcodes, Get_all_barcodes_args>) ;

// int write_out_barcodes(Task<int>) ;

#endif