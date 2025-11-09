#include "../include/utils/CMNetData.h"
#include "../include/CMNetDefs.h"

void CMNetData::AddCMDataHeaderReturnDataCaseAndSolveFunction(int return_data_case, std::function<void(char*, int)> solve_function)
{
	return_data_case_and_function.insert(std::pair<int, std::function<void(char*, int)>>(return_data_case, solve_function));
}

bool CMNetData::CheckDataCaseAndRunSolveFunction(DataHeader& header, char* untreated_buffer, int buffer_length)
{
	std::map<int, std::function<void(char*, int)>>::const_iterator it = GetReturnDataCaseAndFunctionMap().find(header.PresentConduct);
	if (it != GetReturnDataCaseAndFunctionMap().end()) {
		std::function<void(char*, int)> function = (*it).second;
		function(untreated_buffer, buffer_length);
	}
	return true;
}

const std::map<int, std::function<void(char*, int)>>& CMNetData::GetReturnDataCaseAndFunctionMap()
{
	return return_data_case_and_function;
}
