#include "CodeComponent.h"
#include "CodeComponent_Server.h"

void CodeComponent::generate()
{
	if (cppGamePath.empty())
	{
		return;
	}
	print("正在生成组件代码");
	// Game
	const string cppGameRegisterPath = cppGamePath + "/Component/";
	myVector<string> gameComFiles = findClass(cppGamePath, 
	[](const string& fileName) 
	{ 
		return startWith(fileName, "COM") && 
			   fileName != "COMCharacterGame" && 
			   fileName != "COMPlayer" && 
			   fileName != "COMNPC"; 
	},
	[](const string& line)
	{
		return findSubstr(line, " : public GameComponent") || 
			   findSubstr(line, " : public COMCharacterGame") || 
			   findSubstr(line, " : public COMPlayer") || 
			   findSubstr(line, " : public COMNPC") || 
			   findSubstr(line, " : public COMCharacterSkill") || 
			   findSubstr(line, " : public GameComponent");
	});
	// 生成StringDefine文件
	CodeUtility::generateStringDefine(gameComFiles, 20000, "Component", cppGameStringDefineHeaderFile);
	// ComponentRegister.cpp
	CodeComponent_Server::generateGameComponentRegister(gameComFiles, cppGameRegisterPath);
	print("完成生成组件代码");
	print("");
}