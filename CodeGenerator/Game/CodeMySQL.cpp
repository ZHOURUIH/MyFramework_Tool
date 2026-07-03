#include "CodeMySQL.h"
#include "CodeMySQL_Server.h"

void CodeMySQL::generate()
{
	if (cppGamePath.empty())
	{
		return;
	}
	print("正在生成MySQL");
	// 解析模板文件
	// 整个文件是否已经开始解析
	bool fileStart = false;
	// 是否已经开始解析一个表格的数据体
	bool packetStart = false;
	// 是否已经开始解析一个表格中的索引
	bool indexStart = false;
	myVector<string> lines = openFile("MySQL.txt");
	if (lines.size() == 0)
	{
		return;
	}
	myVector<MySQLInfo> mySQLInfoList;
	MySQLInfo tempInfo;
	FOR_VECTOR(lines)
	{
		if (lines[i] == START_FALG)
		{
			fileStart = true;
			continue;
		}
		if (!fileStart)
		{
			continue;
		}
		string line = lines[i];
		// 去除所有制表符,分号
		removeAll(line, '\t', ';');
		// 成员变量列表起始
		if (line == "{")
		{
			indexStart = false;
			packetStart = true;
			string comment;
			// 表格注释,只取一行,不支持多行
			if (i >= 2 && startWith(lines[i - 2], "//"))
			{
				comment = lines[i - 2].substr(strlen("//"));
				removeStartAll(comment, ' ');
			}
			string tableTitle = lines[i - 1];
			removeAll(tableTitle, '\t', ';');
			myVector<string> titleVector;
			split(tableTitle.c_str(), ":", titleVector);
			if (titleVector.size() != 2)
			{
				ERROR("mysql表格的格式错误: " + lines[i - 1]);
				return;
			}
			// 移除标签
			tempInfo.init(titleVector[0], titleVector[1], comment);
			continue;
		}
		// 成员变量列表结束
		if (line == "}")
		{
			mySQLInfoList.push_back(tempInfo);
			packetStart = false;
			continue;
		}
		if (line == "index:")
		{
			indexStart = true;
			continue;
		}
		if (packetStart)
		{
			// 当前是在解析索引
			if (indexStart)
			{
				tempInfo.mIndexList.push_back(line);
			}
			// 当前是在解析列表的字段
			else
			{
				tempInfo.mMemberList.push_back(parseMySQLMemberLine(line));
			}
		}
	}
	// Game
	string cppGameDataPath = cppGamePath + "DataBase/MySQL/Data/";
	string cppGameTablePath = cppGamePath + "DataBase/MySQL/Table/";
	deleteFolder(cppGameDataPath);
	for (const MySQLInfo& info : mySQLInfoList)
	{
		// 生成代码文件
		CodeMySQL_Server::generateCppMySQLDataFile(info, cppGameDataPath);
		CodeMySQL_Server::generateCppMySQLTableFile(info, cppGameTablePath);
	}
	// 上一层目录生成MySQLHeader.h
	string totalHeaderGamePath = cppGameDataPath;
	removeEnd(totalHeaderGamePath, '/');
	totalHeaderGamePath = getFilePath(totalHeaderGamePath) + "/";
	const string gameBaseHeaderPath = cppGamePath + "Common/GameBase.h";
	const string gameBaseSourcePath = cppGamePath + "Common/GameBase.cpp";
	CodeMySQL_Server::generateCppMySQLRegisteFile(mySQLInfoList, totalHeaderGamePath);
	myVector<string> newGameList;
	for (const MySQLInfo& info : mySQLInfoList)
	{
		newGameList.push_back("MD" + info.mMySQLClassName);
	}
	CodeMySQL_Server::generateMySQLInstanceDeclare(mySQLInfoList, gameBaseHeaderPath, "");
	CodeMySQL_Server::generateMySQLInstanceDefine(mySQLInfoList, gameBaseSourcePath);
	CodeMySQL_Server::generateMySQLInstanceClear(mySQLInfoList, gameBaseSourcePath);
	print("完成生成MySQL");
	print("");
}