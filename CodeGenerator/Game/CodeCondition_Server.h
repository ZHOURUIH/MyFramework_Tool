#pragma once

#include "CodeUtility.h"

class CodeCondition_Server : public CodeUtility
{
public:
	static void generateCppConditionRegister(const myVector<string>& conditionList, const string& filePath);
};