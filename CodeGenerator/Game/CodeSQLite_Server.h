#pragma once

#include "CodeSQLite.h"

// 只能在CodeExcel之前执行
class CodeSQLite_Server : public CodeSQLite
{
public:
	// c++,服务器生成的都是按照Excel的格式生成的,因为无论是Excel还是SQLite在服务器都是生成同样的二进制文件
	// 服务器就不需要生成SQLite表格的全局静态变量了,因为都是转成跟Excel转换以后一样的二进制了,代码不区分SQLite还是Excel转换而来的
	static void generateCppSQLiteDataFile(const SQLiteInfo& sqliteInfo, const string& dataFilePath);
	static void generateCppSQLiteTableFile(const SQLiteInfo& sqliteInfo, const string& tableFilePath);
};