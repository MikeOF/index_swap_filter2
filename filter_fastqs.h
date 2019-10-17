#ifndef FILTER_FASTQS_H
#define FILTER_FASTQS_H

#include <string>
#include <iostream>
#include <unordered_set>
#include <stdexcept>

#include "path.h"
#include "gzfile.h"

using namespace std ;

void filter_fastqs(int, string, unordered_set<string>) ;


#endif