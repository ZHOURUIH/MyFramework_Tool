#include "CodeBaseCheck_Server.h"

void CodeBaseCheck_Server::doGenerate(const string& path)
{
	if (findString(path.c_str(), "Dependency/"))
	{
		return;
	}
	auto lines = openFile(path);
	for (int i = 0; i < lines.size(); ++i)
	{
		// 如果带逗号就跳过,基类中带逗号的无法添加BASE宏
		if (findString(lines[i].c_str(), ","))
		{
			continue;
		}
		const string className = findClassName(lines[i]);
		const string baseClassName = findClassBaseName(lines[i]);
		if (className != "" && baseClassName != "" && i + 2 < lines.size())
		{
			if (findString(lines[i + 2].c_str(), "BASE("))
			{
				lines[i + 2] = "\tBASE(" + className + ", " + baseClassName + ");";
			}
			else if (findString(lines[i + 1].c_str(), "{};"))
			{
				lines[i + 1] = "{";
				lines[i + 2] = "};";
				lines.insert(i + 2, "\tBASE(" + className + ", " + baseClassName + ");");
			}
			else
			{
				lines.insert(i + 2, "\tBASE(" + className + ", " + baseClassName + ");");
			}
		}
	}
	writeFile(path, lines);
}