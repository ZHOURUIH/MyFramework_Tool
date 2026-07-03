#pragma once

#include "CodeUtility.h"

class CodeCondition_Client : public CodeUtility
{
public:
	static void generateCSharpConditionRegister(const myVector<string>& conditionList, const string& filePath);
};