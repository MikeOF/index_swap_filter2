#ifndef WORKDIR_H
#define WORKDIR_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "path.h"
#include "sample.h"

using namespace std ;

class Workdir {

	Path base_dir_path ;
	unordered_map<string, Path> sample_dir_by_sample_key ;
	vector<string> sample_keys ;

	unordered_map<string, string> bcsnrid_path_by_sample_key ;
	unordered_map<string, unordered_map<string, string>> bcsnrid_chunks_path_by_bc_fastq_path_by_sample_key ;

	unordered_map<string, string> suspect_bcsnrid_path_by_sample_key ;
	unordered_map<string, string> suspect_bcsnrid_chunks_path_by_sample_key ;

	unordered_map<string, string> swapped_in_read_ids_path_by_sample_key ; 

	public:

		Workdir (Path, const unordered_map<string, Sample>&) ;

		vector<string> get_sample_keys() { return sample_keys ; }

		string get_bcsnrid_path(string) ;
		string get_bcsnrid_chunks_path(string, string) ;
		string get_suspect_bcsnrid_path(string) ;
		string get_suspect_bcsnrid_chunks_path(string) ;
		string get_swapped_in_read_ids_path(string) ;
} ;

#endif