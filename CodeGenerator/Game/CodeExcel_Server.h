#pragma once

#include "CodeExcel.h"

class CodeExcel_Server : public CodeExcel
{
public:
	static void generate(const myVector<CSVInfo>& infoList);
	static void generateCppExcelDataFile(const CSVInfo& sqliteInfo, const string& dataFilePath);
	static void generateCppExcelTableFile(const CSVInfo& sqliteInfo, const string& tableFilePath);
	static void generateCppExcelRegisteFile(const myVector<string>& tableFileList, const string& filePath);
	static void generateCppExcelInstanceDeclare(const myVector<string>& tableFileList, const string& gameBaseHeaderFileName, const string& exprtMacro);
	static void generateCppExcelInstanceDefine(const myVector<string>& tableFileList, const string& gameBaseCppFileName);
	static void generateCppExcelSTLPoolRegister(const myVector<string>& tableFileList, const string& gameSTLPoolFile);
	static void generateCppExcelInstanceClear(const myVector<string>& tableFileList, const string& gameBaseCppFileName);
	static void generateCppGlobalConfig(const CSVInfo& globalConfig, const string& tableFilePath);
	static void generateCppBuff(const CSVInfo& config);
};