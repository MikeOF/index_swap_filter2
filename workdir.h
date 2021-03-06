#ifndef WORKDIR_H
#define WORKDIR_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>

#include "path.h"
#include "sample.h"

using namespace std ;

class Workdir {

	Path base_dir_path ;
	unordered_set<string> sample_keys ;
	string suspect_read_fastq_path ;
	Path cug_label_base_dir_path ;
	string unfiltered_overlap_path ;
	string filtered_overlap_path ;

	// per star ref
	unordered_set<string> star_reference_paths ;

	unordered_map<string, string> alignment_dir_path_by_star_reference_path ;

	unordered_map<string, string> sam_path_by_star_reference_path ;

	unordered_map<string, string> annotation_gtf_path_by_star_reference_path ;

	unordered_map<string, string> cug_label_path_by_star_reference_path ;
	unordered_map<string, string> cug_label_chunks_path_by_star_reference_path ;

	unordered_map<string, string> called_swaps_path_by_star_reference_path ;

	// per sample

	unordered_map<string, Path> sample_dir_by_sample_key ;

	unordered_map<string, string> bcsnrid_path_by_sample_key ;
	unordered_map<string, unordered_map<string, string>> bcsnrid_chunks_path_by_bc_fastq_path_by_sample_key ;

	unordered_map<string, string> suspect_bcsnrid_path_by_sample_key ;
	unordered_map<string, string> suspect_bcsnrid_chunks_path_by_sample_key ;

	unordered_map<string, string> suspect_read_fastq_path_by_sample_key ;

	unordered_map<string, string> swapped_in_read_ids_path_by_sample_key ;

	public:

		Workdir (Path, string, const unordered_map<string, Sample>&) ;

		unordered_set<string> get_sample_keys() { return this->sample_keys ; }

		unordered_set<string> get_star_reference_paths() { return this->star_reference_paths ; }

		string get_unfiltered_overlap_path() { return this->unfiltered_overlap_path ; }
		string get_filtered_overlap_path() { return this->filtered_overlap_path ; }

		string get_bcsnrid_path(string) ;
		string get_bcsnrid_chunks_path(string, string) ;
		string get_suspect_bcsnrid_path(string) ;
		string get_suspect_bcsnrid_chunks_path(string) ;
		string get_suspect_read_fastq_path(string) ;
		string get_suspect_read_fastq_path_list_string() ;
		string get_alignment_dir_path(string) ;
		string get_annotation_dir_path(string) ;
		string get_sam_path(string) ;
		string get_cug_label_path(string) ;
		string get_cug_label_chunks_path(string) ;
		string get_called_swaps_path(string) ;
		string get_swapped_in_read_ids_path(string) ;
} ;

#endif