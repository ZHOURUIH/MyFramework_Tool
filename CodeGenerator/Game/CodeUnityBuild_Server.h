#pragma once

#include "CodeUtility.h"

class CodeUnityBuild_Server : public CodeUtility
{
public:
	static void generateCppUnityBuild(const string& filePath, const string& unityBuildFileName);
};