#include "../include/CMThreadPool.hpp"

CMThreadPool::CMThreadPool(const uint8_t num)
{
	isStop = false;
	for (int i = 0; i <= num; i++) {
		threads.emplace_back([this] {
			while (1) {
				std::function<void()> func;
				{
					std::unique_lock<std::mutex> lock(mtx);
					m_cv.wait(lock, [this] {
						return !task.empty() || isStop;
						});
					if (isStop && task.empty())return;

					func = task.front();
					task.pop();
				}
				func();
			}
			});
	}
}

void CMThreadPool::waitAll()
{
	for (auto& a : threads) {
		a.join();
	}
}

CMThreadPool::~CMThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(mtx);
		isStop = true;
	}
	m_cv.notify_all();
	for (auto& a : threads) {
		a.join();
	}
}
