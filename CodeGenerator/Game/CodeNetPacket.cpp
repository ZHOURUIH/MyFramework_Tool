#include "CodeNetPacket.h"
#include "CodeNetPacket_Client.h"
#include "CodeNetPacket_Server.h"

void CodeNetPacket::generate()
{
	print("正在生成网络消息");
	myVector<PacketStruct> structInfoList;
	myVector<PacketInfo> packetInfoList;
	parsePacketConfig(structInfoList, packetInfoList);
	if (packetInfoList.size() == 0)
	{
		return;
	}

	if (!cppGamePath.empty())
	{
		CodeNetPacket_Server::generateCpp(structInfoList, packetInfoList);
	}

	if (!ClientHotFixPath.empty())
	{
		CodeNetPacket_Client::generateCSharp(structInfoList, packetInfoList);
	}
	print("完成生成网络消息");
	print("");
}

void CodeNetPacket::generateVirtualClient()
{
	if (VirtualClientSocketPath.empty())
	{
		LOG("未配置虚拟客户端项目路径");
		return;
	}
	print("正在生成虚拟客户端网络消息");
	myVector<PacketStruct> structInfoList;
	myVector<PacketInfo> packetInfoList;
	parsePacketConfig(structInfoList, packetInfoList);

	CodeNetPacket_Client::generateCSharpVirtualClient(structInfoList, packetInfoList);
	print("完成生成虚拟客户端网络消息");
	print("");
}

void CodeNetPacket::parsePacketConfig(myVector<PacketStruct>& structInfoList, myVector<PacketInfo>& packetInfoList)
{
	// 解析模板文件
	myVector<string> csLines = openFile("PacketCS.txt");
	myVector<string> scLines = openFile("PacketSC.txt");
	myVector<string> structLines = openFile("PacketStruct.txt");
	if (csLines.size() == 0)
	{
		LOG("没有协议文件PacketCS.txt");
		return;
	}
	if (scLines.size() == 0)
	{
		LOG("没有协议文件PacketSC.txt");
		return;
	}

	// 解析结构体定义
	bool structStart = false;
	myVector<PacketMember> tempStructMemberList;
	int tempStructNameLine = 0;
	FOR_VECTOR(structLines)
	{
		string line = structLines[i];
		// 忽略注释
		if (startWith(line, "//"))
		{
			continue;
		}
		// 如果后面插有注释,则去除
		int pos = -1;
		if (findString(line.c_str(), "//", &pos))
		{
			line = line.substr(0, pos);
		}
		// 去除所有制表符,分号
		removeAll(line, '\t', ';');
		// 没有成员变量的消息包
		if (line == "{}")
		{
			PacketStruct info;
			parseStructName(structLines[i - 1], info);
			info.mComment = structLines[i - 2];
			structInfoList.push_back(info);
			continue;
		}
		// 成员变量列表起始
		if (line == "{")
		{
			structStart = true;
			tempStructNameLine = i - 1;
			tempStructMemberList.clear();
			continue;
		}
		// 成员变量列表结束
		if (line == "}")
		{
			if (!structStart)
			{
				ERROR("未找到前一个匹配的{, PacketStruct,前5行内容:");
				int printStartLine = (int)i - 5;
				clampMin(printStartLine, 0);
				for (int j = printStartLine; j <= (int)i; ++j)
				{
					ERROR(structLines[j]);
				}
			}
			PacketStruct info;
			parseStructName(structLines[tempStructNameLine], info);
			info.mMemberList = tempStructMemberList;
			info.mComment = structLines[tempStructNameLine - 1];
			structInfoList.push_back(info);
			structStart = false;
			tempStructMemberList.clear();
			tempStructNameLine = -1;
			continue;
		}
		if (structStart)
		{
			PacketMember member = parseMemberLine(line);
			member.mIndex = tempStructMemberList.size();
			tempStructMemberList.push_back(member);
			if (tempStructMemberList.size() >= 64 && tempStructMemberList[tempStructMemberList.size() - 1].mOptional)
			{
				ERROR("仅支持前64个字段允许设置为可选字段,包名:" + structLines[tempStructNameLine]);
			}
		}
	}

	// 解析消息定义
	myVector<string> allLines;
	allLines.addRange(csLines);
	allLines.addRange(scLines);
	bool packetStart = false;
	myVector<PacketMember> tempMemberList;
	int tempPacketNameLine = 0;
	FOR_VECTOR(allLines)
	{
		string line = allLines[i];
		// 忽略注释
		if (startWith(line, "//"))
		{
			continue;
		}
		// 去除所有制表符,分号
		removeAll(line, '\t', ';');
		// 没有成员变量的消息包
		if (line == "{}")
		{
			PacketInfo info;
			parsePacketName(allLines[i - 1], info);
			info.mComment = allLines[i - 2];
			packetInfoList.push_back(info);
			continue;
		}
		// 成员变量列表起始
		if (line == "{")
		{
			packetStart = true;
			tempPacketNameLine = i - 1;
			tempMemberList.clear();
			continue;
		}
		// 成员变量列表结束
		if (line == "}")
		{
			if (!packetStart)
			{
				ERROR("未找到前一个匹配的{, NetPacket,前5行内容:");
				int printStartLine = (int)i - 5;
				clampMin(printStartLine, 0);
				for (int j = printStartLine; j <= (int)i; ++j)
				{
					ERROR(allLines[j]);
				}
			}
			PacketInfo info;
			parsePacketName(allLines[tempPacketNameLine], info);
			info.mMemberList = tempMemberList;
			info.mComment = allLines[tempPacketNameLine - 1];
			packetInfoList.push_back(info);
			packetStart = false;
			tempMemberList.clear();
			tempPacketNameLine = -1;
			continue;
		}
		if (packetStart)
		{
			PacketMember member = parseMemberLine(line);
			member.mIndex = tempMemberList.size();
			tempMemberList.push_back(member);
			if (tempMemberList.size() >= 64 && tempMemberList[tempMemberList.size() - 1].mOptional)
			{
				ERROR("仅支持前64个字段允许设置为可选字段,包名:" + allLines[tempPacketNameLine]);
			}
		}
	}
}

string CodeNetPacket::generatePacketVersion(const myVector<PacketInfo>& packetList, const myVector<PacketStruct>& structInfoList)
{
	string allParamString;
	for (const PacketInfo& packetInfo : packetList)
	{
		allParamString += packetInfo.mPacketName;
		for (const PacketMember& member : packetInfo.mMemberList)
		{
			allParamString += member.mTypeName + member.mMemberName;
		}
	}
	for (const PacketStruct& structInfo : structInfoList)
	{
		allParamString += structInfo.mStructName;
		for (const PacketMember& member : structInfo.mMemberList)
		{
			allParamString += member.mTypeName + member.mMemberName;
		}
	}
	return generateStringMD5(allParamString);
}

bool CodeNetPacket::isSameType(const string& sourceType, const string& curType)
{
	// string和bool类型不合并
	if (sourceType == "string" || sourceType == "bool" || curType == "string" || curType == "bool")
	{
		return false;
	}
	if (sourceType == curType)
	{
		return true;
	}
	if (sourceType == "float")
	{
		return curType == "Vector2" || curType == "Vector3" || curType == "Vector4";
	}
	if (sourceType == "ushort")
	{
		return curType == "Vector2UShort";
	}
	if (sourceType == "int")
	{
		return curType == "Vector2Int" || curType == "Vector3Int";
	}
	if (sourceType == "uint")
	{
		return curType == "Vector2UInt" || curType == "Vector3UInt";
	}
	return false;
}

string CodeNetPacket::toPODType(const string& type)
{
	if (type == "Vector2" || type == "Vector3" || type == "Vector4")
	{
		return "float";
	}
	if (type == "Vector2UShort")
	{
		return "ushort";
	}
	if (type == "Vector2Int" || type == "Vector3Int")
	{
		return "int";
	}
	if (type == "Vector2UInt" || type == "Vector3UInt")
	{
		return "uint";
	}
	return type;
}

bool CodeNetPacket::isCustomStructType(const string& type)
{
	return startWith(type, "NetStruct");
}

void CodeNetPacket::generateMemberGroup(const myVector<PacketMember>& memberList, myVector<myVector<PacketMember>>& memberNameList)
{
	myVector<PacketMember> sameTypeMemberList;
	FOR_VECTOR(memberList)
	{
		const PacketMember& item = memberList[i];
		if (i > 0)
		{
			const PacketMember& lastItem = memberList[i - 1];
			// 如果上一个成员变量与当前的类型不一致,或者当前是一个optional变量,则将之前的变量写入,自定义的结构体类型的变量也不会归到一组
			if (item.mOptional || lastItem.mOptional || !isSameType(toPODType(item.mTypeName), toPODType(lastItem.mTypeName)) || isCustomStructType(item.mTypeName))
			{
				memberNameList.push_back(sameTypeMemberList);
				sameTypeMemberList.clear();
			}
			// 继续向后遍历
			sameTypeMemberList.push_back(item);
		}
		// 特殊判断第0个元素,因为没有上一个可以做比较
		else
		{
			if (item.mOptional)
			{
				memberNameList.push_back(myVector<PacketMember>{ item });
			}
			else
			{
				sameTypeMemberList.push_back(item);
			}
		}
	}
	if (sameTypeMemberList.size() > 0)
	{
		memberNameList.push_back(sameTypeMemberList);
	}
}