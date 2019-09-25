#ifndef SUSPECT_BARCODES_H
#define SUSPECT_BARCODES_H

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

#include "gz_files.h"
#include "sample.h"
#include "workdir.h"
#include "task_pool.h"

// struct Suspect_barcodes {
// 	std::string sample_key ;
// 	std::unordered_set<std::string> barcodes ; 
// } ;

struct Get_all_barcodes_args {
	std::unordered_set<std::string> * suspect_barcodes_ptr ;
	Sample * sample_ptr ;
	Workdir * workdir_ptr ;
} ;

void write_out_suspect_barcodes(std::vector<std::string>, std::unordered_map<std::string, Sample>, Workdir, int) ;

int get_all_barcodes(Task<int, Get_all_barcodes_args>) ;

// int write_out_barcodes(Task<int>) ;

#endif