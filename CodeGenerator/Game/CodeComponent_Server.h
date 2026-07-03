#pragma once

#include "CodeUtility.h"

class CodeComponent_Server : public CodeUtility
{
public:
	static void generateGameComponentRegister(const myVector<string>& componentList, const string& filePath);
};