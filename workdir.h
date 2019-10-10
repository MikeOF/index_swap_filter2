#ifndef WORKDIR_H
#define WORKDIR_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "path.h"
#include "sample.h"

using namespace std ;

class Workdir {

	Path base_dir_path ;
	unordered_map<string, Path> sample_dir_by_sample_key ;
	vector<string> sample_keys ;
	string suspect_read_fastq_path ;
	Path cug_label_base_dir_path ;

	unordered_map<string, string> bcsnrid_path_by_sample_key ;
	unordered_map<string, unordered_map<string, string>> bcsnrid_chunks_path_by_bc_fastq_path_by_sample_key ;

	unordered_map<string, string> suspect_bcsnrid_path_by_sample_key ;
	unordered_map<string, string> suspect_bcsnrid_chunks_path_by_sample_key ;

	unordered_map<string, string> suspect_read_fastq_path_by_sample_key ;

	unordered_map<string, string> alignment_dir_path_by_star_reference_path ;
	unordered_map<string, string> cug_label_path_by_star_reference_path ;

	unordered_map<string, string> swapped_in_read_ids_path_by_sample_key ; 

	public:

		Workdir (Path, const unordered_map<string, Sample>&) ;

		vector<string> get_sample_keys() { return this->sample_keys ; }

		string get_suspect_read_fastq_path() { return this->suspect_read_fastq_path ; }

		unordered_set<string> get_star_reference_paths() ;

		string get_bcsnrid_path(string) ;
		string get_bcsnrid_chunks_path(string, string) ;
		string get_suspect_bcsnrid_path(string) ;
		string get_suspect_bcsnrid_chunks_path(string) ;
		string get_suspect_read_fastq_path(string) ;
		string get_alignment_dir_path(string) ;
		string get_cug_label_path(string) ;
		string get_swapped_in_read_ids_path(string) ;
} ;

#endif