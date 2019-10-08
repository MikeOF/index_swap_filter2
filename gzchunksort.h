#ifndef GZCHUNKSORT_H
#define GZCHUNKSORT_H

#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <map>

#include "gzfiles.h"
#include "path.h"

using namespace std ;

template <class K>
class GzChunkSortWriter {

	const int chunk_size = 2000000 ;
	const int dir_limit = 3 ;

	Path out_dir_path ;
	map<K, vector<string>> sorted_lines ;
	vector<string> file_vect ;
	int line_count = 0 ;
	int out_file_count = 0 ;
	int out_dir_count = 1 ;
	bool closed = false ;

	void write_lines() ;

	public:

		GzChunkSortWriter(const string&) ;

		void write_line(K, string) ;
		void flush_close() ;
		vector<string> get_files() ;
};

template <class K>
int collect_sorted_chunks(const string&, const vector<string>, const function<K (string)>) ;

#endif
