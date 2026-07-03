#include "CodeSQLite_Client.h"
#include "SQLiteDescription.h"
#include "SQLiteGlobal.h"
#include "SQLiteCommon.h"

// SQLiteData.cs文件
void CodeSQLite_Client::generateCSharpSQLiteDataFile(const SQLiteInfo& sqliteInfo, const string& dataFileHotFixPath)
{
	if (!sqliteInfo.mClientSQLite || sqliteInfo.mOwner == OWNER::SERVER_ONLY || sqliteInfo.mOwner == OWNER::NONE)
	{
		return;
	}
	string file;
	string dataClassName = "SD" + sqliteInfo.mSQLiteName;
	line(file, "// auto generate start");
	line(file, "using Mono.Data.Sqlite;");
	line(file, "using System;");
	line(file, "using System.Collections.Generic;");
	line(file, "using UnityEngine;");
	line(file, "");
	line(file, "// " + sqliteInfo.mComment);
	line(file, "public class " + dataClassName + " : SQLiteData");
	line(file, "{");
	myVector<pair<string, string>> listMemberList;
	for (const SQLiteMember& member : sqliteInfo.mMemberList)
	{
		const string& name = member.mName;
		if (name == "ID")
		{
			continue;
		}
		line(file, "\tpublic const string " + name + " = " + "\"" + name + "\";");
	}
	for (const SQLiteMember& member : sqliteInfo.mMemberList)
	{
		const string& name = member.mName;
		if (name == "ID")
		{
			continue;
		}
		// 因为模板文件是按照C++来写的,但是有些类型在C#中是没有的,所以要转换为C#中对应的类型
		string typeName = cppTypeToCSharpType(member.mType);
		if (!member.mEnumRealType.empty())
		{
			int pos0;
			if (findString(typeName.c_str(), "List<", &pos0))
			{
				int pos1;
				findString(typeName.c_str(), ">", &pos1);
				replace(typeName, pos0 + (int)strlen("List<"), pos1, member.mEnumRealType);
			}
			else
			{
				typeName = convertToCSharpType(member.mEnumRealType);
			}
		}

		string publicType;
		if (member.mOwner == OWNER::CLIENT_ONLY || member.mOwner == OWNER::BOTH)
		{
			publicType = "public";
		}
		else
		{
			publicType = "protected";
		}
		// 列表类型的成员变量存储到单独的列表,因为需要分配内存
		bool isList = findString(typeName.c_str(), "List");
		if (isList)
		{
			listMemberList.push_back(make_pair(typeName, name));
		}

		string memberLine;
		if (!isList)
		{
			memberLine = "\t" + publicType + " " + typeName + " m" + name + ";";
		}
		else
		{
			memberLine = "\t" + publicType + " " + typeName + " m" + name + " = new();";
		}
		appendWithAlign(memberLine, "// " + member.mComment, 52);
		line(file, memberLine);
	}
	line(file, "\tpublic override void parse(SqliteDataReader reader)");
	line(file, "\t{");
	line(file, "\t\tbase.parse(reader);");
	const uint memberCount = sqliteInfo.mMemberList.size();
	FOR_I(memberCount)
	{
		const SQLiteMember& member = sqliteInfo.mMemberList[i];
		if (member.mName == "ID")
		{
			continue;
		}
		line(file, "\t\tparseParam(reader, ref m" + member.mName + ", " + IToS(i) + ");");
	}
	line(file, "\t}");
	line(file, "}");
	line(file, "// auto generate end", false);
	writeFile(dataFileHotFixPath + dataClassName + ".cs", file);
}

// SQLiteTable.cs文件
void CodeSQLite_Client::generateCSharpSQLiteTableFile(const SQLiteInfo& sqliteInfo, const string& tableFileHotFixPath)
{
	if (!sqliteInfo.mClientSQLite || sqliteInfo.mOwner == OWNER::SERVER_ONLY || sqliteInfo.mOwner == OWNER::NONE)
	{
		return;
	}
	string tableClassName = "SQLite" + sqliteInfo.mSQLiteName;
	const string fullPath = tableFileHotFixPath + tableClassName + ".cs";
	// 不覆盖现有文件
	if (isFileExist(fullPath))
	{
		return;
	}
	// SQLiteTable.cs文件
	string table;
	line(table, "using System;");
	line(table, "using System.Collections.Generic;");
	line(table, "");
	line(table, "public class " + tableClassName + " : SQLiteTable");
	line(table, "{");
	line(table, "}", false);
	writeFile(fullPath, table);
}

// SQLiteRegister.cs文件
void CodeSQLite_Client::generateCSharpSQLiteRegisteFileFile(const myVector<SQLiteInfo>& sqliteInfo, const string& fileHotFixPath)
{
	string hotFixfile;
	line(hotFixfile, "#if USE_SQLITE");
	line(hotFixfile, "// auto generate start");
	line(hotFixfile, "using System;");
	line(hotFixfile, "using static GBR;");
	line(hotFixfile, "using static FrameBaseHotFix;");
	line(hotFixfile, "");
	line(hotFixfile, "public class SQLiteRegister");
	line(hotFixfile, "{");
	line(hotFixfile, "\tpublic static void registeAll()");
	line(hotFixfile, "\t{");
	for (const SQLiteInfo& info : sqliteInfo)
	{
		if (info.mClientSQLite && info.mOwner != OWNER::SERVER_ONLY && info.mOwner != OWNER::NONE)
		{
			string lineStr = "\t\tregisteTable(out mSQLite%s, typeof(SD%s), \"%s\");";
			replaceAll(lineStr, "%s", info.mSQLiteName);
			line(hotFixfile, lineStr);
		}
	}
	line(hotFixfile, "");
	line(hotFixfile, "\t\t// 进入热更以后,所有资源都处于可用状态");
	line(hotFixfile, "\t\tmSQLiteManager.resourceAvailable();");
	line(hotFixfile, "\t}");
	line(hotFixfile, "\t//------------------------------------------------------------------------------------------------------------------------------");
	line(hotFixfile, "\tprotected static void registeTable<T>(out T table, Type dataType, string tableName) where T : SQLiteTable");
	line(hotFixfile, "\t{");
	line(hotFixfile, "\t\ttable = mSQLiteManager.registeTable(typeof(T), dataType, tableName) as T;");
	line(hotFixfile, "\t}");
	line(hotFixfile, "}");
	line(hotFixfile, "// auto generate end");
	line(hotFixfile, "#endif", false);
	writeFile(fileHotFixPath + "SQLiteRegister.cs", hotFixfile);
}

// GameBaseHotFix.cs文件
void CodeSQLite_Client::generateCSharpSQLiteDeclare(const myVector<SQLiteInfo>& sqliteInfo, const string& fileHotFixPath)
{
	myVector<string> insertLines;
	for (const SQLiteInfo& info : sqliteInfo)
	{
		if (info.mClientSQLite && info.mOwner != OWNER::SERVER_ONLY && info.mOwner != OWNER::NONE)
		{
			insertLines.push_back("\tpublic static SQLite" + info.mSQLiteName + " mSQLite" + info.mSQLiteName + ";");
		}
	}

	string fileName = fileHotFixPath + "GameBaseHotFix.cs";
	if (!isFileExist(fileName))
	{
		string file;
		line(file, "using System;");
		line(file, "");
		line(file, "public class GBR");
		line(file, "{");
		line(file, "// auto generate SQLite start");
		for (const string& str : insertLines)
		{
			line(file, str);
		}
		line(file, "// auto generate SQLite end");
		line(file, "}", false);
		writeFile(fileName, file);
	}
	else
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(fileName, codeList, lineStart,
			[](const string& codeLine) { return endWith(codeLine, "// auto generate SQLite start"); },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate SQLite end"); }, false))
		{
			// 如果找不到就在第一个{下一行插入
			FOR_VECTOR(codeList)
			{
				if (!codeList[i].empty() && codeList[i][0] == '{')
				{
					lineStart = i;
					codeList.insert(++lineStart, "\t// auto generate SQLite start");
					codeList.insert(++lineStart, "\t// auto generate SQLite end");
					break;
				}
			}
			lineStart = lineStart - 1;
		}
		for (const string& str : insertLines)
		{
			codeList.insert(++lineStart, str);
		}
		writeFile(fileName, codeList);
	}
}