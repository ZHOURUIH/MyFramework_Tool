#include "CodeCondition.h"
#include "CodeCondition_Server.h"
#include "CodeCondition_Client.h"

void CodeCondition::generate()
{
	print("正在生成组件代码");
	// Game
	const string registerCppPath = !cppGamePath.empty() ? cppGamePath + "/ConditionManager/" : "";
	const string registerCSPath = ClientHotFixPath + "/ConditionManager/";

	// 先读取表格描述
	CSVInfo csvInfo;
	parseCSV(ExcelPath + "Condition.csv", csvInfo.mHeader, csvInfo.mDataList);
	if (csvInfo.mDataList.size() == 0)
	{
		return;
	}
	int nameColumn = -1;
	FOR_VECTOR(csvInfo.mHeader.mColumnDataList)
	{
		if (csvInfo.mHeader.mColumnDataList[i]->mName == "VariableName")
		{
			nameColumn = i;
			break;
		}
	}
	if (nameColumn < 0)
	{
		return;
	}
	myVector<string> conditionList;
	FOR_VECTOR(csvInfo.mDataList)
	{
		conditionList.push_back(csvInfo.mDataList[i][nameColumn]);
	}
	if (!registerCppPath.empty())
	{
		CodeCondition_Server::generateCppConditionRegister(conditionList, registerCppPath);
	}
	CodeCondition_Client::generateCSharpConditionRegister(conditionList, registerCSPath);
	print("完成生成组件代码");
	print("");
}