#ifndef FILES_H
#define FILES_H

#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <errno.h>

bool is_file(std::string) ;
bool is_dir(std::string) ;
bool open_gz_file(std::string) ;



#endif