#pragma once
#include <functional>
#include <list>
#include <atomic>
#include <mutex>

class ThreadTool
{
public:
	/*
	* �������ύһ������
	* ����`task`��Ҫ�ύ������
	*/
	void submit(const std::function<void()> & task);
	ThreadTool();
	~ThreadTool();
private:
	void add_new_thread();
	std::atomic_bool is_stop = false;
	/* ��¼��ǰ���ڵ��߳����� */
	std::atomic_int cur_thread_nums = 0;
	/* ��¼��ǰ�����߳� */
	std::atomic_int unused_thread_nums = 0;
	std::mutex mx_task_list;
	std::list<std::function<void()>> task_list;
};

