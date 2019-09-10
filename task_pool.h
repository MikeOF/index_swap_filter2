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

template <class T>
struct Task {
	std::function<T (Task<T>)> func ;
	std::unordered_map<std::string, std::string> string_args; 
	Sample * sample_ptr ;
	Workdir * workdir_ptr ;
};

template <class T>
std::stack<T> run_tasks(int threads, std::stack<Task<T>> task_stack) {

	std::vector<std::future<T>> children; 
	std::stack<T> return_stack ;
	std::chrono::milliseconds span (500);

	bool running = true ;
	while (running) {

		// check children
		std::stack<int> to_remove ;
		for (int i = 0; i < children.size(); i++) {

			if (children[i].wait_for(span) == std::future_status::ready) {

				to_remove.push(i) ; return_stack.push(children[i].get()) ;
			} 
		}

		// remove completed children
		while (!to_remove.empty()) { 

			children.erase(children.begin() + to_remove.top()) ; to_remove.pop() ; 
		}

		// start threads
		int children_to_start = threads - children.size() ;
		if (children_to_start > task_stack.size()) { children_to_start = task_stack.size() ; }

		for (int i = 0; i < children_to_start; i++) {

			std::cout << "hello" << std::endl ;

			Task<T> task  = task_stack.top() ; task_stack.pop() ;

			children.push_back(std::async (task.func, task)) ;
		}

		if (children.empty() && task_stack.empty()) { running = false ; }
	}

	return return_stack ;
}

#endif