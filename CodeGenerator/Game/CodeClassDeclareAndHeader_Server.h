#pragma once

#include "CodeUtility.h"

class CodeClassDeclareAndHeader_Server : public CodeUtility
{
public:
	static void generateCppFrameClassAndHeader(const string& path, const string& targetFilePath);
	static void generateCppGameClassAndHeader(const string& path, const string& targetFilePath);
};