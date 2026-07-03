#include "CodeCondition_Server.h"

// ConditionRegister.h和ConditionRegister.cpp
void CodeCondition_Server::generateCppConditionRegister(const myVector<string>& conditionList, const string& filePath)
{
	// 头文件
	string header;
	line(header, "// auto generate start");
	line(header, "#pragma once");
	line(header, "");
	line(header, "#include \"GameBase.h\"");
	line(header, "");
	line(header, "class ConditionRegister");
	line(header, "{");
	line(header, "public:");
	line(header, "\tstatic void registeAll();");
	line(header, "};");
	line(header, "// auto generate end", false);
	writeFile(filePath + "ConditionRegister.h", header);

	// 源文件
	string source;
	line(source, "// auto generate start");
	line(source, "#include \"GameHeader.h\"");
	line(source, "");
	line(source, "void ConditionRegister::registeAll()");
	line(source, "{");
	FOR_VECTOR(conditionList)
	{
		line(source, "\tmConditionFactoryManager->addFactory<Condition" + conditionList[i] + ">(EDCondition::" + conditionList[i] + "_ID);");
	}
	line(source, "}");
	line(source, "// auto generate end", false);
	writeFile(filePath + "ConditionRegister.cpp", source);
}