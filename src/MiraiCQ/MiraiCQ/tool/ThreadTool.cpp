#include "ThreadTool.h"
#include "TimeTool.h"
#include <thread>
#include "../log/MiraiLog.h"
#include <spdlog/fmt/fmt.h>

void ThreadTool::submit(const std::function<void()> & task)
{
	{
		// ����������������
		std::lock_guard<std::mutex> lock(mx_task_list);
		task_list.push_back(task);
	}
	// û�п����̣߳�������һ���̣߳���֤������˳������
	if (unused_thread_nums == 0)
	{
		add_new_thread();
	}
	
}

static bool has_task(const std::list<std::function<void()>>& task_list, std::mutex& mx_task_list)
{
	std::lock_guard<std::mutex> lock(mx_task_list);
	return ((task_list.size() > 0) ? true : false);
}

static size_t get_task_nums(const std::list<std::function<void()>>& task_list, std::mutex& mx_task_list)
{
	std::lock_guard<std::mutex> lock(mx_task_list);
	return task_list.size();
}

ThreadTool::ThreadTool()
{
	is_stop = false;
	// ����һ���ػ��߳�
	++cur_thread_nums;
	//MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("cur_thread_nums:{}", cur_thread_nums));
	std::thread([&]() {
		int i = 0;
		while (true) {
			++i;
			if (i == 300)
			{
				MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("cur_thread_nums:{}", cur_thread_nums));
				MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("unused_thread_nums:{}", unused_thread_nums));
				MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("task_nums:{}", get_task_nums(task_list, mx_task_list)));
				i = 0;
			}
			TimeTool::sleep(100);
			// ���û��δʹ�õ��߳�,����������������һ���߳�
			if (unused_thread_nums == 0 && has_task(task_list,mx_task_list))
			{
				add_new_thread();
			}
			// ����̳߳��Ѿ�ֹͣ���������û���������˳��ػ��߳�
			std::lock_guard<std::mutex> lock(mx_task_list);
			if (is_stop && !has_task(task_list, mx_task_list))
				break;
		}
		// �߳��˳���
		--cur_thread_nums;
		//MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("cur_thread_nums:{}", cur_thread_nums));
	}).detach();
}

ThreadTool::~ThreadTool()
{
	// ֹͣ�̳߳�
	is_stop = false;
	// �ȴ��߳�ȫ��ֹͣ
	while (cur_thread_nums)
		TimeTool::sleep(10);
}

void ThreadTool::add_new_thread()
{
	++cur_thread_nums;
	//MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("cur_thread_nums:{}", cur_thread_nums));
	std::thread([&]() {
		// ���ڱ���Ƿ����
		bool is_unused = false;
		while (true)
		{
			std::function<void()> task = nullptr;
			{
				// �����������һ������
				std::lock_guard<std::mutex> lock(mx_task_list);
				if (task_list.size() > 0) {
					task = (*task_list.begin());
					task_list.pop_front();
				}
			}
			if (task) {
				// ����õ��ˣ���ִ��
				if (is_unused)
				{
					// ɾ�����б��
					is_unused = false;
					--unused_thread_nums;
					//MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("unused_thread_nums:{}", unused_thread_nums));
				}
				task();
			}
			else {
				// ���û�õ���˵���������Ϊ�գ�
				if (is_unused){
					// ����Ѿ������Ϊ���У�˵������û�õ�task�����˳�
					break;
				}
				//����̳߳�ֹͣ�����������Ϊ�գ����˳��߳�
				if (is_stop && !has_task(task_list, mx_task_list)) {
						break;
				}
				TimeTool::sleep(100);
				// �����̱߳��Ϊ����,Ȼ�����ѭ��
				++unused_thread_nums;
				//MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("unused_thread_nums:{}", unused_thread_nums));
				is_unused = true;
			}

		}
		// ɾ�����б��
		if (is_unused) {
			--unused_thread_nums;
			//MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("unused_thread_nums:{}", unused_thread_nums));
		}
		// �߳��˳���
		--cur_thread_nums;
		//MiraiLog::get_instance()->add_debug_log("ThreadTool", fmt::format("cur_thread_nums:{}", cur_thread_nums));
	}).detach();
}
