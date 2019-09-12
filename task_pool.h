#ifndef TASK_POOL_H
#define TASK_POOL_H

#include <vector>
#include <string>
#include <thread>
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

template <class R, class A>
struct Task {
	std::function<R (Task<R, A>)> func ;
	A args; 
} ;

template <class R, class A>
std::stack<R> run_tasks(int threads, std::stack<Task<R, A>> task_stack) {

	std::vector<Task<R, A>> tasks ;
	std::vector<std::future<R>> children; 
	std::stack<R> return_stack ;
	std::chrono::seconds wait_span (1) ;
	std::chrono::seconds sleep_span (5) ;

	// transfer tasks to a vector
	while(!task_stack.empty()) {
		tasks.emplace_back(task_stack.top()) ; task_stack.pop() ;
	}

	int task_idx = 0 ;
	bool running = true ;
	while (running) {

		// check children
		std::stack<int> to_remove ;
		for (int i = 0; i < children.size(); i++) {

			if (children[i].wait_for(wait_span) == std::future_status::ready) {

				std::cout << "child " + std::to_string(i) + " is ready\n" ;

				to_remove.push(i) ; return_stack.push(children[i].get()) ;

			} else {

				std::cout << "child " + std::to_string(i) + " is not ready\n" ;
			}
		}

		// remove completed children
		while (!to_remove.empty()) { 

			children.erase(children.begin() + to_remove.top()) ; to_remove.pop() ; 
		}

		// determine how many tasks to start
		int children_to_start = threads - children.size() ;
		if (children_to_start > tasks.size() - task_idx) { children_to_start = tasks.size() - task_idx ; }

		// start tasks
		for (int i = 0; i < children_to_start; i++) {

			children.emplace_back( std::async (std::launch::async, tasks[task_idx].func, tasks[task_idx] ) ) ;

			task_idx++ ;
		}

		// check to see if we are done, sleep if we aren't
		if (children.empty() && task_idx == tasks.size()) { running = false ; }
		else { std::this_thread::sleep_for (sleep_span) ; }
	}

	return return_stack ;
}

#endif