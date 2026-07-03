#include "CodeBaseCheck.h"
#include "CodeBaseCheck_Server.h"

void CodeBaseCheck::generate()
{
	if (ServerGameProjectPath.empty())
	{
		return;
	}
	print("正在进行Base检测");
	// 只收集指定文件中定义的枚举
	myVector<string> files;
	findFiles(ServerGameProjectPath, files, ".h");
	findFiles(ServerFrameProjectPath, files, ".h");
	for (const string& file : files)
	{
		CodeBaseCheck_Server::doGenerate(file);
	}
	print("完成Base检测");
	print("");
}