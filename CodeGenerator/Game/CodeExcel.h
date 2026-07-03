#pragma once

#include "CodeUtility.h"

class CodeExcel : public CodeUtility
{
public:
	static void generate();
protected:
	static string paramNameToFunctionName(const string& paramName);
};