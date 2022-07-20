#include "ThreadPool.h"

using namespace v4d::processing;

ThreadPoolBase::~ThreadPoolBase() {
	Shutdown();
}
