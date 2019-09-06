#include "task_pool.h"

void run_tasks(int threads, std::stack<Task> task_stack) {

	std::vector<std::future<void>> children; 
	std::chrono::milliseconds span (500);

	bool running = true ;
	while (running) {

		// check children
		std::stack<int> to_remove ;
		for (int i = 0; i < children.size(); i++) {

			if (children[i].wait_for(span) == std::future_status::ready) {

				to_remove.push(i) ; children[i].get() ;
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

			Task task  = task_stack.top() ; task_stack.pop() ;

			children.push_back(std::async (task.func, task)) ;
		}

		if (children.empty() && task_stack.empty()) { running = false ; }
	}
}
