#pragma once
#include <mutex>
#include "Consts.h"
#include <queue>

class ComputationTaskPool
{
private:
	mutex __comp_pool_locker;
	queue<ComputeTask> __tasks;
public:
	ComputeTask get();
	void add(ComputeTask t);
};