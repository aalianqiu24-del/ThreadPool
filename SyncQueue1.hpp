#pragma once
#include<list>
#include<mutex>
#include<condition_variable>

template<class Task>
class SyncQueue
{
private:
	std::list<Task>m_queue;
	std::mutex m_mutex;//互斥锁
	std::condition_variable m_notEmpty;//消费者    可以消费
	std::condition_variable m_notFull;//生产者     可以生产
	std::condition_variable m_waitStop;
	size_t m_maxSize;               //任务上限
	bool m_Stop;                      //false 不停止  true 停止  

	bool IsFull()const { return m_queue.size() >= m_maxSize; }
	bool IsEmpty()const { return m_queue.empty(); }

	template<class F>
	void Add(F&& f)//生产者
	{
		std::unique_lock<std::mutex>locker(m_mutex);//当 locker 的作用域结束时（即函数返回或异常退出时），std::unique_lock 的析构函数会自动释放锁。
		while (!m_Stop && IsFull())
		{
			m_notFull.wait(locker);//1释放锁 2进入条件变量等待 -----唤醒-----3进入互斥锁等待 4就绪
		}
		if (m_Stop)return;
		m_queue.push_back(std::forward<F>(f));
		m_notEmpty.notify_all();
	}
public:
	SyncQueue(size_t maxsize):m_maxSize(maxsize),m_Stop(false){}
	~SyncQueue()
	{
	}
	void Stop() // 强制停止
	{
		{
			std::unique_lock<std::mutex>locker(m_mutex);
			m_Stop = true;
		}
		m_notEmpty.notify_all();
		m_notFull.notify_all();
	}
	void WaitStop()
	{
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!IsEmpty())
		{
			//m_waitStop.wait(locker);//1234
			m_waitStop.wait_for(locker, std::chrono::milliseconds(1));
		}
		m_Stop = true;
		m_notEmpty.notify_all();
		m_notFull.notify_all();
	}
	void Put(Task&& task)//生产者
	{
		Add(std::forward<Task>(task));//1 2 3 4 
	}
	void Put(const Task& task)
	{
		Add(task);
	}
	void Take(Task& task)//消费者
	{
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!m_Stop && IsEmpty())
		{
			m_notEmpty.wait(locker);
		}
		if (m_Stop)return;
		task = m_queue.front();
		m_queue.pop_front();
		m_notFull.notify_all();
	}
	void Take(std::list<Task>& tasklist)//消费者
	{
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!m_Stop && IsEmpty())
		{
			m_notEmpty.wait(locker);
		}
		if (m_Stop)return;
		tasklist = std::move(m_queue);
		m_notFull.notify_all();
	}

	bool Empty()const
	{
		std::unique_lock<std::mutex>locker(m_mutex);
		return m_queue.empty();
	}
	bool Full()const
	{
		std::unique_lock<std::mutex>locker(m_mutex);
		return m_queue.size() >= m_maxSize;
	}
};
