#include "CodeEnumCheck.h"
#include "CodeEnumCheck_Server.h"

void CodeEnumCheck::generate()
{
	if (cppGamePath.empty() || cppFramePath.empty())
	{
		return;
	}
	print("正在生成枚举检测");
	// 只收集指定文件中定义的枚举
	CodeEnumCheck_Server::doGenerate("GameEnumCheck", cppGamePath + "Common/", cppGamePath + "Common/GameEnum.h");
	CodeEnumCheck_Server::doGenerate("FrameEnumCheck", cppFramePath + "Common/", cppFramePath + "Common/FrameEnum.h");
	print("完成生成枚举检测");
	print("");
}