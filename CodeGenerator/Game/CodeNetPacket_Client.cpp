#include "CodeNetPacket_Client.h"

void CodeNetPacket_Client::generateCSharp(const myVector<PacketStruct>& structInfoList, const myVector<PacketInfo>& packetInfoList)
{
	string csharpCSHotfixPath = ClientHotFixPath + "Socket/ClientServer/";
	string csharpSCHotfixPath = ClientHotFixPath + "Socket/ServerClient/";
	string csharpStructHotfixPath = ClientHotFixPath + "Socket/Struct/";
	string csharpPacketDefinePath = ClientHotFixPath + "Socket/";

	myVector<string> hotfixList;
	for (const PacketInfo& packetInfo : packetInfoList)
	{
		hotfixList.push_back(packetInfo.mPacketName);
	}
	myVector<string> hotfixStructList;
	for (const PacketStruct& structInfo : structInfoList)
	{
		hotfixStructList.push_back(structInfo.mStructName);
	}
	// 删除无用的消息
	// c# CS热更
	for (const string& file : findFiles(csharpCSHotfixPath, ".cs"))
	{
		if (!hotfixList.contains(getFileNameNoSuffix(file, true)))
		{
			deleteFile(file);
		}
	}
	// c# SC热更
	for (const string& file : findFiles(csharpSCHotfixPath, ".cs"))
	{
		if (!hotfixList.contains(getFileNameNoSuffix(file, true)))
		{
			deleteFile(file);
		}
	}
	// 删除无用的结构体代码
	for (const string& file : findFiles(csharpStructHotfixPath, ".cs"))
	{
		if (!hotfixStructList.contains(getFileNameNoSuffix(file, true)))
		{
			deleteFile(file);
		}
	}

	// 生成cs代码
	for (const PacketInfo& packetInfo : packetInfoList)
	{
		generateCSharpPacketFile(packetInfo, csharpCSHotfixPath, csharpSCHotfixPath);
	}
	generateCSharpPacketDefineFile(packetInfoList, csharpPacketDefinePath);
	generateCSharpPacketRegisteFile(packetInfoList, structInfoList, csharpPacketDefinePath);

	// 生成结构体代码
	for (const PacketStruct& item : structInfoList)
	{
		generateCSharpStruct(item, csharpStructHotfixPath);
	}
}

void CodeNetPacket_Client::generateCSharpVirtualClient(const myVector<PacketStruct>& structInfoList, const myVector<PacketInfo>& packetInfoList)
{
	string csharpCSGamePath = VirtualClientSocketPath + "ClientServer/";
	string csharpSCGamePath = VirtualClientSocketPath + "ServerClient/";
	string csharpStructGamePath = VirtualClientSocketPath + "Struct/";
	string csharpPacketDefinePath = VirtualClientSocketPath;

	myVector<string> packetNameList;
	for (const PacketInfo& packetInfo : packetInfoList)
	{
		packetNameList.push_back(packetInfo.mPacketName);
	}
	myVector<string> structNameList;
	for (const PacketStruct& structInfo : structInfoList)
	{
		structNameList.push_back(structInfo.mStructName);
	}

	// 删除无用的消息
	// CS
	for (const string& file : findFiles(csharpCSGamePath, ".cs"))
	{
		if (!packetNameList.contains(getFileNameNoSuffix(file, true)))
		{
			deleteFile(file);
			deleteFile(file + ".meta");
		}
	}
	// SC
	for (const string& file : findFiles(csharpSCGamePath, ".cs"))
	{
		if (!packetNameList.contains(getFileNameNoSuffix(file, true)))
		{
			deleteFile(file);
			deleteFile(file + ".meta");
		}
	}
	// 删除无用的结构体代码
	for (const string& file : findFiles(csharpStructGamePath, ".cs"))
	{
		if (!structNameList.contains(getFileNameNoSuffix(file, true)))
		{
			deleteFile(file);
		}
	}

	// 生成cs代码
	for (const PacketInfo& packetInfo : packetInfoList)
	{
		generateCSharpPacketFile(packetInfo, csharpCSGamePath, csharpSCGamePath);
	}
	generateCSharpPacketDefineFile(packetInfoList, csharpPacketDefinePath);
	generateCSharpPacketRegisteFile(packetInfoList, structInfoList, csharpPacketDefinePath);

	// 生成结构体代码
	for (const PacketStruct& item : structInfoList)
	{
		generateCSharpStruct(item, csharpStructGamePath);
	}
}

// PacketDefine.cs文件
void CodeNetPacket_Client::generateCSharpPacketDefineFile(const myVector<PacketInfo>& packetList, const string& filePath)
{
	string str;
	line(str, "// auto generate start");
	line(str, "using System;");
	line(str, "");
	line(str, "public class PACKET_TYPE");
	line(str, "{");
	line(str, "\tpublic static ushort MIN = 0;");
	line(str, "");
	int csPacketNumber = 10000;
	for (const auto& item : packetList)
	{
		if (startWith(item.mPacketName, "CS"))
		{
			line(str, "\tpublic static ushort " + packetNameToUpper(item.mPacketName) + " = " + IToS(++csPacketNumber) + ";");
		}
	}
	line(str, "");
	int scPacketNumber = 20000;
	for (const auto& item : packetList)
	{
		if (startWith(item.mPacketName, "SC"))
		{
			line(str, "\tpublic static ushort " + packetNameToUpper(item.mPacketName) + " = " + IToS(++scPacketNumber) + ";");
		}
	}
	line(str, "}");
	line(str, "// auto generate end", false);
	writeFile(filePath + "PacketDefine.cs", str);
}

// PacketRegister.cs文件
void CodeNetPacket_Client::generateCSharpPacketRegisteFile(const myVector<PacketInfo>& packetList, const myVector<PacketStruct>& structInfoList, const string& filePath)
{
	string str;
	line(str, "// auto generate start");
	line(str, "using System;");
	line(str, "using static FrameBaseHotFix;");
	line(str, "");
	line(str, "public class PacketRegister");
	line(str, "{");
	line(str, "\tpublic static string PACKET_VERSION = \"" + generatePacketVersion(packetList, structInfoList) + "\";");
	line(str, "\tpublic static void registeAll()");
	line(str, "\t{");
	myVector<PacketInfo> udpCSList;
	uint packetCount = packetList.size();
	FOR_I(packetCount)
	{
		if (!startWith(packetList[i].mPacketName, "CS"))
		{
			continue;
		}
		line(str, "\t\tregistePacket<" + packetList[i].mPacketName + ">(PACKET_TYPE." + packetNameToUpper(packetList[i].mPacketName) + ");");
		if (packetList[i].mUDP)
		{
			udpCSList.push_back(packetList[i]);
		}
	}
	line(str, "");
	myVector<PacketInfo> udpSCList;
	FOR_I(packetCount)
	{
		if (!startWith(packetList[i].mPacketName, "SC"))
		{
			continue;
		}
		line(str, "\t\tregistePacket<" + packetList[i].mPacketName + ">(PACKET_TYPE." + packetNameToUpper(packetList[i].mPacketName) + ");");
		if (packetList[i].mUDP)
		{
			udpSCList.push_back(packetList[i]);
		}
	}
	if (udpCSList.size() > 0)
	{
		line(str, "");
		FOR_VECTOR(udpCSList)
		{
			line(str, "\t\tregisteUDP(PACKET_TYPE." + packetNameToUpper(udpCSList[i].mPacketName) + ", \"" + udpCSList[i].mPacketName + "\");");
		}
	}
	if (udpSCList.size() > 0)
	{
		line(str, "");
		FOR_VECTOR(udpSCList)
		{
			line(str, "\t\tregisteUDP(PACKET_TYPE." + packetNameToUpper(udpSCList[i].mPacketName) + ", \"" + udpSCList[i].mPacketName + "\");");
		}
	}
	line(str, "\t}");
	line(str, "\tprotected static void registePacket<T>(ushort type) where T : NetPacketBit");
	line(str, "\t{");
	line(str, "\t\tmNetPacketTypeManager.registePacket(typeof(T), type);");
	line(str, "\t}");
	line(str, "\tprotected static void registeUDP(ushort type, string packetName)");
	line(str, "\t{");
	line(str, "\t\tmNetPacketTypeManager.registeUDPPacketName(type, packetName);");
	line(str, "\t}");
	line(str, "}");
	line(str, "// auto generate end", false);
	writeFile(filePath + "PacketRegister.cs", str);
}

// CSPacket.cs和SCPacket.cs文件
void CodeNetPacket_Client::generateCSharpPacketFile(const PacketInfo& packetInfo, const string& csFileHotfixPath, const string& scFileHotfixPath)
{
	string packetName = packetInfo.mPacketName;
	if (!startWith(packetName, "CS") && !startWith(packetName, "SC"))
	{
		return;
	}

	myVector<string> usingList;
	myVector<string> customList;
	string fullPath;
	if (startWith(packetName, "CS"))
	{
		fullPath = csFileHotfixPath + packetName + ".cs";
	}
	else if (startWith(packetName, "SC"))
	{
		fullPath = scFileHotfixPath + packetName + ".cs";
	}

	myVector<myVector<PacketMember>> memberGroupList;
	generateMemberGroup(packetInfo.mMemberList, memberGroupList);

	myVector<string> generateCodes;
	generateCodes.push_back(packetInfo.mComment);
	generateCodes.push_back("public class " + packetName + " : NetPacketBit");
	generateCodes.push_back("{");
	for (const PacketMember& item : packetInfo.mMemberList)
	{
		generateCodes.push_back("\t" + cSharpMemberDeclareString(item));
	}
	if (packetInfo.mMemberList.size() > 0)
	{
		// init
		generateCodes.push_back("\tpublic " + packetName + "()");
		generateCodes.push_back("\t{");
		for (const PacketMember& item : packetInfo.mMemberList)
		{
			generateCodes.push_back("\t\taddParam(" + item.mMemberName + ", " + (item.mOptional ? "true" : "false") + ");");
		}
		generateCodes.push_back("\t}");

		// get
		if (startWith(packetName, "CS"))
		{
			generateCodes.push_back("\tpublic static " + packetName + " get() { return PACKET<" + packetName + ">(); }");
		}

		// read
		generateCodes.push_back("\tpublic override bool read(SerializerBitRead reader, bool needReadSign, ulong fieldFlag)");
		generateCodes.push_back("\t{");
		generateCodes.push_back("\t\tbool success = true;");
		FOR_VECTOR(memberGroupList)
		{
			const myVector<PacketMember>& memberGroup = memberGroupList[i];
			if (memberGroup.size() == 1)
			{
				const PacketMember& item = memberGroup[0];
				// 可选字段需要特别判断一下
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (hasBit(fieldFlag, " + IToS(item.mIndex) + "))");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tsuccess = success && " + item.mMemberName + ".read(reader, needReadSign);");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tsuccess = success && " + item.mMemberName + ".read(reader, needReadSign);");
				}
			}
			else
			{
				myVector<string> nameList;
				string groupTypeName = expandMembersInGroupCSharp(memberGroup, nameList, false);
				myVector<string> list = multiMemberReadLineCSharp(nameList, groupTypeName, true);
				FOR_VECTOR(list)
				{
					generateCodes.push_back("\t\t" + list[i]);
				}
			}
		}
		generateCodes.push_back("\t\treturn success;");
		generateCodes.push_back("\t}");

		// write
		generateCodes.push_back("\tpublic override void write(SerializerBitWrite writer, bool needWriteSign, out ulong fieldFlag)");
		generateCodes.push_back("\t{");
		generateCodes.push_back("\t\tbase.write(writer, needWriteSign, out fieldFlag);");
		FOR_VECTOR(memberGroupList)
		{
			const myVector<PacketMember>& memberGroup = memberGroupList[i];
			if (memberGroup.size() == 1)
			{
				const PacketMember& item = memberGroup[0];
				// 可选字段需要特别判断一下
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValid)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tsetBitOne(ref fieldFlag, " + IToS(item.mIndex) + ");");
					generateCodes.push_back("\t\t\t" + item.mMemberName + ".write(writer, needWriteSign);");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\t" + item.mMemberName + ".write(writer, needWriteSign);");
				}
			}
			else
			{
				myVector<string> nameList;
				string groupTypeName = expandMembersInGroupCSharp(memberGroup, nameList, true);
				myVector<string> list = multiMemberWriteLineCSharp(nameList, groupTypeName, true);
				FOR_VECTOR(list)
				{
					generateCodes.push_back("\t\t" + list[i]);
				}
			}
		}
		generateCodes.push_back("\t}");

		// hasSign
		generateCodes.push_back("\tprotected override bool generateHasSignInternal()");
		generateCodes.push_back("\t{");
		for (const PacketMember& item : packetInfo.mMemberList)
		{
			string csType = cppTypeToCSharpType(item.mTypeName);
			// 无符号的基础数据类型
			if (csType == "string" || csType == "bool" || csType == "byte" || csType == "ushort" || csType == "uint" || csType == "ulong" || csType == "Vector2UInt" || csType == "Vector2UShort")
			{
				// 无符号的不做判断
			}
			// 有符号的基础数据类型
			else if (csType == "sbyte" || csType == "short" || csType == "int" || csType == "long" || csType == "float" || csType == "double")
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValid && " + item.mMemberName + ".mValue < 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValue < 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
			}
			else if (csType == "Vector2Int" || csType == "Vector2Short" || csType == "Vector2")
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValid && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0))");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
			}
			else if (csType == "Vector3" || csType == "Vector3Int")
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValid && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0))");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
			}
			else if (csType == "Vector4")
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValid && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0 || " + item.mMemberName + ".w < 0))");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0 || " + item.mMemberName + ".w < 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
			}
			// 无符号的基础数据类型的列表
			else if (csType == "List<string>" || csType == "List<byte>" || csType == "List<ushort>" || csType == "List<uint>" || csType == "List<ulong>")
			{
				// 无符号的不做判断
			}
			// 有符号的基础数据类型的列表
			else if (csType == "List<sbyte>" || csType == "List<short>" || csType == "List<int>" || csType == "List<long>" || csType == "List<float>" || csType == "List<double>")
			{
				string elementType = getElementTypeCS(csType);
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValid)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tforeach (" + elementType + " item in " + item.mMemberName + ")");
					generateCodes.push_back("\t\t\t{");
					generateCodes.push_back("\t\t\t\tif (item < 0)");
					generateCodes.push_back("\t\t\t\t{");
					generateCodes.push_back("\t\t\t\t\treturn true;");
					generateCodes.push_back("\t\t\t\t}");
					generateCodes.push_back("\t\t\t}");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tforeach (" + elementType + " item in " + item.mMemberName + ")");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tif (item < 0)");
					generateCodes.push_back("\t\t\t{");
					generateCodes.push_back("\t\t\t\treturn true;");
					generateCodes.push_back("\t\t\t}");
					generateCodes.push_back("\t\t}");
				}
			}
			// 自定义数据结构的列表
			else if (startWith(csType, "List<"))
			{
				string elementType = getElementTypeCS(csType);
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValid)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tforeach (" + elementType + " item in " + item.mMemberName + ")");
					generateCodes.push_back("\t\t\t{");
					generateCodes.push_back("\t\t\t\tif (item.hasSign())");
					generateCodes.push_back("\t\t\t\t{");
					generateCodes.push_back("\t\t\t\t\treturn true;");
					generateCodes.push_back("\t\t\t\t}");
					generateCodes.push_back("\t\t\t}");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tforeach (" + elementType + " item in " + item.mMemberName + ")");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tif (item.hasSign())");
					generateCodes.push_back("\t\t\t{");
					generateCodes.push_back("\t\t\t\treturn true;");
					generateCodes.push_back("\t\t\t}");
					generateCodes.push_back("\t\t}");
				}
			}
			// 自定义数据结构
			else
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".mValid && " + item.mMemberName + ".hasSign())");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + ".hasSign())");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
			}
		}
		generateCodes.push_back("\t\treturn false;");
		generateCodes.push_back("\t}");
	}
	else
	{
		// get
		if (startWith(packetName, "CS"))
		{
			generateCodes.push_back("\tpublic static " + packetName + " get() { return PACKET<" + packetName + ">(); }");
		}
	}
	if (isFileExist(fullPath))
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(fullPath, codeList, lineStart,
			[](const string& codeLine) { return codeLine == "// auto generate start"; },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }))
		{
			return;
		}
		for (const string& line : generateCodes)
		{
			codeList.insert(++lineStart, line);
		}
		writeFile(fullPath, codeList);
	}
	else
	{
		myVector<string> codeList;
		codeList.push_back("using static FrameUtility;");
		codeList.push_back("using static GBR;");
		codeList.push_back("using static GU;");
		codeList.push_back("");
		codeList.push_back("// auto generate start");
		codeList.addRange(generateCodes);
		codeList.push_back("\t// auto generate end");
		if (startWith(packetName, "SC"))
		{
			codeList.push_back("\tpublic override void execute()");
			codeList.push_back("\t{}");
		}
		else if (startWith(packetName, "CS"))
		{
			codeList.push_back("\tpublic static void send()");
			codeList.push_back("\t{");
			codeList.push_back("\t\tsendPacket(get());");
			codeList.push_back("\t}");
		}
		codeList.push_back("}");
		writeFile(fullPath, codeList);
	}
}

void CodeNetPacket_Client::generateCSharpStruct(const PacketStruct& structInfo, const string& hotFixPath)
{
	bool hasOptional = false;
	for (const PacketMember& item : structInfo.mMemberList)
	{
		hasOptional |= item.mOptional;
	}
	myVector<string> codeList;
	codeList.push_back("using System;");
	codeList.push_back("using UnityEngine;");
	codeList.push_back("using System.Collections;");
	codeList.push_back("using System.Collections.Generic;");
	codeList.push_back("using static FrameUtility;");
	if (hasOptional)
	{
		codeList.push_back("using static BinaryUtility;");
	}
	codeList.push_back("");
	codeList.push_back("public class " + structInfo.mStructName + " : NetStructBit");
	codeList.push_back("{");
	for (const PacketMember& item : structInfo.mMemberList)
	{
		codeList.push_back("\t" + cSharpMemberDeclareString(item));
	}

	// 构造
	codeList.push_back("\tpublic " + structInfo.mStructName + "()");
	codeList.push_back("\t{");
	for (const PacketMember& item : structInfo.mMemberList)
	{
		codeList.push_back("\t\taddParam(" + item.mMemberName + ", " + (item.mOptional ? "true" : "false") + ");");
	}
	codeList.push_back("\t}");
	
	myVector<myVector<PacketMember>> memberGroupList;
	generateMemberGroup(structInfo.mMemberList, memberGroupList);

	// readInternal
	codeList.push_back("\tprotected override bool readInternal(ulong fieldFlag, SerializerBitRead reader, bool needReadSign)");
	codeList.push_back("\t{");
	codeList.push_back("\t\tbool success = true;");
	FOR_VECTOR(memberGroupList)
	{
		const myVector<PacketMember>& memberGroup = memberGroupList[i];
		if (memberGroup.size() == 1)
		{
			const PacketMember& member = memberGroup[0];
			if (member.mOptional)
			{
				codeList.push_back("\t\tif (" + member.mMemberName + ".mValid = hasBit(fieldFlag, " + IToS(member.mIndex) + "))");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\t" + singleMemberReadLineCSharp(member.mMemberName, member.mTypeName));
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\t" + singleMemberReadLineCSharp(member.mMemberName, member.mTypeName));
			}
		}
		else
		{
			myVector<string> nameList;
			string groupTypeName = expandMembersInGroupCSharp(memberGroup, nameList, false);
			myVector<string> list = multiMemberReadLineCSharp(nameList, groupTypeName, false);
			FOR_VECTOR(list)
			{
				codeList.push_back("\t\t" + list[i]);
			}
		}
	}
	codeList.push_back("\t\treturn success;");
	codeList.push_back("\t}");

	// write
	codeList.push_back("\tpublic override void write(SerializerBitWrite writer, bool needWriteSign)");
	codeList.push_back("\t{");
	codeList.push_back("\t\tbase.write(writer, needWriteSign);");
	FOR_VECTOR(memberGroupList)
	{
		const myVector<PacketMember>& memberGroup = memberGroupList[i];
		if (memberGroup.size() == 1)
		{
			const PacketMember& member = memberGroup[0];
			if (member.mOptional)
			{
				codeList.push_back("\t\tif (" + member.mMemberName + ".mValid)");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\t" + singleMemberWriteLineCSharp(member.mMemberName, member.mTypeName));
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\t" + singleMemberWriteLineCSharp(member.mMemberName, member.mTypeName));
			}
		}
		else
		{
			myVector<string> nameList;
			string groupTypeName = expandMembersInGroupCSharp(memberGroup, nameList, true);
			myVector<string> list = multiMemberWriteLineCSharp(nameList, groupTypeName, false);
			FOR_VECTOR(list)
			{
				codeList.push_back("\t\t" + list[i]);
			}
		}
	}
	codeList.push_back("\t}");

	// hasSign
	codeList.push_back("\tpublic override bool hasSign()");
	codeList.push_back("\t{");
	for (const PacketMember& item : structInfo.mMemberList)
	{
		string csType = cppTypeToCSharpType(item.mTypeName);
		// 无符号的基础数据类型
		if (csType == "string" || csType == "bool" || csType == "byte" || csType == "ushort" || csType == "uint" || csType == "ulong" || csType == "Vector2UInt" || csType == "Vector2UShort")
		{
			// 无符号的不做判断
		}
		// 有符号的基础数据类型
		else if (csType == "sbyte" || csType == "short" || csType == "int" || csType == "long" || csType == "float" || csType == "double")
		{
			if (item.mOptional)
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".mValid && " + item.mMemberName + ".mValue < 0)");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".mValue < 0)");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
		}
		else if (csType == "Vector2Int" || csType == "Vector2Short" || csType == "Vector2")
		{
			if (item.mOptional)
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".mValid && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0))");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0)");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
		}
		else if (csType == "Vector3" || csType == "Vector3Int")
		{
			if (item.mOptional)
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".mValid && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0))");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0)");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
		}
		else if (csType == "Vector4")
		{
			if (item.mOptional)
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".mValid && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0 || " + item.mMemberName + ".w < 0))");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0 || " + item.mMemberName + ".w < 0)");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
		}
		// 无符号的基础数据类型的列表
		else if (csType == "List<string>" || csType == "List<byte>" || csType == "List<ushort>" || csType == "List<uint>" || csType == "List<ulong>")
		{
			// 无符号的不做判断
		}
		// 有符号的基础数据类型的列表
		else if (csType == "List<sbyte>" || csType == "List<short>" || csType == "List<int>" || csType == "List<long>" || csType == "List<float>" || csType == "List<double>")
		{
			string elementType = getElementTypeCS(csType);
			if (item.mOptional)
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".mValid)");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\tforeach (" + elementType + " item in " + item.mMemberName + ")");
				codeList.push_back("\t\t\t{");
				codeList.push_back("\t\t\t\tif (item < 0)");
				codeList.push_back("\t\t\t\t{");
				codeList.push_back("\t\t\t\t\treturn true;");
				codeList.push_back("\t\t\t\t}");
				codeList.push_back("\t\t\t}");
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\tforeach (" + elementType + " item in " + item.mMemberName + ")");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\tif (item < 0)");
				codeList.push_back("\t\t\t{");
				codeList.push_back("\t\t\t\treturn true;");
				codeList.push_back("\t\t\t}");
				codeList.push_back("\t\t}");
			}
		}
		// 自定义数据类型的列表
		else if (startWith(csType, "List<"))
		{
			string elementType = getElementTypeCS(csType);
			if (item.mOptional)
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".mValid)");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\tforeach (" + elementType + " item in " + item.mMemberName + ")");
				codeList.push_back("\t\t\t{");
				codeList.push_back("\t\t\t\tif (item.hasSign())");
				codeList.push_back("\t\t\t\t{");
				codeList.push_back("\t\t\t\t\treturn true;");
				codeList.push_back("\t\t\t\t}");
				codeList.push_back("\t\t\t}");
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\tforeach (" + elementType + " item in " + item.mMemberName + ")");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\tif (item.hasSign())");
				codeList.push_back("\t\t\t{");
				codeList.push_back("\t\t\t\treturn true;");
				codeList.push_back("\t\t\t}");
				codeList.push_back("\t\t}");
			}
		}
		// 自定义数据类型
		else
		{
			if (item.mOptional)
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".mValid && " + item.mMemberName + ".hasSign())");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
			else
			{
				codeList.push_back("\t\tif (" + item.mMemberName + ".hasSign())");
				codeList.push_back("\t\t{");
				codeList.push_back("\t\t\treturn true;");
				codeList.push_back("\t\t}");
			}
		}
	}
	codeList.push_back("\t\treturn false;");
	codeList.push_back("\t}");

	// resetProperty
	codeList.push_back("\tpublic override void resetProperty()");
	codeList.push_back("\t{");
	codeList.push_back("\t\tbase.resetProperty();");
	for (const PacketMember& item : structInfo.mMemberList)
	{
		codeList.push_back("\t\t" + item.mMemberName + ".resetProperty();");
	}
	codeList.push_back("\t}");

	codeList.push_back("}");
	codeList.push_back("");

	// StructList
	codeList.push_back("public class " + structInfo.mStructName + "_List : SerializableBit");
	codeList.push_back("{");
	codeList.push_back("\tpublic List<" + structInfo.mStructName + "> mList = new();");
	codeList.push_back("\tpublic " + structInfo.mStructName + " this[int index]");
	codeList.push_back("\t{");
	codeList.push_back("\t\tget { return mList[index]; }");
	codeList.push_back("\t\tset { mList[index] = value; }");
	codeList.push_back("\t}");
	codeList.push_back("\tpublic int Count{ get { return mList.Count; } }");
	codeList.push_back("\tpublic override bool read(SerializerBitRead reader, bool needReadSign)");
	codeList.push_back("\t{");
	codeList.push_back("\t\treturn reader.readCustomList(mList, needReadSign);");
	codeList.push_back("\t}");
	codeList.push_back("\tpublic override void write(SerializerBitWrite writer, bool needWriteSign)");
	codeList.push_back("\t{");
	codeList.push_back("\t\twriter.writeCustomList(mList, needWriteSign);");
	codeList.push_back("\t}");
	codeList.push_back("\tpublic override void resetProperty()");
	codeList.push_back("\t{");
	codeList.push_back("\t\tbase.resetProperty();");
	codeList.push_back("\t\tUN_CLASS_LIST(mList);");
	codeList.push_back("\t}");
	codeList.push_back("\tpublic List<" + structInfo.mStructName + ">.Enumerator GetEnumerator(){ return mList.GetEnumerator(); }");
	codeList.push_back("}");

	writeFile(hotFixPath + structInfo.mStructName + ".cs", codeList);
}

string CodeNetPacket_Client::singleMemberReadLineCSharp(const string& memberName, const string& memberType)
{
	string csType = cppTypeToCSharpType(memberType);
	if (csType == "string")
	{
		return "success = success && reader.readString(out " + memberName + ".mValue);";
	}
	else if (csType == "List<string>" || csType == "List<byte>" || csType == "List<ushort>" || csType == "List<uint>" || csType == "List<ulong>")
	{
		return "success = success && reader.readList(" + memberName + ".mValue);";
	}
	else if (csType == "List<sbyte>" || csType == "List<short>" || csType == "List<int>" || csType == "List<long>" ||
		csType == "List<float>" || csType == "List<double>")
	{
		return "success = success && reader.readList(" + memberName + ".mValue, needReadSign);";
	}
	else if (csType == "bool" || csType == "byte" || csType == "ushort" || csType == "uint" || csType == "ulong" || csType == "Vector2UShort" || csType == "Vector2UInt")
	{
		return "success = success && reader.read(out " + memberName + ".mValue);";
	}
	else if (csType == "sbyte" || csType == "short" || csType == "int" || csType == "long" ||
		csType == "float" || csType == "double" || csType == "Vector2" || csType == "Vector2Short" ||
		csType == "Vector2Int" || csType == "Vector3Int" || csType == "Vector3" || csType == "Vector4")
	{
		return "success = success && reader.read(out " + memberName + ".mValue, needReadSign);";
	}
	ERROR("不支持的类型:" + memberType);
	return "";
}

string CodeNetPacket_Client::singleMemberWriteLineCSharp(const string& memberName, const string& memberType)
{
	string csType = cppTypeToCSharpType(memberType);
	if (csType == "string")
	{
		return "writer.writeString(" + memberName + ");";
	}
	if (csType == "List<string>" || csType == "List<byte>" || csType == "List<ushort>" || csType == "List<uint>" || csType == "List<ulong>")
	{
		return "writer.writeList(" + memberName + ");";
	}
	else if (csType == "List<sbyte>" || csType == "List<short>" || csType == "List<int>" || csType == "List<long>" ||
		csType == "List<float>" || csType == "List<double>")
	{
		return "writer.writeList(" + memberName + ", needWriteSign);";
	}
	else if (csType == "bool" || csType == "byte" || csType == "ushort" || csType == "uint" || csType == "ulong" ||
		csType == "Vector2UShort" || csType == "Vector2UInt")
	{
		return "writer.write(" + memberName + ");";
	}
	else if (csType == "sbyte" || csType == "short" || csType == "int" || csType == "long" || csType == "float" ||
		csType == "double" || csType == "Vector2" || csType == "Vector2Int" || csType == "Vector2Short" || csType == "Vector3" ||
		csType == "Vector3Int" || csType == "Vector4")
	{
		return "writer.write(" + memberName + ", needWriteSign);";
	}
	ERROR("不支持的类型:" + memberType);
	return "";
}

myVector<string> CodeNetPacket_Client::multiMemberReadLineCSharp(const myVector<string>& memberNameList, const string& memberType, bool supportCustom)
{
	string csType = cppTypeToCSharpType(memberType);
	myVector<string> list;
	if (csType == "string")
	{
		FOR_VECTOR(memberNameList)
		{
			list.push_back("success = success && reader.readString(out " + memberNameList[i] + ");");
		}
		return list;
	}
	if (csType == "bool")
	{
		FOR_VECTOR(memberNameList)
		{
			list.push_back("success = success && reader.read(out " + memberNameList[i] + ");");
		}
		return list;
	}
	if (startWith(csType, "List<"))
	{
		const string elementType = getElementTypeCS(csType);
		FOR_VECTOR(memberNameList)
		{
			if (elementType == "string" || elementType == "byte" || elementType == "ushort" || elementType == "uint" || elementType == "ulong" ||
				elementType == "Vector2UShort" || elementType == "Vector2UInt")
			{
				list.push_back("success = success && reader.readList(" + memberNameList[i] + ");");
			}
			else if (elementType == "sbyte" || elementType == "short" || elementType == "int" || elementType == "long" || elementType == "float" ||
				elementType == "double" || elementType == "Vector2" || elementType == "Vector2Int" || elementType == "Vector2Short" ||
				elementType == "Vector3" || elementType == "Vector3Int" || elementType == "Vector4")
			{
				list.push_back("success = success && reader.readList(" + memberNameList[i] + ", needReadSign);");
			}
			else if (elementType == "bool")
			{
				ERROR("不支持bool的列表");
			}
			else
			{
				if (supportCustom)
				{
					list.push_back("success = success && reader.readCustomList(" + memberNameList[i] + ", needReadSign);");
				}
				else
				{
					ERROR("不支持自定义结构体:" + memberType);
				}
			}
		}
		return list;
	}
	if (csType == "byte" || csType == "ushort" || csType == "uint" || csType == "ulong" || csType == "Vector2UShort" || csType == "Vector2UInt")
	{
		const int memberCount = memberNameList.size();
		if (memberCount <= 4)
		{
			string members;
			FOR_I(memberCount)
			{
				members += "out " + memberNameList[i];
				if (i < memberCount - 1)
				{
					members += ", ";
				}
			}
			list.push_back("success = success && reader.read(" + members + ");");
		}
		else
		{
			string tempVarName = "values" + memberNameList[0].substr(1, memberNameList[0].find_first_of('.') - 1);
			list.push_back("Span<" + csType + "> " + tempVarName + " = stackalloc " + csType + "[" + IToS(memberCount) + "];");
			list.push_back("success = success && reader.read(ref " + tempVarName + ");");
			for (int i = 0; i < memberCount; ++i)
			{
				list.push_back(memberNameList[i] + " = " + tempVarName + "[" + IToS(i) + "];");
			}
		}
	}
	else if (csType == "sbyte" || csType == "short" || csType == "int" || csType == "long" || csType == "float" ||
		csType == "double" || csType == "Vector2" || csType == "Vector2Int" || csType == "Vector2Short" || csType == "Vector3" ||
		csType == "Vector3Int" || csType == "Vector4")
	{
		const int memberCount = memberNameList.size();
		if (memberCount <= 4)
		{
			string members;
			FOR_I(memberCount)
			{
				members += "out " + memberNameList[i];
				if (i < memberCount - 1)
				{
					members += ", ";
				}
			}
			list.push_back("success = success && reader.read(" + members + ", needReadSign);");
		}
		else
		{
			string tempVarName = "values" + memberNameList[0].substr(1, memberNameList[0].find_first_of('.') - 1);
			list.push_back("Span<" + csType + "> " + tempVarName + " = stackalloc " + csType + "[" + IToS(memberCount) + "];");
			list.push_back("success = success && reader.read(ref " + tempVarName + ", needReadSign);");
			for (int i = 0; i < memberCount; ++i)
			{
				list.push_back(memberNameList[i] + " = " + tempVarName + "[" + IToS(i) + "];");
			}
		}
	}
	else
	{
		if (supportCustom)
		{
			FOR_VECTOR(memberNameList)
			{
				list.push_back("success = success && reader.readCustomList(" + memberNameList[i] + ", needReadSign);");
			}
		}
		else
		{
			ERROR("不支持自定义结构体:" + memberType);
		}
	}
	return list;
}

myVector<string> CodeNetPacket_Client::multiMemberWriteLineCSharp(const myVector<string>& memberNameList, const string& memberType, bool supportCustom)
{
	string csType = cppTypeToCSharpType(memberType);
	myVector<string> list;
	if (csType == "string")
	{
		FOR_VECTOR(memberNameList)
		{
			list.push_back("writer.writeString(out " + memberNameList[i] + ");");
		}
		return list;
	}
	if (csType == "bool")
	{
		FOR_VECTOR(memberNameList)
		{
			list.push_back("writer.write(out " + memberNameList[i] + ");");
		}
		return list;
	}
	if (startWith(csType, "List<"))
	{
		const string elementType = getElementTypeCS(csType);
		FOR_VECTOR(memberNameList)
		{
			if (elementType == "string" || elementType == "byte" || elementType == "ushort" || elementType == "uint" ||
				elementType == "ulong" || elementType == "Vector2UShort" || elementType == "Vector2UInt")
			{
				list.push_back("writer.writeList(" + memberNameList[i] + ");");
			}
			else if (elementType == "sbyte" || elementType == "short" || elementType == "int" || elementType == "long" || elementType == "float" ||
				elementType == "double" || elementType == "Vector2" || elementType == "Vector2Int" || elementType == "Vector2Short" ||
				elementType == "Vector3" || elementType == "Vector4")
			{
				list.push_back("writer.writeList(" + memberNameList[i] + ", needWriteSign);");
			}
			else if (elementType == "bool")
			{
				ERROR("不支持bool的列表");
			}
			else
			{
				if (supportCustom)
				{
					list.push_back("writer.writeCustomList(" + memberNameList[i] + ", needWriteSign);");
				}
				else
				{
					ERROR("不支持自定义结构体:" + memberType);
				}
			}
		}
		return list;
	}
	if (csType == "byte" || csType == "ushort" || csType == "uint" || csType == "ulong")
	{
		string members;
		FOR_VECTOR(memberNameList)
		{
			members += memberNameList[i];
			if (i < memberNameList.size() - 1)
			{
				members += ", ";
			}
		}
		list.push_back("writer.write(stackalloc " + csType + "[" + IToS(memberNameList.size()) + "]{ " + members + " });");
	}
	else if (csType == "sbyte" || csType == "short" || csType == "int" || csType == "long" || csType == "float" || csType == "double")
	{
		string members;
		FOR_VECTOR(memberNameList)
		{
			members += memberNameList[i];
			if (i < memberNameList.size() - 1)
			{
				members += ", ";
			}
		}
		list.push_back("writer.write(stackalloc " + csType + "[" + IToS(memberNameList.size()) + "]{ " + members + " }, needWriteSign);");
	}
	else
	{
		if (supportCustom)
		{
			FOR_VECTOR(memberNameList)
			{
				list.push_back("writer.writeCustom(" + memberNameList[i] + ", needWriteSign);");
			}
		}
		else
		{
			ERROR("不支持自定义结构体:" + memberType);
		}
	}
	return list;
}

string CodeNetPacket_Client::expandMembersInGroupCSharp(const myVector<PacketMember>& memberList, myVector<string>& memberNameList, bool supportSimplify)
{
	if (memberList.size() == 0)
	{
		return "";
	}
	FOR_VECTOR(memberList)
	{
		const string& typeName = memberList[i].mTypeName;
		const string& memberName = memberList[i].mMemberName;
		if (typeName == "Vector2" || typeName == "Vector2UShort" || typeName == "Vector2Int" || typeName == "Vector2UInt")
		{
			memberNameList.push_back(memberName + ".mValue.x");
			memberNameList.push_back(memberName + ".mValue.y");
		}
		else if (typeName == "Vector3" || typeName == "Vector3Int")
		{
			memberNameList.push_back(memberName + ".mValue.x");
			memberNameList.push_back(memberName + ".mValue.y");
			memberNameList.push_back(memberName + ".mValue.z");
		}
		else if (typeName == "Vector4")
		{
			memberNameList.push_back(memberName + ".mValue.x");
			memberNameList.push_back(memberName + ".mValue.y");
			memberNameList.push_back(memberName + ".mValue.z");
			memberNameList.push_back(memberName + ".mValue.w");
		}
		else
		{
			if (!supportSimplify)
			{
				memberNameList.push_back(memberName + ".mValue");
			}
			else
			{
				memberNameList.push_back(memberName);
			}
		}
	}
	return toPODType(memberList[0].mTypeName);
}