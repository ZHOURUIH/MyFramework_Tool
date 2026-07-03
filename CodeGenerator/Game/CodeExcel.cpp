#include "CodeExcel.h"
#include "CodeExcel_Server.h"
#include "CodeExcel_Client.h"
#include "CodeSQLite.h"

void CodeExcel::generate()
{
	if (ExcelPath.empty())
	{
		return;
	}
	print("正在生成Excel");

	// 先读取表格描述
	myVector<CSVInfo> infoList;
	myVector<string> csvFiles;
	findFiles(ExcelPath, csvFiles, ".csv");
	FOR_VECTOR(csvFiles)
	{
		infoList.push_back(CSVInfo());
		parseCSV(csvFiles[i], infoList[i].mHeader, infoList[i].mDataList);
	}
	if (!cppGamePath.empty())
	{
		CodeExcel_Server::generate(infoList);
	}
	if (!ClientHotFixPath.empty())
	{
		CodeExcel_Client::generate(infoList);
	}
	print("完成生成Excel");
	print("");
}

string CodeExcel::paramNameToFunctionName(const string& paramName)
{
	myVector<string> elements;
	split(paramName.c_str(), "_", elements);
	string functionName;
	FOR_VECTOR(elements)
	{
		string temp = toLower(elements[i]);
		temp[0] = toUpper(temp[0]);
		functionName += temp;
	}
	return functionName;
}