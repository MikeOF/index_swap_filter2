#ifndef SUSPECT_BARCODES_H
#define SUSPECT_BARCODES_H

#include <iostream>
#include <vector>
#include <tuple>
#include <stack>
#include <string>
#include <unordered_map>

#include "gzfiles.h"
#include "task_pool.h"

using namespace std ;

struct Write_out_suspect_barcodes_args {
	unordered_map<string, string> suspect_barcodes_path_by_sample_key ;
	unordered_map<string, string> sorted_barcode_read_ids_path_by_sample_key ;
	unordered_map<string, tuple<string, string>> sample_name_project_name_by_sample_key ;
	function<string (string)> key_getter ;
} ;

int write_out_suspect_barcodes(Task<int, Write_out_suspect_barcodes_args>) ;

#endif