#include "CodeComponent_Server.h"

// GameComponentRegister.cpp
void CodeComponent_Server::generateGameComponentRegister(const myVector<string>& componentList, const string& filePath)
{
	string source;
	line(source, "// auto generate start");
	line(source, "#include \"GameHeader.h\"");
	line(source, "");
	line(source, "void GameComponentRegister::registeAll()");
	line(source, "{");
	FOR_VECTOR(componentList)
	{
		line(source, "\tmGameComponentFactoryManager->addFactory<" + componentList[i] + ">();");
	}
	line(source, "}");
	line(source, "// auto generate end", false);
	writeFile(filePath + "GameComponentRegister.cpp", source);
}