#include "CodeExcel_Client.h"

void CodeExcel_Client::generate(const myVector<CSVInfo>& infoList)
{
	string csExcelDataPath = ClientHotFixPath + "DataBase/Excel/Data/";
	string csExcelTablePath = ClientHotFixPath + "DataBase/Excel/Table/";
	string csExcelTableDeclareHotFixPath = ClientHotFixPath + "Common/";

	// 筛选出Client的表格
	myVector<CSVInfo> clientExcelList;
	myVector<string> tableNameList;
	for (const CSVInfo& info : infoList)
	{
		if (info.mHeader.mOwner == OWNER::BOTH || info.mHeader.mOwner == OWNER::CLIENT_ONLY)
		{
			clientExcelList.push_back(info);
			tableNameList.push_back(info.mHeader.mTableName);
		}
	}
	// 删除C#的代码文件,c#的只删除代码文件,不删除meta文件
	for (const string& str : findFiles(csExcelDataPath, ".cs"))
	{
		deleteFile(str);
	}
	for (const string& str : findFiles(csExcelTablePath, ".cs"))
	{
		// 只删除已经不存在的表格
		if (!tableNameList.contains(removeStartString(getFileNameNoSuffix(str, true), "Excel")))
		{
			deleteFile(str);
		}
	}

	// 生成代码文件
	for (const CSVInfo& info : clientExcelList)
	{
		CodeExcel_Client::generateCSharpExcelDataFile(info, csExcelDataPath);
		CodeExcel_Client::generateCSharpExcelTableFile(info, csExcelTablePath);
	}

	// 在上一层目录生成ExcelRegister.cs文件
	CodeExcel_Client::generateCSharpExcelRegisteFileFile(clientExcelList, getFilePath(csExcelDataPath) + "/");
	CodeExcel_Client::generateCSharpExcelDeclare(clientExcelList, csExcelTableDeclareHotFixPath);

	for (const CSVInfo& info : clientExcelList)
	{
		if (info.mHeader.mTableName == "Global")
		{
			CodeExcel_Client::generateCSharpGlobalConfig(info, csExcelDataPath);
		}
		else if (info.mHeader.mTableName == "Buff")
		{
			CodeExcel_Client::generateCSharpBuff(info);
		}
	}
}

// ExcelData.cs文件
void CodeExcel_Client::generateCSharpExcelDataFile(const CSVInfo& info, const string& dataFileHotFixPath)
{
	if (info.mHeader.mOwner == OWNER::SERVER_ONLY || info.mHeader.mOwner == OWNER::NONE)
	{
		return;
	}

	myMap<string, int> colNameList;
	int variableNameIndex = -1;
	int variableCommentIndex = -1;
	FOR_VECTOR(info.mHeader.mColumnDataList)
	{
		const string& name = info.mHeader.mColumnDataList[i]->mName;
		colNameList.insert(name, i);
		if (name == "VariableName")
		{
			variableNameIndex = i;
		}
		else if (name == "VariableComment")
		{
			variableCommentIndex = i;
		}
	}

	int customParamCount = 0;
	FOR_I(20)
	{
		if (!colNameList.contains("Param" + IToS(i)) ||
			!colNameList.contains("ParamType" + IToS(i)) ||
			!colNameList.contains("ParamName" + IToS(i)) ||
			!colNameList.contains("ParamComment" + IToS(i)))
		{
			customParamCount = i;
			break;
		}
	}

	// first是变量名,second是注释,用于通过变量来访问ID
	myMap<int, pair<string, string>> variableList;
	if (variableNameIndex >= 0)
	{
		for (const auto& row : info.mDataList)
		{
			if (!row[variableNameIndex].empty())
			{
				variableList.insert(StringUtility::SToI(row[0]), make_pair(row[variableNameIndex], row[variableCommentIndex]));
			}
		}
	}

	string file;
	string dataClassName = "ED" + info.mHeader.mTableName;
	line(file, "// auto generate start");
	line(file, "using System;");
	line(file, "using System.Collections.Generic;");
	line(file, "using UnityEngine;");
	line(file, "");
	line(file, "// " + info.mHeader.mComment);
	line(file, "public class " + dataClassName + " : ExcelDataT<" + dataClassName + ">");
	line(file, "{");
	// 表示ID的静态变量
	if (variableList.size() > 0)
	{
		// 如果只有3列, 也就是除了ID, 变量名和注释以外就没了, 那生成对象也没意义, 只需要生成ID就行了
		// 大于3列就需要生成对象
		if (info.mHeader.mColumnDataList.size() > 3)
		{
			for (const auto& item : variableList)
			{
				string str = "\tpublic const int " + item.second.first + "_ID = " + IToS(item.first) + ";";
				appendWithAlign(str, "// " + item.second.second, 52);
				line(file, str);
			}
			line(file, "");
			for (const auto& item : variableList)
			{
				string str = "\tpublic static " + dataClassName + " _" + item.second.first + ";";
				appendWithAlign(str, "// " + item.second.second, 52);
				line(file, str);
			}
			line(file, "");
			for (const auto& item : variableList)
			{
				string str = "\tpublic static " + dataClassName + " " + item.second.first + 
					" { get { return _" + item.second.first + " ??= mTable.query(" + item.second.first + "_ID); } }";
				appendWithAlign(str, "// " + item.second.second, 52);
				line(file, str);
			}
		}
		else
		{
			for (const auto& item : variableList)
			{
				string str = "\tpublic static int " + item.second.first + " = " + IToS(item.first) + ";";
				appendWithAlign(str, "// " + item.second.second, 52);
				line(file, str);
			}
		}
		line(file, "");
	}
	uint memberCount = info.mHeader.mColumnDataList.size();
	mySet<string> listMemberSet;
	myVector<pair<string, string>> listMemberList;
	FOR_I(memberCount)
	{
		const ColumnData* member = info.mHeader.mColumnDataList[i];
		const string& name = member->mName;
		if (name == "ID")
		{
			continue;
		}

		// 不在客户端使用的则不定义成员变量
		if (member->mOwner != OWNER::CLIENT_ONLY && member->mOwner != OWNER::BOTH)
		{
			continue;
		}
		string typeName = convertToCSharpType(member->mType);
		// 列表类型的成员变量存储到单独的列表,因为需要分配内存
		bool isList = findString(typeName.c_str(), "List");
		if (isList)
		{
			listMemberList.push_back(make_pair(typeName, name));
			listMemberSet.insert(name);
		}
		string memberLine;
		if (!isList)
		{
			memberLine = "\tpublic " + typeName + " m" + name + ";";
		}
		else
		{
			memberLine = "\tpublic " + typeName + " m" + name + " = new();";
		}
		appendWithAlign(memberLine, "// " + info.mHeader.mColumnDataList[i]->mComment, 52);
		line(file, memberLine);
	}
	line(file, "\tpublic override bool read(SerializerRead reader)");
	line(file, "\t{");
	if (memberCount == 0)
	{
		line(file, "\t\treturn base.read(reader);");
	}
	else
	{
		line(file, "\t\tbool result = base.read(reader);");
		FOR_I(memberCount)
		{
			const ColumnData* memberInfo = info.mHeader.mColumnDataList[i];
			const string& name = memberInfo->mName;
			const string& type = memberInfo->mType;
			const string& enumRealType = memberInfo->mEnumRealType;
			if (name == "ID")
			{
				continue;
			}
			// 不在客户端使用的则不读取
			if (memberInfo->mOwner != OWNER::CLIENT_ONLY && memberInfo->mOwner != OWNER::BOTH)
			{
				continue;
			}
			const string typeName = convertToCSharpType(type);
			if (typeName == "string")
			{
				line(file, "\t\tresult = result && reader.readString(out m" + name + ");");
			}
			else if (listMemberSet.contains(name))
			{
				if (enumRealType == "byte")
				{
					line(file, "\t\tresult = result && reader.readEnumByteList(m" + name + ");");
				}
				else
				{
					line(file, "\t\tresult = result && reader.readList(m" + name + ");");
				}
			}
			else if (enumRealType == "byte")
			{
				line(file, "\t\tresult = result && reader.readEnumByte(out m" + name + ");");
			}
			else
			{
				line(file, "\t\tresult = result && reader.read(out m" + name + ");");
			}
		}
		line(file, "\t\treturn result;");
	}
	line(file, "\t}");

	line(file, "}");

	// 要生成参数的代码,必须要有对应ID的常量名
	if (customParamCount > 0 && variableNameIndex > 0)
	{
		bool hasCustomParam = false;
		mySet<int> paramIDList;
		for (const auto& row : info.mDataList)
		{
			if (!row[colNameList["Param0"]].empty())
			{
				hasCustomParam = true;
				break;
			}
		}
		if (hasCustomParam)
		{
			for (const auto& row : info.mDataList)
			{
				if (row[colNameList["Param0"]].empty())
				{
					continue;
				}
				const string paramClassName = "ED" + info.mHeader.mTableName + "_" + row[variableNameIndex];
				line(file, "");
				line(file, "public class " + paramClassName);
				line(file, "{");
				// 变量定义
				FOR_I(customParamCount)
				{
					string indexSuffix = IToS(i);
					const string& paramValue = row[colNameList["Param" + indexSuffix]];
					const string& paramType = row[colNameList["ParamType" + indexSuffix]];
					const string& paramName = row[colNameList["ParamName" + indexSuffix]];
					const string& paramComment = row[colNameList["ParamComment" + indexSuffix]];
					if (paramValue.empty())
					{
						break;
					}
					string csharpType = cppTypeToCSharpType(paramType);
					string lineContent;
					if (paramType == "Vector<int>")
					{
						myVector<int> values;
						SToIs(paramValue, values);
						lineContent = "\tpublic static " + csharpType + " m" + paramName + " = new() { " + IsToS(values.data(), values.size(), 0, ", ") + " };";
					}
					else if (paramType == "Vector<llong>")
					{
						myVector<llong> values;
						SToLLs(paramValue, values);
						lineContent = "\tpublic static " + csharpType + " m" + paramName + " = new() { " + LLsToS(values.data(), values.size(), 0, ", ") + " };";
					}
					else if (paramType == "Vector<float>")
					{
						myVector<float> values;
						SToFs(paramValue, values);
						string valueStr;
						FOR_VECTOR(values)
						{
							string str = FToS(values[i]);
							if ((int)str.find_first_of('.') < 0)
							{
								str += ".0f";
							}
							else
							{
								str += "f";
							}
							valueStr += str;
							if (i != values.size())
							{
								valueStr += ", ";
							}
						}
						lineContent = "\tpublic static " + csharpType + " m" + paramName + " = new() { " + valueStr + " };";
					}
					else if (paramType == "int" || paramType == "llong")
					{
						lineContent = "\tpublic static " + csharpType + " m" + paramName + " = " + paramValue + ";";
					}
					else if (paramType == "float")
					{
						string str = paramValue;
						if ((int)str.find_first_of('.') < 0)
						{
							str += ".0f";
						}
						else
						{
							str += "f";
						}
						lineContent = "\tpublic static " + csharpType + " m" + paramName + " = " + str + ";";
					}
					else
					{
						ERROR("不支持的参数类型:" + paramType + ", 表格:" + info.mHeader.mTableName + ", id:" + row[0]);
					}
					if (!lineContent.empty())
					{
						appendWithAlign(lineContent, "// " + paramComment, 52);
						line(file, lineContent);
					}
				}
				line(file, "}");
			}
		}
	}
	line(file, "// auto generate end", false);
	writeFile(dataFileHotFixPath + dataClassName + ".cs", file);
}

// ExcelTable.cs文件
void CodeExcel_Client::generateCSharpExcelTableFile(const CSVInfo& info, const string& tableFilePath)
{
	if (info.mHeader.mOwner == OWNER::SERVER_ONLY || info.mHeader.mOwner == OWNER::NONE)
	{
		return;
	}
	string tableClassName = "Excel" + info.mHeader.mTableName;
	string dataClassName = "ED" + info.mHeader.mTableName;
	myVector<string> insertLines;
	insertLines.push_back("\tprotected override void checkAllDataDefault()");
	insertLines.push_back("\t{");
	insertLines.push_back("\t\tforeach (" + dataClassName + " item in queryAll())");
	insertLines.push_back("\t\t{");
	
	int preCheckLines = insertLines.size();
	myMap<string, myVector<int>> linkLengthMap;
	for (const ColumnData* member : info.mHeader.mColumnDataList)
	{
		if (member->mOwner == OWNER::BOTH || member->mOwner == OWNER::CLIENT_ONLY)
		{
			const string& linkLength = member->mLinkLength;
			if (!linkLength.empty())
			{
				if (!linkLengthMap.contains(linkLength))
				{
					myVector<int> tmep{ member->mIndex };
					linkLengthMap.insert(linkLength, tmep);
				}
				else
				{
					linkLengthMap[linkLength].push_back(member->mIndex);
				}
			}
			const string& name = member->mName;
			const string& linkTable = member->mLinkTable;
			if (!linkTable.empty())
			{
				bool isLinkExcel = false;
				if (isFileExist(SQLitePath + linkTable + ".db"))
				{
					isLinkExcel = false;
				}
				else if (isFileExist(ExcelPath + linkTable + ".csv"))
				{
					isLinkExcel = true;
				}
				else
				{
					ERROR("找不到客户端索引的表格:" + linkTable + ", 当前表格:" + info.mHeader.mTableName + ", 字段名:" + name);
					continue;
				}
				const string tableVarPrefix = isLinkExcel ? "mExcel" : "mSQLite";
				if (member->mEnumRealType.empty())
				{
					insertLines.push_back("\t\t\t" + tableVarPrefix + linkTable + ".checkData(item.m" + name + ", item.mID, this);");
				}
				else
				{
					insertLines.push_back("\t\t\t" + tableVarPrefix + linkTable + ".checkData((int)item.m" + name + ", item.mID, this);");
				}
			}
			if (!member->mEnumRealType.empty())
			{
				insertLines.push_back("\t\t\tcheckEnum(item.m" + name + ", \"m" + name + "\", item.mID);");
			}
			if (!member->mFlag.empty())
			{
				string flagName = member->mFlag;
				string flagParam;
				int pos = -1;
				if (findString(member->mFlag.c_str(), ":", &pos))
				{
					flagName = member->mFlag.substr(0, pos);
					flagParam = member->mFlag.substr(pos + 1);
				}
				if (flagName == "Path")
				{
					insertLines.push_back("\t\t\tif (!item.m" + name + ".isEmpty())");
					insertLines.push_back("\t\t\t{");
					if (flagParam == "AllowSpace")
					{
						insertLines.push_back("\t\t\t\tcheckPath(item.m" + name + ", false);");
					}
					else if (flagParam == "NotAllowSpace")
					{
						insertLines.push_back("\t\t\t\tcheckPath(item.m" + name + ");");
					}
					insertLines.push_back("\t\t\t}");
				}
				else if (flagName == "ItemName")
				{
					if (startWith(member->mType, "Vector<"))
					{
						string tempListVarName = name;
						tempListVarName[0] = toLower(tempListVarName[0]);
						insertLines.push_back("\t\t\tusing var a" + name + " = new ListScope<string>(out var " + tempListVarName + ");");
						insertLines.push_back("\t\t\tfor (int i = 0; i < item.m" + flagParam + ".Count; ++i)");
						insertLines.push_back("\t\t\t{");
						insertLines.push_back("\t\t\t\t" + tempListVarName + ".add(mExcelItem.query(item.m" + flagParam + "[i])?.mName);");
						insertLines.push_back("\t\t\t}");
						insertLines.push_back("\t\t\tcheckStringValue(item.m" + name + ", " + tempListVarName + ", item.mID);");
					}
					else
					{
						insertLines.push_back("\t\t\tcheckStringValue(item.m" + name + ", mExcelItem.query(item.m" + flagParam + ", false)?.mName, item.mID);");
					}
				}
				else if (flagName == "PropertyName")
				{
					insertLines.push_back("\t\t\tcheckStringValue(item.m" + name + ", GD.PROPERTY_NAME.get(item.m" + flagParam + "), item.mID);");
				}
				else if (flagName == "EquipTypeName")
				{
					insertLines.push_back("\t\t\tcheckStringValue(item.m" + name + ", GD.EQUIP_TYPE_NAME.get(item.m" + flagParam + "), item.mID);");
				}
			}
		}
	}
	FOREACH(iterLink, linkLengthMap)
	{
		const auto& colList = iterLink->second;
		if (colList.size() > 1)
		{
			FOR_I(colList.size() - 1)
			{
				const string& name0 = info.mHeader.mColumnDataList[colList[i]]->mName;
				const string& name1 = info.mHeader.mColumnDataList[colList[i + 1]]->mName;
				insertLines.push_back("\t\t\tcheckListPair(item.m" + name0 + ", item.m" + name1 + ", item.mID);");
			}
		}
	}
	const int checkLines = insertLines.size() - preCheckLines;
	insertLines.push_back("\t\t}");
	insertLines.push_back("\t}");
	// 如果没有任何需要检查的,就只插入一个空函数
	if (checkLines == 0)
	{
		insertLines.clear();
		insertLines.push_back("\tprotected override void checkAllDataDefault() {}");
	}

	// ExcelTable.cs文件
	string csFileName = tableFilePath + tableClassName + ".cs";
	if (!isFileExist(csFileName))
	{
		string table;
		line(table, "using System;");
		line(table, "using System.Collections.Generic;");
		line(table, "using static GBR;");
		line(table, "");
		line(table, "public class " + tableClassName + " : ExcelTableT<" + dataClassName + ">");
		line(table, "{");
		line(table, "\t// auto generate start");
		for (const string& str : insertLines)
		{
			line(table, str);
		}
		line(table, "\t// auto generate end");
		line(table, "}", false);
		writeFile(csFileName, table);
	}
	else
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(csFileName, codeList, lineStart,
			[](const string& codeLine) { return endWith(codeLine, "// auto generate start"); },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }, false))
		{
			// 如果找不到就在倒数第二行插入
			int lineIndex = codeList.size() - 2;
			codeList.insert(++lineIndex, "\t// auto generate start");
			codeList.insert(++lineIndex, "\t// auto generate end");
			lineStart = lineIndex - 1;
		}
		for (const string& str : insertLines)
		{
			codeList.insert(++lineStart, str);
		}
		writeFile(csFileName, codeList);
	}
}

// ExcelRegister.cs文件
void CodeExcel_Client::generateCSharpExcelRegisteFileFile(const myVector<CSVInfo>& info, const string& fileHotFixPath)
{
	string hotFixfile;
	line(hotFixfile, "// auto generate start");
	line(hotFixfile, "using System;");
	line(hotFixfile, "using static GBR;");
	line(hotFixfile, "using static FrameBaseHotFix;");
	line(hotFixfile, "");
	line(hotFixfile, "public class ExcelRegister");
	line(hotFixfile, "{");
	line(hotFixfile, "\tpublic static void registeAll()");
	line(hotFixfile, "\t{");
	for (const CSVInfo& info : info)
	{
		if (info.mHeader.mOwner != OWNER::SERVER_ONLY && info.mHeader.mOwner != OWNER::NONE)
		{
			string lineStr = "\t\tregisteTable(out mExcel%s, typeof(ED%s), \"%s\");";
			replaceAll(lineStr, "%s", info.mHeader.mTableName);
			line(hotFixfile, lineStr);
		}
	}
	line(hotFixfile, "");
	line(hotFixfile, "\t\t// 进入热更以后,所有资源都处于可用状态");
	line(hotFixfile, "\t\tmExcelManager.resourceAvailable();");
	line(hotFixfile, "\t}");
	line(hotFixfile, "\t//------------------------------------------------------------------------------------------------------------------------------");
	line(hotFixfile, "\tprotected static void registeTable<T>(out T table, Type dataType, string tableName) where T : ExcelTable");
	line(hotFixfile, "\t{");
	line(hotFixfile, "\t\ttable = mExcelManager.registe(tableName, typeof(T), dataType) as T;");
	line(hotFixfile, "\t}");
	line(hotFixfile, "}");
	line(hotFixfile, "// auto generate end", false);
	writeFile(fileHotFixPath + "ExcelRegister.cs", hotFixfile);
}

// GameBaseHotFix.cs文件
void CodeExcel_Client::generateCSharpExcelDeclare(const myVector<CSVInfo>& info, const string& fileHotFixPath)
{
	myVector<string> insertLines;
	for (const CSVInfo& info : info)
	{
		if (info.mHeader.mOwner != OWNER::SERVER_ONLY && info.mHeader.mOwner != OWNER::NONE)
		{
			insertLines.push_back("\tpublic static Excel" + info.mHeader.mTableName + " mExcel" + info.mHeader.mTableName + ";");
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
		line(file, "// auto generate Excel start");
		for (const string& str : insertLines)
		{
			line(file, str);
		}
		line(file, "// auto generate Excel end");
		line(file, "}", false);
		writeFile(fileName, file);
	}
	else
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(fileName, codeList, lineStart,
			[](const string& codeLine) { return endWith(codeLine, "// auto generate Excel start"); },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate Excel end"); }, false))
		{
			// 如果找不到就在第一个{下一行插入
			FOR_VECTOR(codeList)
			{
				if (!codeList[i].empty() && codeList[i][0] == '{')
				{
					lineStart = i;
					codeList.insert(++lineStart, "\t// auto generate Excel start");
					codeList.insert(++lineStart, "\t// auto generate Excel end");
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

// 生成EDGlobal对应的cs代码
void CodeExcel_Client::generateCSharpGlobalConfig(const CSVInfo& globalConfig, const string& dataFilePath)
{
	// EDGlobal.cs
	string dataClassName = "ED" + globalConfig.mHeader.mTableName;
	string tableClassName = "Excel" + globalConfig.mHeader.mTableName;
	string dataFileName = dataFilePath + dataClassName + ".cs";
	myVector<string> insertLines0;
	insertLines0.push_back("\tprivate static bool mGlobalLoaded;");
	int paramTypeIndex = -1;
	int paramNameIndex = -1;
	int paramValueIndex = -1;
	int paramDescIndex = -1;
	FOR_VECTOR(globalConfig.mHeader.mColumnDataList)
	{
		const string& name = globalConfig.mHeader.mColumnDataList[i]->mName;
		if (name == "ParamType")
		{
			paramTypeIndex = i;
		}
		else if (name == "ParamName")
		{
			paramNameIndex = i;
		}
		else if (name == "ParamValue")
		{
			paramValueIndex = i;
		}
		else if (name == "ParamDesc")
		{
			paramDescIndex = i;
		}
	}
	// 静态变量的对象
	FOR_VECTOR(globalConfig.mDataList)
	{
		const auto& row = globalConfig.mDataList[i];
		const string& paramType = row[paramTypeIndex];
		const string& paramName = row[paramNameIndex];
		const string& paramDesc = row[paramDescIndex];
		if (paramType != "float" && paramType != "int" && paramType != "llong")
		{
			string temp = "\tprivate static " + cppTypeToCSharpType(paramType) + " _" + paramName + ";";
			appendWithAlign(temp, "// " + paramDesc, 52);
			insertLines0.push_back(temp);
		}
	}
	insertLines0.push_back("");

	// 常量
	FOR_VECTOR(globalConfig.mDataList)
	{
		const auto& row = globalConfig.mDataList[i];
		const string& paramType = row[paramTypeIndex];
		const string& paramName = row[paramNameIndex];
		const string& paramValue = row[paramValueIndex];
		const string& paramDesc = row[paramDescIndex];
		if (paramType == "float")
		{
			string floatStr = paramValue;
			if ((int)floatStr.find_first_of('.') < 0)
			{
				floatStr += ".0";
			}
			string temp = "\tpublic const " + paramType + " " + paramName + " = " + floatStr + "f;";
			appendWithAlign(temp, "// " + paramDesc, 52);
			insertLines0.push_back(temp);
		}
		else if (paramType == "int")
		{
			string temp = "\tpublic const " + paramType + " " + paramName + " = " + paramValue + ";";
			appendWithAlign(temp, "// " + paramDesc, 52);
			insertLines0.push_back(temp);
		}
		else if (paramType == "llong")
		{
			string temp = "\tpublic const " + cppTypeToCSharpType(paramType) + " " + paramName + " = " + paramValue + ";";
			appendWithAlign(temp, "// " + paramDesc, 52);
			insertLines0.push_back(temp);
		}
	}
	insertLines0.push_back("");

	// 静态变量的访问字段
	FOR_VECTOR(globalConfig.mDataList)
	{
		const auto& row = globalConfig.mDataList[i];
		const string& paramType = row[paramTypeIndex];
		const string& paramName = row[paramNameIndex];
		const string& paramDesc = row[paramDescIndex];
		if (paramType != "float" && paramType != "int" && paramType != "llong")
		{
			string temp = "\tpublic static " + cppTypeToCSharpType(paramType) + " " + paramName + "{ get { loadAllParam(); return _" + paramName + "; } }";
			appendWithAlign(temp, "// " + paramDesc, 52);
			insertLines0.push_back(temp);
		}
	}
	insertLines0.push_back("");

	myVector<string> insertLines1;
	insertLines1.push_back("\tpublic static void loadAllParam()");
	insertLines1.push_back("\t{");
	insertLines1.push_back("\t\tif (mGlobalLoaded)");
	insertLines1.push_back("\t\t{");
	insertLines1.push_back("\t\t\treturn;");
	insertLines1.push_back("\t\t}");
	insertLines1.push_back("\t\tmGlobalLoaded = true;");
	insertLines1.push_back("\t\tusing var a = new DicScope<string, string>(out var paramMap);");
	insertLines1.push_back("\t\tforeach (EDGlobal data in mTable.queryAll())");
	insertLines1.push_back("\t\t{");
	insertLines1.push_back("\t\t\tparamMap.add(data.mParamName, data.mParamValue.removeAllEmpty());");
	insertLines1.push_back("\t\t}");
	FOR_VECTOR(globalConfig.mDataList)
	{
		const auto& row = globalConfig.mDataList[i];
		const string& paramType = row[paramTypeIndex];
		const string& paramName = row[paramNameIndex];
		const string& paramValue = row[paramValueIndex];
		const string& paramDesc = row[paramDescIndex];
		if (paramType == "Vector2Int")
		{
			insertLines1.push_back("\t\t_" + paramName + " = paramMap[\"" + paramName + "\"].SToV2I();");
		}
		else if (paramType == "Vector2")
		{
			insertLines1.push_back("\t\t_" + paramName + " = paramMap[\"" + paramName + "\"].SToV2();");
		}
		else if (paramType == "Vector3")
		{
			insertLines1.push_back("\t\t_" + paramName + " = paramMap[\"" + paramName + "\"].SToV3();");
		}
		else if (paramType == "Vector3Int")
		{
			insertLines1.push_back("\t\t_" + paramName + " = paramMap[\"" + paramName + "\"].SToV3I();");
		}
		else if (paramType == "Vector<int>")
		{
			insertLines1.push_back("\t\t_" + paramName + " = paramMap[\"" + paramName + "\"].SToIs();");
		}
		else if (paramType == "Vector<float>")
		{
			insertLines1.push_back("\t\t_" + paramName + " = paramMap[\"" + paramName + "\"].SToFs();");
		}
		else if (paramType == "Vector<llong>")
		{
			insertLines1.push_back("\t\t_" + paramName + " = paramMap[\"" + paramName + "\"].SToLs();");
		}
	}
	insertLines1.push_back("\t}");

	myVector<string> codeList = openFile(dataFileName);
	int lineStart0 = 0;
	FOR_VECTOR(codeList)
	{
		if (codeList[i] == "{")
		{
			lineStart0 = i + 1;
			break;
		}
	}
	FOR_VECTOR(insertLines0)
	{
		codeList.insert(lineStart0++, insertLines0[i]);
	}
	// 将loadAllParam插入到类的最后
	int lineStart1 = 0;
	FOR_VECTOR_INVERSE(codeList)
	{
		if (codeList[i] == "}")
		{
			lineStart1 = i;
			break;
		}
	}
	FOR_VECTOR(insertLines1)
	{
		codeList.insert(lineStart1++, insertLines1[i]);
	}

	writeFile(dataFileName, codeListToString(codeList));
}

void CodeExcel_Client::generateCSharpBuff(const CSVInfo& config)
{
	int variableNameIndex = -1;
	FOR_VECTOR(config.mHeader.mColumnDataList)
	{
		const string& name = config.mHeader.mColumnDataList[i]->mName;
		if (name == "VariableName")
		{
			variableNameIndex = i;
			break;
		}
	}

	myVector<string> buffClassNameList;
	if (variableNameIndex >= 0)
	{
		for (const auto& item : config.mDataList)
		{
			buffClassNameList.push_back(item[variableNameIndex]);
		}
	}
	const string registerFilePath = ClientHotFixPath + "Character/CharacterComponent/StateMachine/StateRegister.cs";
	// 更新特定部分代码
	myVector<string> codeList;
	int lineStart = -1;
	if (!findCustomCode(registerFilePath, codeList, lineStart,
		[](const string& codeLine) { return endWith(codeLine, "// auto generate start"); },
		[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }))
	{
		return;
	}

	for (const string& name : buffClassNameList)
	{
		codeList.insert(++lineStart, "\t\tregisteState<" + name + ", " + name + "Param>(EDBuff." + name + "_ID);");
	}
	writeFile(registerFilePath, codeList);
}