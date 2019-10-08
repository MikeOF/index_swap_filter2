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

template <class K>
int collect_sorted_chunks(const string& out_file_path_str, const vector<string> file_vect, const function<K (string)> key_getter) {

	// check args
	Path out_file_path = Path(out_file_path_str) ;
	if (out_file_path.exists()) throw runtime_error("output file path exists already") ;
	if (!out_file_path.get_parent_path().is_dir()) throw runtime_error("output file path does not have a directory as a parent") ;

	if (file_vect.empty()) throw runtime_error("passed an empty file vector") ;

	// handle the case of one chunk
	if (file_vect.size() == 1) {
		Path(file_vect.at(0)).rename(out_file_path_str) ;
		return -1 ;
	}

	// initialize file streams
	Gzout gzout = Gzout(out_file_path.to_string()) ;

	vector<Gzin *> gzin_vect ;
    vector<string> lines ;
    vector<K> keys ;
	for (string file : file_vect) { 

		gzin_vect.push_back(new Gzin(file)) ; 
		if (!gzin_vect.back()->has_next_line()) throw runtime_error("an empty chunk file was given") ;

		lines.push_back(gzin_vect.back()->read_line()) ;
		keys.push_back(key_getter(lines.back())) ;
	}

    // cylce through the input lines and write them in key order
    int lines_written = 0 ;
    while(gzin_vect.size() > 1) {

        // get the low key
        string low_key = keys.at(0) ;
        for (string key : keys) { if (key < low_key) low_key = string(key) ; }

        // write out LTE lines
        stack<Gzin *> to_evict ;
        for (int i = 0; i < keys.size(); i++) {

            while (keys.at(i) <= low_key) {

                gzout.write_line(lines.at(i)) ; lines_written++ ;

                // either get a new line for this gzins or evict it
                if (gzin_vect.at(i)->has_next_line()) {

                    lines.at(i) = gzin_vect.at(i)->read_line() ;
                    keys.at(i) = key_getter(lines.at(i)) ;
                
                } else {

                	to_evict.push(gzin_vect.at(i)) ;
                    break ;
                }
            }
        }

		// evict finished gzins
        while (!to_evict.empty()) {

        	for (int i = 0; i < gzin_vect.size(); i++) {

        		if (gzin_vect.at(i) == to_evict.top()) {

					gzin_vect.erase(gzin_vect.begin() + i) ;
					lines.erase(lines.begin() + i);
					keys.erase(keys.begin() + i);

					delete to_evict.top() ;

					break ;
        		}
        	}
        	to_evict.pop() ;
        }
    }

    // print out any remaining lines
    if (gzin_vect.size() == 1) {

	    gzout.write_line(lines.at(0)) ; lines_written++ ;

	    while (gzin_vect.at(0)->has_next_line()) { 
	    	gzout.write_line(gzin_vect.at(0)->read_line()) ; lines_written++ ;
	    }
	    delete gzin_vect.at(0) ;
	}

    // fluse and close the output file
    gzout.flush_close() ;

    return lines_written ;
}
