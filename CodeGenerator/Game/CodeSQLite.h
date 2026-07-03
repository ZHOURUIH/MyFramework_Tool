#pragma once

#include "CodeUtility.h"

// 只能在CodeExcel之前执行
class CodeSQLite : public CodeUtility
{
public:
	static myVector<string> mSQLiteForServerTableList;
public:
	static void generate();
protected:
	static string paramNameToFunctionName(const string& paramName);
};