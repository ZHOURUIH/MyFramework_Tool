#include "CodeSQLite.h"
#include "CodeSQLite_Client.h"
#include "CodeSQLite_Server.h"
#include "SQLiteDescription.h"
#include "SQLiteGlobal.h"
#include "SQLiteCommon.h"

myVector<string> CodeSQLite::mSQLiteForServerTableList;

void CodeSQLite::generate()
{
	if (SQLitePath.empty())
	{
		return;
	}
	print("正在生成SQLite");

	// 先读取表格描述
	myVector<SQLiteInfo> sqliteInfoList;
	for (const string& file : findFiles(SQLitePath, ".db"))
	{
		SQLiteDescription table;
		table.setTableName("Z_Description");
		table.init(file);
		const auto& list = table.queryAll();
		if (list.size() == 0)
		{
			continue;
		}
		SQLiteInfo info;
		info.mMemberList.clear();
		info.mSQLiteName = getFileNameNoSuffix(file, true);
		for (const auto& item : list)
		{
			TDDescription* data = item.second;
			if (item.first == 1)
			{
				info.mComment = data->mName;
			}
			else if (item.first == 2)
			{
				;
			}
			else if (item.first == 3)
			{
				if (data->mName == "All")
				{
					info.mOwner = OWNER::BOTH;
				}
				else if (data->mName == "Client")
				{
					info.mOwner = OWNER::CLIENT_ONLY;
				}
				else if (data->mName == "Server")
				{
					info.mOwner = OWNER::SERVER_ONLY;
				}
				else if (data->mName == "None")
				{
					info.mOwner = OWNER::NONE;
				}
				else
				{
					ERROR("表格所属错误:" + info.mSQLiteName);
				}
			}
			else if (item.first == 4)
			{
				info.mClientSQLite = StringUtility::SToBool(data->mName);
			}
			else
			{
				SQLiteMember member;
				if (data->mOwner == "All")
				{
					member.mOwner = OWNER::BOTH;
				}
				else if (data->mOwner == "Client")
				{
					member.mOwner = OWNER::CLIENT_ONLY;
				}
				else if (data->mOwner == "Server")
				{
					member.mOwner = OWNER::SERVER_ONLY;
				}
				else if (data->mOwner == "None")
				{
					member.mOwner = OWNER::NONE;
				}
				else
				{
					ERROR("owner错误:" + info.mSQLiteName);
				}
				member.mName = data->mName;
				member.mComment = data->mDesc;
				member.mType = data->mType;
				member.mLinkTable = data->mLinkTable;
				int leftPos = 0;
				int rightPos = 0;
				if (findSubstr(member.mType, "(", &leftPos) && findSubstr(member.mType, ")", &rightPos))
				{
					member.mEnumRealType = member.mType.substr(leftPos + 1, rightPos - leftPos - 1);
					member.mType = member.mType.erase(leftPos, rightPos - leftPos + 1);
				}
				info.mMemberList.push_back(member);
			}
		}
		SQLiteCommon tableMain;
		tableMain.setTableName(info.mSQLiteName.c_str());
		tableMain.init(file);
		const myMap<int, TDCommon*>& listMain = tableMain.queryAll();
		FOREACH(item0, listMain)
		{
			info.mDataMap.insert(item0->first, item0->second->getDataList());
		}
		sqliteInfoList.push_back(info);
	}
	
	// cpp
	if (!cppGamePath.empty())
	{
		string cppGameDataPath = cppGamePath + "DataBase/Excel/Data/";
		string cppGameTablePath = cppGamePath + "DataBase/Excel/Table/";
		myVector<SQLiteInfo> serverGameSQLiteList;
		mSQLiteForServerTableList.clear();
		for (const SQLiteInfo& info : sqliteInfoList)
		{
			if ((info.mOwner == OWNER::BOTH || info.mOwner == OWNER::SERVER_ONLY))
			{
				serverGameSQLiteList.push_back(info);
				mSQLiteForServerTableList.push_back(info.mSQLiteName);
			}
		}
		// 删除C++的代码文件,只删ExcelData中的,因为里面都是自动生成的,ExcelTable中的包含手动写的代码,而且是Excel和SQLite混在一起,就不删除了
		deleteFolder(cppGameDataPath);

		// 生成代码文件
		for (const SQLiteInfo& info : serverGameSQLiteList)
		{
			CodeSQLite_Server::generateCppSQLiteDataFile(info, cppGameDataPath);
			CodeSQLite_Server::generateCppSQLiteTableFile(info, cppGameTablePath);
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	// csharp
	if (!ClientHotFixPath.empty())
	{
		string csSQLiteDataHotFixPath = ClientHotFixPath + "DataBase/SQLite/Data/";
		string csSQLiteTableHotFixPath = ClientHotFixPath + "DataBase/SQLite/Table/";
		string csSQLiteTableDeclareHotFixPath = ClientHotFixPath + "Common/";

		// 筛选出Client的表格
		myVector<SQLiteInfo> clientSQLiteList;
		myVector<string> sqliteNameList;
		for (const SQLiteInfo& info : sqliteInfoList)
		{
			if (info.mOwner == OWNER::BOTH || info.mOwner == OWNER::CLIENT_ONLY)
			{
				clientSQLiteList.push_back(info);
				sqliteNameList.push_back(info.mSQLiteName);
			}
		}
		// 删除C#的代码文件,c#的只删除代码文件,不删除meta文件
		for (const string& str : findFiles(csSQLiteDataHotFixPath, ".cs"))
		{
			deleteFile(str);
		}

		// 生成代码文件
		for (const SQLiteInfo& info : clientSQLiteList)
		{
			// .cs代码的SQLite格式
			if (info.mClientSQLite)
			{
				CodeSQLite_Client::generateCSharpSQLiteDataFile(info, csSQLiteDataHotFixPath);
				CodeSQLite_Client::generateCSharpSQLiteTableFile(info, csSQLiteTableHotFixPath);
			}
		}

		// 在上一层目录生成SQLiteRegister.cs文件
		CodeSQLite_Client::generateCSharpSQLiteRegisteFileFile(clientSQLiteList, getFilePath(csSQLiteDataHotFixPath) + "/");
		CodeSQLite_Client::generateCSharpSQLiteDeclare(clientSQLiteList, csSQLiteTableDeclareHotFixPath);
	}
	print("完成生成SQLite");
	print("");
}

string CodeSQLite::paramNameToFunctionName(const string& paramName)
{
	myVector<string> elements;
	split(paramName.c_str(), "_", elements);
	string functionName;
	FOR_VECTOR(elements)
	{
		string temp = toLower(elements[i]);
		temp[0] = toUpper(temp[0]);
		functionName += temp;
	}
	return functionName;
}