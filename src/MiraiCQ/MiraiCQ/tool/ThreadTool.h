#pragma once
#include <functional>
#include <list>
#include <atomic>
#include <shared_mutex>

class ThreadTool
{
public:
	/*
	* �������ύһ������
	* ����`task`��Ҫ�ύ������
	* ����ֵ���Ƿ��ύ�ɹ�
	*/
	bool submit(const std::function<void()> & task);
	/*
	* ����������̳߳�(����ģʽ)
	* ����ֵ��ThreadTool�ǿ�ָ��
	*/
	static ThreadTool* get_instance();
	/*
	* ��������õ�ǰ�̳߳��߳�����
	* ����ֵ����ǰ�̳߳��߳�����
	*/
	int get_cur_thread_nums() const;
	/*
	* ��������õ�ǰ�̳߳ؿ����߳�����
	* ����ֵ����ǰ�̳߳ؿ����߳�����
	*/
	int get_unused_thread_nums() const;
	/*
	* ��������õ�ǰ���������������
	* ����ֵ����ǰ���������������
	*/
	size_t get_task_list_nums();
private:
	ThreadTool();
	/* ����һ���߳� */
	void add_new_thread();
	/* ��¼��ǰ���ڵ��߳����� */
	std::atomic_int cur_thread_nums = 0;
	/* ��¼��ǰ�����߳� */
	std::atomic_int unused_thread_nums = 0;
	/* ������� */
	std::shared_mutex mx_task_list;
	std::list<std::function<void()>> task_list;
};

