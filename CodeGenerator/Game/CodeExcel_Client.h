#pragma once

#include "CodeExcel.h"

class CodeExcel_Client : public CodeExcel
{
public:
	static void generate(const myVector<CSVInfo>& infoList);
	//C#这里不再使用SQLite,而是将SQLite转换为自定义的数据来读取,也跟Excel转换以后的数据一样
	static void generateCSharpExcelDataFile(const CSVInfo& sqliteInfo, const string& dataFileHotFixPath);
	static void generateCSharpExcelTableFile(const CSVInfo& sqliteInfo, const string& tableFileHotFixPath);
	static void generateCSharpExcelRegisteFileFile(const myVector<CSVInfo>& sqliteInfo, const string& fileHotFixPath);
	static void generateCSharpExcelDeclare(const myVector<CSVInfo>& sqliteInfo, const string& fileHotFixPath);
	static void generateCSharpGlobalConfig(const CSVInfo& globalConfig, const string& tableFilePath);
	static void generateCSharpBuff(const CSVInfo& config);
};