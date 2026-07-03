#pragma once

#include "CodeUtility.h"

class CodeBaseCheck_Server : public CodeUtility
{
public:
	static void doGenerate(const string& path);
};