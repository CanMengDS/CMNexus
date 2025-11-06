#pragma once
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>

class CMThreadPool {
public:
	CMThreadPool(const uint8_t num);
	template<class T>
	void enterTask(T func);
	void waitAll();
	~CMThreadPool();
private:
	std::vector<std::thread> threads;
	std::queue<std::function<void()>> task;
	std::mutex mtx;
	std::condition_variable m_cv;
	bool isStop;
};

template<class T>
inline void CMThreadPool::enterTask(T func)
{
	{
		std::unique_lock<std::mutex> lock(mtx);
		task.emplace(func);
	}
	m_cv.notify_one();
}
