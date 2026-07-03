#include "CodeUnityBuild.h"
#include "CodeUnityBuild_Server.h"

void CodeUnityBuild::generate()
{
	if (ServerGameProjectPath.empty() || cppGamePath.empty() || cppFramePath.empty())
	{
		return;
	}
	print("正在生成UnityBuild");
	// 生成UnityBuild.cpp文件
	CodeUnityBuild_Server::generateCppUnityBuild(cppGamePath, "UnityBuildGame.cpp");
	CodeUnityBuild_Server::generateCppUnityBuild(cppFramePath, "UnityBuildFrame.cpp");
	print("完成生成UnityBuild");
	print("");
}