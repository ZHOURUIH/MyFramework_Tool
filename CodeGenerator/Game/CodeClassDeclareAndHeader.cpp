#include "CodeClassDeclareAndHeader.h"
#include "CodeClassDeclareAndHeader_Server.h"

void CodeClassDeclareAndHeader::generate()
{
	if (ServerFrameProjectPath.empty() || cppGamePath.empty())
	{
		return;
	}
	print("正在生成类声明");
	CodeClassDeclareAndHeader_Server::generateCppFrameClassAndHeader(ServerFrameProjectPath, cppFramePath + "Common/");
	CodeClassDeclareAndHeader_Server::generateCppGameClassAndHeader(cppGamePath, cppGamePath + "Common/");
	print("完成生成类声明");
	print("");
}