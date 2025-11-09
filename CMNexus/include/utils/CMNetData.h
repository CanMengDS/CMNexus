#pragma once
#include<iostream>
#include<map>
#include<functional>


class DataHeader;
/// <summary>
/// 注意:该类仅用于CMDataHeader，对于自定义数据头/协议没有作用！
/// </summary>
/// 
class CMNetData {
public:
	CMNetData() = default;
	~CMNetData() = default;
	void AddCMDataHeaderReturnDataCaseAndSolveFunction(int return_data_case, std::function<void(char*, int)> solve_function);
	bool CheckDataCaseAndRunSolveFunction(DataHeader& header, char* untreated_buffer, int buffer_length);


	const std::map<int, std::function<void(char*, int)>>& GetReturnDataCaseAndFunctionMap();
private:
	std::map<int, std::function<void(char*, int)>> return_data_case_and_function;
};