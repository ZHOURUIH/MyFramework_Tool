#pragma once

#include "CodeSQLite.h"

// 只能在CodeExcel之前执行
class CodeSQLite_Client : public CodeSQLite
{
public:
	static void generateCSharpSQLiteDataFile(const SQLiteInfo& sqliteInfo, const string& dataFileHotFixPath);
	static void generateCSharpSQLiteTableFile(const SQLiteInfo& sqliteInfo, const string& tableFileHotFixPath);
	static void generateCSharpSQLiteRegisteFileFile(const myVector<SQLiteInfo>& sqliteInfo, const string& fileHotFixPath);
	static void generateCSharpSQLiteDeclare(const myVector<SQLiteInfo>& info, const string& fileHotFixPath);
};