#include "gzchunksort.h"

using namespace std ;

template <class K>
GzChunkSortWriter<K>::GzChunkSortWriter(const string& out_dir_path_str) {

	// initialize the output dir
	this->out_dir_path = Path(out_dir_path_str) ;
	if (this->out_dir_path.exists()) throw runtime_error("output dir path exists") ;
	if (!this->out_dir_path.get_parent_path().is_dir()) throw runtime_error("output dir path does not have an existant parent path") ;

	this->out_dir_path.make_dir() ;
}

template <class K>
void GzChunkSortWriter<K>::write_lines() {

	// count this line
	this->out_file_count++ ;

	// address dir limit
	if (this->out_file_count > this->dir_limit) {
		this->out_file_count = 1 ;
		this->out_dir_count++ ;
	}

	// make the output file path
	Path new_out_file_path = this->out_dir_path.join("dir" + to_string(this->out_dir_count)) ;
	if (!new_out_file_path.exists()) { new_out_file_path.make_dir() ; }

	new_out_file_path = new_out_file_path.join("file" + to_string(this->out_file_count)) ;
	this->file_vect.push_back(new_out_file_path.to_string()) ;

	// check the new file
	if (new_out_file_path.exists()) throw runtime_error("new output file already exists") ;

	// write out the lines
	Gzout gzout = Gzout(new_out_file_path.to_string()) ;
	for (map<string, vector<string>>::iterator it = this->sorted_lines.begin(); it != this->sorted_lines.end(); ++it) {

		for (string sline : it->second) { gzout.write_line(sline) ; }
	}
    this->sorted_lines.clear() ; 
    gzout.flush_close() ;

    this->line_count = 0 ;
}

template <class K>
void GzChunkSortWriter<K>::write_line(K key, string line) {

	if (this->closed) throw runtime_error("attempted to write to a closed GzChunkSortWriter") ;

	if (this->sorted_lines.find(key) != this->sorted_lines.end()) {

		this->sorted_lines.at(key).push_back(line) ;

	} else {

		vector<string> lvect ; lvect.push_back(line) ;
		this->sorted_lines.insert(pair<string, vector<string>>(key, lvect)) ;
	}

	this->line_count++ ;
	if (this->line_count >= this->chunk_size) this->write_lines() ;
}

template <class K>
void GzChunkSortWriter<K>::flush_close() {

	if (this->sorted_lines.size() > 0) this->write_lines() ; 
	this->closed = true ;
}

template <class K>
vector<string> GzChunkSortWriter<K>::get_files () {

	if(!this->closed) throw runtime_error("attempted to get files before GzChunkSortWriter is closed") ;
	return this->file_vect ;
}
