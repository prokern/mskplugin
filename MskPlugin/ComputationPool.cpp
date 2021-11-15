#include "pch.h"
#include "ComputationPool.h"
#include "Global.h"

ComputeTask ComputationTaskPool::get()
{
	lock_guard<mutex> lock(__comp_pool_locker);
	if (__tasks.empty()) {
		return ComputeTask();
	}
	ComputeTask cs = __tasks.front();
	__tasks.pop();
	return cs;
}

void ComputationTaskPool::add(ComputeTask t)
{
	lock_guard<mutex> lock(__comp_pool_locker);
	__tasks.push(t);
}
