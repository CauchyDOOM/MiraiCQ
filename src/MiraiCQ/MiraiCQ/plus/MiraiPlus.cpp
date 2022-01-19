#include "MiraiPlus.h"

#include <Windows.h>
#include <mutex>
#include <fstream>
#include <algorithm>

#include "../log/MiraiLog.h"
#include "../tool/PathTool.h"
#include "../tool/StrTool.h"
#include "../tool/TimeTool.h"
#include <jsoncpp/json.h>

using namespace std;
MiraiPlus::MiraiPlus()
{
}

MiraiPlus::~MiraiPlus()
{
}

bool MiraiPlus::load_plus(const std::string& dll_name, std::string & err_msg) 
{
	err_msg.clear();
	std::string bin_path = PathTool::get_exe_dir() + "";
	PathTool::create_dir(bin_path);
	//SetDllDirectoryA(bin_path.c_str()); 
	std::string cqp_path = bin_path + "CQP.dll";
	if (!PathTool::is_file_exist(cqp_path.c_str()))
	{
		err_msg = "CQP.dll����Ŀ¼��";
		return false;
	}
	if (GetModuleHandleA(cqp_path.c_str()) == NULL)
	{
		if (!LoadLibraryA(cqp_path.c_str()))
		{
			err_msg = "CQP.dll����ʧ��";
			return false;
		}
	}
	/* ����ͼƬĿ¼ */ 
	PathTool::create_dir(PathTool::get_exe_dir() + "data\\");
	PathTool::create_dir(PathTool::get_exe_dir() + "data\\image");

	std::string dll_path = PathTool::get_exe_dir() + "app\\" + dll_name + ".dll";
	std::string json_path = PathTool::get_exe_dir() + "app\\" + dll_name + ".json";
	if (!PathTool::is_file_exist(dll_path))
	{
		err_msg = "ģ��dll�ļ�������";
		return false;
	}
	if (!PathTool::is_file_exist(json_path))
	{
		err_msg = "ģ��json�ļ�������";
		return false;
	}
	ifstream json_fp;
	json_fp.open(json_path);
	if (!json_fp.is_open())
	{
		err_msg = "ģ��json�ļ���ʧ��";
		return false;
	}
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(json_fp, root))
	{
		err_msg = "ģ��json�ļ�����ʧ��";
		return false;
	}
	json_fp.close();
	std::shared_ptr<PlusDef> plus_def(new PlusDef);
	plus_def->module_hand = LoadLibraryA(dll_path.c_str());
	if (!plus_def->module_hand)
	{
		err_msg = "ģ��dll�ļ�����ʧ��";
		return false;
	}

	plus_def->filename = dll_name;

	Json::Value def_str = "";
	{
		auto name_json = root.get("name", def_str);
		if (name_json.isString() && name_json.asString() != "")
		{
			plus_def->name = name_json.asString();
		}
		else
		{
			err_msg = "ģ��json�ļ�����ʧ��,ȱ��name";
			return false;
		}
	}

	{
		auto version_json = root.get("version", def_str);
		if (version_json.isString() && version_json.asString() != "")
		{
			plus_def->version = version_json.asString();
		}
		else
		{
			err_msg = "ģ��json�ļ�����ʧ��,ȱ��version";
			return false;
		}
	}

	{
		auto author_json = root.get("author", def_str);
		if (author_json.isString() && author_json.asString() != "")
		{
			plus_def->author = author_json.asString();
		}
		else
		{
			err_msg = "ģ��json�ļ�����ʧ��,ȱ��author";
			return false;
		}
	}

	{
		auto description_json = root.get("description", def_str);
		if (description_json.isString())
		{
			plus_def->description = description_json.asString();
		}
		else
		{
			err_msg = "ģ��json�ļ�����ʧ��,ȱ��description";
			return false;
		}
	}

	auto event_json_arr = root.get("event","");
	if (event_json_arr.isArray())
	{
		for (auto& node : event_json_arr)
		{
			auto event = make_shared<PlusDef::Event>();
			{
				auto fun_name_json = node.get("function", def_str);
				if (fun_name_json.isString() && fun_name_json.asString() != "")
				{
					event->fun_name = fun_name_json.asString();
				}
				else
				{
					err_msg = "ģ��json�ļ�����ʧ��,ȱ��function";
					return false;
				}
			}
			{
				auto type_json = node.get("type", def_str);
				if (type_json.isInt())
				{
					event->type = type_json.asInt();
				}
				else
				{
					err_msg = "ģ��json�ļ�����ʧ��,event����`"+ event->fun_name +"`ȱ��type";
					return false;
				}
			}
			{
				auto priority_json = node.get("priority", def_str);
				if (priority_json.isInt())
				{
					event->priority = priority_json.asInt();
				}
				else
				{
					event->priority = 30000;
					std::string msg = "����`"+ event->fun_name +"`ȱ��priority,ʹ��Ĭ��ֵ30000";
					MiraiLog::get_instance()->add_debug_log("load_plus", msg);
				}
			}
			
			{
				void* dll_func = GetProcAddress((HMODULE)plus_def->module_hand, event->fun_name.c_str());
				if (dll_func == NULL)
				{
					err_msg = "����`"+ event->fun_name +"`����ʧ��";
					return false;
				}
				event->function = dll_func;
			}
			plus_def->event_vec.push_back(event);
		}
	}
	auto meun_json_arr = root.get("menu", "");
	if (meun_json_arr.isArray())
	{
		for (auto& node : meun_json_arr)
		{
			auto menu = make_shared<PlusDef::Menu>();
			{
				auto fun_name_json = node.get("function", def_str);
				if (fun_name_json.isString() && fun_name_json.asString() != "")
				{
					menu->fun_name = fun_name_json.asString();
				}
				else
				{
					err_msg = "ģ��json�ļ�����ʧ��,ȱ��function";
					return false;
				}
				auto name_json = node.get("name", def_str);
				if (name_json.isString() && name_json.asString() != "")
				{
					menu->name = name_json.asString();
				}
				else
				{
					err_msg = "ģ��json�ļ�����ʧ��,ȱ��name";
					return false;
				}
			}
			{
				void* dll_func = GetProcAddress((HMODULE)plus_def->module_hand, menu->fun_name.c_str());
				if (dll_func == NULL)
				{
					err_msg = "����`" + menu->fun_name + "`����ʧ��";
					return false;
				}
				menu->function = dll_func;
			}
			plus_def->menu_vec.push_back(menu);
		}
	}

	plus_def->ac = StrTool::gen_ac();
	{
		typedef __int32(__stdcall* fun_ptr_type_1)(__int32);
		fun_ptr_type_1 fun_ptr1 = (fun_ptr_type_1)GetProcAddress((HMODULE)plus_def->module_hand, "Initialize");
		if (!fun_ptr1)
		{
			err_msg = "����Initialize��ȡʧ��";
			return false;
		}
		fun_ptr1(plus_def->ac);
		MiraiLog::get_instance()->add_debug_log("PLUS", plus_def->name + "��auth_code:" + std::to_string(plus_def->ac));
	}

	{
		unique_lock<shared_mutex> lock(mx_plus_map);
		plus_map[plus_def->ac] = plus_def;
	}

	return true;
}

bool MiraiPlus::enable_plus(int ac, std::string & err_msg) 
{
	err_msg.clear();
	auto plus = get_plus(ac);
	if (!plus)
	{
		err_msg = "ac������";
		return false;
	}

	if (plus->is_enable) /* ����Ѿ������� */
	{
		return true;
	}
	else
	{
		plus->is_enable = true;
	}
	
	/* �����һ������ */
	if (plus->is_first_enable)
	{
		plus->is_first_enable = false;
		auto fun =  plus->get_event_fun(1001); /* ������� */
		if (fun)
		{ 
			/* shared_lock<shared_mutex> lock(plus->mx); */
			typedef __int32(__stdcall* fun_ptr_type_1)();
			int ret = ((fun_ptr_type_1)fun->function)();
			if (ret != 0)
			{
				err_msg = "���`"+ plus->name+"`�ܾ�����";
				plus->is_enable = false;
				return false;
			}
		}
			
	}

	auto fun = plus->get_event_fun(1003); /* ������� */
	if (fun)
	{
		/* shared_lock<shared_mutex> lock(plus->mx); */
		typedef __int32(__stdcall* fun_ptr_type_1)();
		int ret = ((fun_ptr_type_1)fun->function)();
		if (ret != 0)
		{
			err_msg = "���`" + plus->name + "`�ܾ�����";
			plus->is_enable = false;
			return false;
		}
	}
	return true;
}

bool MiraiPlus::disable_plus(int ac, std::string & err_msg) 
{
	err_msg.clear();
	auto plus = get_plus(ac);
	if (!plus)
	{
		err_msg = "ac������";
		return false;
	}
	if (!plus->is_enable) /* ���û������ */
	{
		return true;
	}
	else 
	{
		plus->is_enable = false;
	}
	auto fun = plus->get_event_fun(1004); /* ������� */
	if (fun)
	{
		/* shared_lock<shared_mutex> lock(plus->mx); */
		typedef __int32(__stdcall* fun_ptr_type_1)();
		int ret = ((fun_ptr_type_1)fun->function)();
		if (ret != 0)
		{
			plus->is_enable = true;
			err_msg = "���`" + plus->name + "`�ܾ�������";
			return false;
		}
	}
	return true;
}

bool MiraiPlus::is_enable(int ac) 
{
	auto plus = get_plus(ac);
	if (!plus)
	{
		return false;
	}
	return plus->is_enable;
}

bool MiraiPlus::del_plus(int ac) 
{
	auto plus = get_plus(ac);
	if (!plus)
	{
		/* ac�����ڣ�˵���Ѿ���ж�� */
		return true;
	}
	if (!plus->is_first_enable) /* ��ʼ���Ĳ�����ܵ���ж�غ��� */
	{
		auto fun =plus->get_event_fun(1002); /* ����˳��¼� */
		if (fun)
		{
			/* shared_lock<shared_mutex> lock(plus->mx); */
			typedef __int32(__stdcall* fun_ptr_type_1)();
			int ret = ((fun_ptr_type_1)fun->function)();
			if (ret != 0)
			{
				plus->is_enable = true;
				//err_msg = "���`" + plus->name + "`�ܾ���ж��";
				return false;
			}
		}
	}
	return true;
}

std::shared_ptr<MiraiPlus::PlusDef> MiraiPlus::get_plus(int ac) 
{
	shared_lock<shared_mutex> lock(mx_plus_map);
	auto iter = plus_map.find(ac);
	if (iter == plus_map.end())
	{
		/* ac�����ڣ�˵���Ѿ���ж�� */
		return nullptr;
	}
	return iter->second;
	
}

std::map<int, std::shared_ptr<MiraiPlus::PlusDef>> MiraiPlus::get_all_plus() 
{
	shared_lock<shared_mutex> lock(mx_plus_map);
	return plus_map;
}

std::vector<std::pair<int,std::weak_ptr<MiraiPlus::PlusDef>>> MiraiPlus::get_all_ac() 
{
	shared_lock<shared_mutex> lock(mx_plus_map);
	std::vector<std::pair<int, std::weak_ptr<MiraiPlus::PlusDef>>> ret_vec;
	for (auto& it : plus_map)
	{
		ret_vec.push_back({ it.first,it.second });
	}
	return ret_vec;
}

MiraiPlus* MiraiPlus::get_instance() 
{
	static MiraiPlus mirai_plus;
	return &mirai_plus;
}

MiraiPlus::PlusDef::~PlusDef()
{

}

const std::shared_ptr<const MiraiPlus::PlusDef::Event> MiraiPlus::PlusDef::get_event_fun(int type) 
{
	for (const auto& fun : event_vec)
	{
		if (fun->type == type)
		{
			return fun;
		}
	}
	return nullptr;
}

std::vector<std::shared_ptr<const MiraiPlus::PlusDef::Menu>> MiraiPlus::PlusDef::get_menu_vec() 
{
	return menu_vec;
}

std::string MiraiPlus::PlusDef::get_name() 
{
	return this->name;
}

std::string MiraiPlus::PlusDef::get_filename() 
{
	return this->filename;
}

