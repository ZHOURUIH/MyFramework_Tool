#include "CodeSQLite_Server.h"
#include "SQLiteDescription.h"
#include "SQLiteGlobal.h"
#include "SQLiteCommon.h"

// ExcelData.h和ExcelData.cpp文件
void CodeSQLite_Server::generateCppSQLiteDataFile(const SQLiteInfo& sqliteInfo, const string& dataFilePath)
{
	// 不含ID的成员字段列表
	myVector<SQLiteMember> memberNoIDList;
	for (const SQLiteMember& member : sqliteInfo.mMemberList)
	{
		if (member.mName == "ID")
		{
			continue;
		}
		memberNoIDList.push_back(member);
	}
	// 不含ID以及非服务器字段的成员字段列表
	myVector<SQLiteMember> memberUsedInServerNoIDList;
	for (const SQLiteMember& member : memberNoIDList)
	{
		if (member.mOwner != OWNER::SERVER_ONLY && member.mOwner != OWNER::BOTH)
		{
			continue;
		}
		memberUsedInServerNoIDList.push_back(member);
	}

	// first是变量名,second是注释,用于通过变量来访问ID
	myMap<int, pair<string, string>> variableList;
	for (const auto& item : sqliteInfo.mDataMap)
	{
		const auto& tempMap = item.second;
		// 从固定的字段名中获取变量名
		const string& variableName = tempMap.get("VariableName", "");
		const string& variableComment = tempMap.get("VariableComment", "");
		if (variableName.length() > 0)
		{
			variableList.insert(item.first, make_pair(variableName, variableComment));
		}
	}

	// ExcelData.h
	string header;
	string dataClassName = "ED" + sqliteInfo.mSQLiteName;
	line(header, "// auto generate start");
	line(header, "#pragma once");
	line(header, "");
	line(header, "#include \"ExcelData.h\"");
	line(header, "");
	line(header, "// " + sqliteInfo.mComment);
	line(header, "class " + dataClassName + " : public ExcelData");
	line(header, "{");
	line(header, "\tBASE(" + dataClassName + ", ExcelData);");
	if (variableList.size() > 0 || memberUsedInServerNoIDList.size() > 0)
	{
		line(header, "public:");
	}
	if (variableList.size() > 0)
	{
		if (sqliteInfo.mMemberList.size() > 3)
		{
			for (const auto& item : variableList)
			{
				string str = "\tstatic constexpr int " + item.second.first + "_ID = " + IToS(item.first) + ";";
				appendWithAlign(str, "// " + item.second.second, 64);
				line(header, str);
			}
			line(header, "");
			for (const auto& item : variableList)
			{
				string str = "\tstatic " + dataClassName + "* " + item.second.first + ";";
				appendWithAlign(str, "// " + item.second.second, 64);
				line(header, str);
			}
		}
		else
		{
			for (const auto& item : variableList)
			{
				string str = "\tstatic constexpr int " + item.second.first + " = " + IToS(item.first) + ";";
				appendWithAlign(str, "// " + item.second.second, 64);
				line(header, str);
			}
		}
		line(header, "");
	}
	for (const SQLiteMember& member : memberUsedInServerNoIDList)
	{
		const string& type = member.mType;
		const string& name = member.mName;
		string memberLine;
		if (type == "byte" ||
			type == "char" ||
			type == "ushort" ||
			type == "short" ||
			type == "int" ||
			type == "uint" ||
			type == "llong" ||
			type == "ullong")
		{
			memberLine = "\t" + type + " m" + name + " = 0;";
		}
		else if (type == "bool")
		{
			memberLine = "\t" + type + " m" + name + " = false;";
		}
		else if (type == "float")
		{
			memberLine = "\t" + type + " m" + name + " = 0.0f;";
		}
		else if (!member.mEnumRealType.empty())
		{
			if (startWith(type, "Vector<"))
			{
				memberLine = "\t" + type + " m" + name + ";";
			}
			else
			{
				memberLine = "\t" + type + " m" + name + " = (" + type + ")0;";
			}
		}
		else
		{
			memberLine = "\t" + type + " m" + name + ";";
		}
		appendWithAlign(memberLine, "// " + member.mComment, 60);
		line(header, memberLine);
	}
	line(header, "public:");
	line(header, "\tvoid cloneTo(ExcelData* target) override;");
	line(header, "\tvoid read(SerializerRead* reader) override;");
	if (variableList.size() > 0 && sqliteInfo.mMemberList.size() > 3)
	{
		line(header, "\tstatic void postLoadAll(ExcelTableBase* tableBase);");
	}
	else
	{
		line(header, "\tstatic void postLoadAll(ExcelTableBase* tableBase){}");
	}
	line(header, "};");
	line(header, "// auto generate end", false);
	writeFile(dataFilePath + dataClassName + ".h", header);

	// ExcelData.cpp
	string source;
	line(source, "// auto generate start");
	line(source, "#include \"" + dataClassName + ".h\"");
	line(source, "");
	if (sqliteInfo.mMemberList.size() > 3)
	{
		for (const auto& item : variableList)
		{
			line(source, dataClassName + "* " + dataClassName + "::" + item.second.first + " = nullptr;");
		}
		line(source, "");
	}
	line(source, "void " + dataClassName + "::cloneTo(ExcelData* target)");
	line(source, "{");
	line(source, "\tbase::cloneTo(target);");
	// 先检查一下有没有需要拷贝的属性
	if (memberUsedInServerNoIDList.size() > 0)
	{
		line(source, "\tauto* targetData = static_cast<This*>(target);");
		for (const SQLiteMember& member : memberUsedInServerNoIDList)
		{
			const string& name = member.mName;
			// 如果是列表则调用列表的cloneTo
			if (startWith(name, "Vector<"))
			{
				line(source, "\tm" + name + ".cloneTo(targetData->m" + name + ");");
			}
			else
			{
				line(source, "\ttargetData->m" + name + " = m" + name + ";");
			}
		}
	}
	line(source, "}");
	line(source, "");
	line(source, "void " + dataClassName + "::read(SerializerRead* reader)");
	line(source, "{");
	line(source, "\tbase::read(reader);");
	for (const SQLiteMember& member : memberUsedInServerNoIDList)
	{
		const string& type = member.mType;
		const string& name = member.mName;
		if (type == "string")
		{
			line(source, "\treader->readString(m" + name + ");");
		}
		else if (type == "Vector2Int")
		{
			line(source, "\treader->readVector2Int(m" + name + ");");
		}
		else if (type == "Vector2")
		{
			line(source, "\treader->readVector2(m" + name + ");");
		}
		else if (type == "Vector3")
		{
			line(source, "\treader->readVector3(m" + name + ");");
		}
		else if (type == "Vector3Int")
		{
			line(source, "\treader->readVector3Int(m" + name + ");");
		}
		else if (startWith(type, "Vector<"))
		{
			const string elementType = type.substr(strlen("Vector<"), type.length() - strlen("Vector<") - 1);
			if (elementType == "string")
			{
				line(source, "\treader->readStringList(m" + name + ");");
			}
			else if (elementType == "Vector2")
			{
				line(source, "\treader->readVector2List(m" + name + ");");
			}
			else if (elementType == "Vector2Int")
			{
				line(source, "\treader->readVector2IntList(m" + name + ");");
			}
			else if (elementType == "Vector3")
			{
				line(source, "\treader->readVector3List(m" + name + ");");
			}
			else if (elementType == "Vector3Int")
			{
				line(source, "\treader->readVector3IntList(m" + name + ");");
			}
			else
			{
				line(source, "\treader->readList(m" + name + ");");
			}
		}
		else
		{
			line(source, "\treader->read(m" + name + ");");
		}
	}
	line(source, "}");
	if (variableList.size() > 0 && sqliteInfo.mMemberList.size() > 3)
	{
		line(source, "void " + dataClassName + "::postLoadAll(ExcelTableBase* tableBase)");
		line(source, "{");
		line(source, "\tauto* table = static_cast<ExcelTable<" + dataClassName + ">*>(tableBase);");
		for (const auto& item : variableList)
		{
			line(source, "\t" + item.second.first + " = " + "table->getData(" + item.second.first + "_ID);");
		}
		line(source, "}");
	}
	line(source, "// auto generate end", false);
	writeFile(dataFilePath + dataClassName + ".cpp", source);
}

// ExcelTable.h和ExcelTable.cpp文件
void CodeSQLite_Server::generateCppSQLiteTableFile(const SQLiteInfo& sqliteInfo, const string& tableFilePath)
{
	// ExcelTable.h
	string dataClassName = "ED" + sqliteInfo.mSQLiteName;
	string tableClassName = "Excel" + sqliteInfo.mSQLiteName;
	string tableHeaderFile = tableFilePath + tableClassName + ".h";
	if (!isFileExist(tableHeaderFile))
	{
		string table;
		line(table, "#pragma once");
		line(table, "");
		line(table, "#include \"" + dataClassName + ".h\"");
		line(table, "#include \"ExcelTable.h\"");
		line(table, "");
		line(table, "class " + tableClassName + " : public ExcelTable<" + dataClassName + ">");
		line(table, "{");
		line(table, "public:");
		line(table, "\t// auto generate start");
		line(table, "\tvoid checkAllDataDefault() override;");
		line(table, "\t// auto generate end");
		line(table, "};", false);

		writeFile(tableHeaderFile, table);
	}
	else
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(tableHeaderFile, codeList, lineStart,
			[](const string& codeLine) { return endWith(codeLine, "// auto generate start"); },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }, false))
		{
			// 如果找不到就在第一个public下一行插入
			FOR_VECTOR(codeList)
			{
				if (endWith(codeList[i], "public:"))
				{
					codeList.insert(++i, "\t// auto generate start");
					lineStart = i;
					codeList.insert(++i, "\t// auto generate end");
					break;
				}
			}
		}
		codeList.insert(++lineStart, "\tvoid checkAllDataDefault() override;");
		writeFile(tableHeaderFile, codeList);
	}

	// ExcelTable.cpp
	myVector<string> insertLines;
	insertLines.push_back("void " + tableClassName + "::checkAllDataDefault()");
	insertLines.push_back("{");
	insertLines.push_back("\tfor (const auto& item : getAllData())");
	insertLines.push_back("\t{");
	insertLines.push_back("\t\t" + dataClassName + "* data = item.second;");
	bool hasCheck = false;
	for (const SQLiteMember& member : sqliteInfo.mMemberList)
	{
		if (member.mOwner == OWNER::BOTH || member.mOwner == OWNER::SERVER_ONLY)
		{
			const string& name = member.mName;
			const string& linkTable = member.mLinkTable;
			if (!linkTable.empty())
			{
				if (!isFileExist(ExcelPath + linkTable + ".csv") && !isFileExist(ExcelPath + linkTable + ".db"))
				{
					ERROR("找不到服务器索引的表格:" + linkTable + ", 当前表格:" + sqliteInfo.mSQLiteName + ", 字段名:" + name);
					continue;
				}
			}
			if (!linkTable.empty())
			{
				if (member.mEnumRealType.empty())
				{
					insertLines.push_back("\t\tmExcel" + linkTable + "->checkData(data->m" + name + ", item.first, this);");
				}
				else
				{
					insertLines.push_back("\t\tmExcel" + linkTable + "->checkData((int)data->m" + name + ", item.first, this);");
				}
				hasCheck = true;
			}
			if (!member.mEnumRealType.empty())
			{
				insertLines.push_back("\t\tcheckEnumResult(GameEnumCheck::checkEnum(data->m" + name + "), \"m" + name + "\", item.first);");
				hasCheck = true;
			}
		}
	}
	insertLines.push_back("\t}");
	insertLines.push_back("}");
	// 如果没有任何需要检查的,就只插入一个空函数
	if (!hasCheck)
	{
		insertLines.clear();
		insertLines.push_back("void " + tableClassName + "::checkAllDataDefault() {}");
	}
	string tableSourceFile = tableFilePath + tableClassName + ".cpp";
	if (!isFileExist(tableSourceFile))
	{
		string table;
		line(table, "#include \"GameHeader.h\"");
		line(table, "");
		line(table, "// auto generate start");
		for (const string& str : insertLines)
		{
			line(table, str);
		}
		line(table, "// auto generate end", false);
		writeFile(tableSourceFile, table);
	}
	else
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(tableSourceFile, codeList, lineStart,
			[](const string& codeLine) { return endWith(codeLine, "// auto generate start"); },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }, false))
		{
			// 如果找不到就在最后一行插入
			int lineIndex = codeList.size() - 1;
			codeList.insert(++lineIndex, "");
			codeList.insert(++lineIndex, "// auto generate start");
			codeList.insert(++lineIndex, "// auto generate end");
			lineStart = lineIndex - 1;
		}
		for (const string& str : insertLines)
		{
			codeList.insert(++lineStart, str);
		}
		writeFile(tableSourceFile, codeList);
	}
}