#ifndef TASK_POOL_H
#define TASK_POOL_H

#include <vector>
#include <string>
#include <future>
#include <chrono>
#include <unordered_map>
#include <stack>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "sample.h"
#include "whitelist.h"
#include "workdir.h"

struct Task {
	std::function<void (Task)> func ;
	std::unordered_map<std::string, std::string> string_args; 
	Sample * sample_ptr ;
	Workdir * workdir_ptr ;
};

void run_tasks(int threads, std::stack<Task> task_stack) ;

#endif