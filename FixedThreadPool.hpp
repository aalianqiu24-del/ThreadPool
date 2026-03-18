#pragma once

#include"SyncQueue1.hpp"
#include<functional>
#include<memory>
#include<thread>
#include<atomic>
using namespace std;
class FixedThreadPool//固定式线程池
{
public:
	using Task = std::function<void(void)>;
private:
	SyncQueue<Task>m_queue;//任务
	std::list<std::unique_ptr<std::thread>>m_threadgroup;//线程
	std::atomic_bool m_running;//true:当前线程池运行
	std::once_flag m_flag;
	void Start(int numthreads)
	{
		m_running = true;
		for (int i = 0; i < numthreads; ++i)//添加线程
		{
			m_threadgroup.push_back(std::make_unique<std::thread>(
				&FixedThreadPool::RunInThread, this));
		}
	}
	void RunInThread()//入口函数
	{
		while (m_running)
		{
			Task task;
			m_queue.Take(task);
			if (task && m_running)
			{
				task();
			}
		}
	}
	void StopThreadGroup()
	{
		//m_queue.Stop();
		m_queue.WaitStop();
		m_running = false;
		for (auto& tha : m_threadgroup)
		{
			if (tha && tha->joinable())
			{
				tha->join();//等待线程结束，释放资源
			}
		}
		m_threadgroup.clear();
	}
public:
	FixedThreadPool(size_t qusize = 100, int numthreads = 8)
		:m_queue(qusize), m_running(false)
	{
		Start(numthreads);
	}
	~FixedThreadPool()
	{
		Stop();
	}
	void Stop()
	{
		std::call_once(m_flag, [this]()->void {StopThreadGroup(); });
	}
	void AddTask(const Task& task)
	{
		m_queue.Put(task);
	}
	void AddTask(Task&& task)
	{
		m_queue.Put(std::forward<Task>(task));
	}
};