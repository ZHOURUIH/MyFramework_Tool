#include "CodeNetPacket_Server.h"

void CodeNetPacket_Server::generateCpp(const myVector<PacketStruct>& structInfoList, const myVector<PacketInfo>& packetInfoList)
{
	myVector<string> gamePacketNameList;
	for (const PacketInfo& packetInfo : packetInfoList)
	{
		gamePacketNameList.push_back(packetInfo.mPacketName);
	}
	myVector<string> structNameList;
	for (const PacketStruct& structInfo : structInfoList)
	{
		structNameList.push_back(structInfo.mStructName);
	}
	// Game层的消息
	string cppGameCSPacketPath = cppGamePath + "Socket/ClientServer/";
	string cppGameSCPacketPath = cppGamePath + "Socket/ServerClient/";
	string cppGameStructPath = cppGamePath + "Socket/Struct/";
	string cppGamePacketDefinePath = cppGamePath + "Socket/";
	// 删除无用的消息
	// c++ CS
	myVector<string> cppGameCSFiles;
	findFiles(cppGameCSPacketPath, cppGameCSFiles, nullptr, 0);
	for (int i = 0; i < cppGameCSFiles.size(); ++i)
	{
		if (!gamePacketNameList.contains(getFileNameNoSuffix(cppGameCSFiles[i], true)))
		{
			deleteFile(cppGameCSFiles[i]);
			cppGameCSFiles.eraseAt(i--);
		}
	}
	// c++ SC
	myVector<string> cppGameSCFiles;
	findFiles(cppGameSCPacketPath, cppGameSCFiles, nullptr, 0);
	for (int i = 0; i < cppGameSCFiles.size(); ++i)
	{
		if (!gamePacketNameList.contains(getFileNameNoSuffix(cppGameSCFiles[i], true)))
		{
			deleteFile(cppGameSCFiles[i]);
			cppGameSCFiles.eraseAt(i--);
		}
	}
	// 消息结构体代码
	myVector<string> cppNetStructFiles;
	findFiles(cppGameStructPath, cppNetStructFiles, nullptr, 0);
	for (int i = 0; i < cppNetStructFiles.size(); ++i)
	{
		if (!structNameList.contains(getFileNameNoSuffix(cppNetStructFiles[i], true)))
		{
			deleteFile(cppNetStructFiles[i]);
			cppNetStructFiles.eraseAt(i--);
		}
	}

	// 生成c++代码
	for (const PacketInfo& packetInfo : packetInfoList)
	{
		// 找到有没有此文件,有就在原来的文件上修改
		string csHeaderPath = cppGameCSPacketPath;
		string csSourcePath = cppGameCSPacketPath;
		string scHeaderPath = cppGameSCPacketPath;
		for (const string& file : cppGameCSFiles)
		{
			if (endWith(file, packetInfo.mPacketName + ".h"))
			{
				csHeaderPath = getFilePath(file) + "/";
			}
			if (endWith(file, packetInfo.mPacketName + ".cpp"))
			{
				csSourcePath = getFilePath(file) + "/";
			}
		}
		for (const string& file : cppGameSCFiles)
		{
			if (endWith(file, packetInfo.mPacketName + ".h"))
			{
				scHeaderPath = getFilePath(file) + "/";
			}
		}
		CodeNetPacket_Server::generateCppCSPacketFileHeader(packetInfo, csHeaderPath);
		CodeNetPacket_Server::generateCppCSPacketFileSource(packetInfo, csSourcePath);
		CodeNetPacket_Server::generateCppSCPacketFileHeader(packetInfo, scHeaderPath);
		CodeNetPacket_Server::generateCppSCPacketFileSource(packetInfo, scHeaderPath);
	}
	CodeNetPacket_Server::generateCppGamePacketDefineFile(packetInfoList, cppGamePacketDefinePath);
	CodeNetPacket_Server::generateCppGamePacketRegisteFile(packetInfoList, structInfoList, cppGamePacketDefinePath);

	for (const PacketStruct& info : structInfoList)
	{
		CodeNetPacket_Server::generateCppStruct(info, cppGameStructPath);
	}
}

// PacketDefine.h文件
void CodeNetPacket_Server::generateCppGamePacketDefineFile(const myVector<PacketInfo>& packetList, const string& filePath)
{
	const string fullPath = filePath + "GamePacketDefine.h";
	myVector<string> generateList;
	generateList.push_back("\tconstexpr static ushort MIN = 0;");
	generateList.push_back("");
	int csMinValue = 10000;
	generateList.push_back("\tconstexpr static ushort CS_MIN = " + IToS(csMinValue) + ";");
	uint packetCount = packetList.size();
	FOR_I(packetCount)
	{
		if (startWith(packetList[i].mPacketName, "CS"))
		{
			generateList.push_back("\tconstexpr static ushort " + packetList[i].mPacketName + " = " + IToS(++csMinValue) + ";");
		}
	}
	generateList.push_back("");
	int scMinValue = 20000;
	generateList.push_back("\tconstexpr static ushort SC_MIN = " + IToS(scMinValue) + ";");
	FOR_I(packetCount)
	{
		if (startWith(packetList[i].mPacketName, "SC"))
		{
			generateList.push_back("\tconstexpr static ushort " + packetList[i].mPacketName + " = " + IToS(++scMinValue) + ";");
		}
	}
	if (isFileExist(fullPath))
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(fullPath, codeList, lineStart,
			[](const string& codeLine) { return endWith(codeLine, "// auto generate start"); },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }))
		{
			return;
		}
		for (const string& line : generateList)
		{
			codeList.insert(++lineStart, line);
		}
		writeFile(fullPath, codeList);
	}
	else
	{
		string str;
		line(str, "#pragma once");
		line(str, "");
		line(str, "#include \"FrameDefine.h\"");
		line(str, "");
		line(str, "class PACKET_TYPE");
		line(str, "{");
		line(str, "public:");
		line(str, "// auto generate start");
		for (const string& code : generateList)
		{
			line(str, code);
		}
		line(str, "// auto generate end");
		line(str, "};", false);

		writeFile(fullPath, str);
	}
}

// PacketRegister.cpp文件
void CodeNetPacket_Server::generateCppGamePacketRegisteFile(const myVector<PacketInfo>& packetList, const myVector<PacketStruct>& structInfoList, const string& filePath)
{
	string str;
	line(str, "// auto generate start");
	line(str, "#include \"GameHeader.h\"");
	line(str, "");
	line(str, "string GamePacketRegister::PACKET_VERSION = \"" + generatePacketVersion(packetList, structInfoList) + "\";");
	line(str, "void GamePacketRegister::registeAll()");
	line(str, "{");
	myVector<PacketInfo> udpCSList;
	for (const auto& info : packetList)
	{
		if (startWith(info.mPacketName, "CS") && info.mUDP)
		{
			udpCSList.push_back(info);
		}
	}
	for (const auto& info : packetList)
	{
		const string& packetName = info.mPacketName;
		if (!startWith(packetName, "CS"))
		{
			continue;
		}
		line(str, "\tmPacketTCPFactoryManager->addFactory<" + packetName + ">(PACKET_TYPE::" + packetName + ");");
	}
	line(str, "");
	myVector<PacketInfo> udpSCList;
	for (const auto& info : packetList)
	{
		if (!startWith(info.mPacketName, "SC"))
		{
			continue;
		}
		if (info.mUDP)
		{
			udpSCList.push_back(info);
		}
	}
	for (const auto& info : packetList)
	{
		const string& packetName = info.mPacketName;
		if (!startWith(packetName, "SC"))
		{
			continue;
		}
		line(str, "\tmPacketTCPFactoryManager->addFactory<" + packetName + ">(PACKET_TYPE::" + packetName + ");");
	}
	if (udpCSList.size() > 0)
	{
		line(str, "");
		for (const auto& info : udpCSList)
		{
			line(str, "\tmPacketTCPFactoryManager->addUDPPacket(PACKET_TYPE::" + info.mPacketName + "); ");
		}
	}
	if (udpSCList.size() > 0)
	{
		line(str, "");
		for (const auto& info : udpSCList)
		{
			line(str, "\tmPacketTCPFactoryManager->addUDPPacket(PACKET_TYPE::" + info.mPacketName + "); ");
		}
	}
	line(str, "};");
	line(str, "// auto generate end", false);
	writeFile(filePath + "GamePacketRegister.cpp", str);
}

// CSPacket.h
void CodeNetPacket_Server::generateCppCSPacketFileHeader(const PacketInfo& packetInfo, const string& filePath)
{
	const string& packetName = packetInfo.mPacketName;
	if (!startWith(packetName, "CS"))
	{
		return;
	}

	bool hasOptional = false;
	for (const auto& item : packetInfo.mMemberList)
	{
		if (item.mOptional)
		{
			hasOptional = true;
			break;
		}
	}

	myVector<string> generateCodes;
	generateCodes.push_back(packetInfo.mComment);
	generateCodes.push_back("class " + packetName + " : public PacketTCP");
	generateCodes.push_back("{");
	generateCodes.push_back("\tBASE(" + packetName + ", PacketTCP);");
	if (hasOptional)
	{
		generateCodes.push_back("public:");
		generateCodes.push_back("\tenum class Field : byte");
		generateCodes.push_back("\t{");
		FOR_I(packetInfo.mMemberList.size())
		{
			const auto& item = packetInfo.mMemberList[i];
			if (item.mOptional)
			{
				if (i >= 64)
				{
					ERROR("可选字段的下标不能超过63");
					break;
				}
				generateCodes.push_back("\t\t" + item.mMemberNameNoPrefix + " = " + IToS(i) + ",");
			}
		}
		generateCodes.push_back("\t};");
	}
	generateCodes.push_back("public:");
	generateCppPacketMemberDeclare(packetInfo.mMemberList, generateCodes);
	generateCodes.push_back("\tstatic " + packetName + " mStaticObject;");
	generateCodes.push_back("\tstatic string mPacketName;");
	generateCodes.push_back("public:");
	generateCodes.push_back("\t" + packetName + "()");
	generateCodes.push_back("\t{");
	generateCodes.push_back("\t\tmType = PACKET_TYPE::" + packetName + ";");
	generateCodes.push_back("\t\tmShowInfo = " + boolToString(packetInfo.mShowInfo) + ";");
	generateCodes.push_back("\t}");
	generateCodes.push_back("\tstatic " + packetName + "& get()");
	generateCodes.push_back("\t{");
	generateCodes.push_back("\t\tmStaticObject.resetProperty();");
	generateCodes.push_back("\t\treturn mStaticObject;");
	generateCodes.push_back("\t}");
	generateCodes.push_back("\tstatic const string& getStaticPacketName() { return mPacketName; }");
	generateCodes.push_back("\tconst string& getPacketName() override { return mPacketName; }");
	generateCppPacketReadWrite(packetInfo, generateCodes);
	generateCodes.push_back("\tvoid execute() override;");

	// CSPacket.h
	string headerFullPath = filePath + packetName + ".h";
	if (isFileExist(headerFullPath))
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(headerFullPath, codeList, lineStart,
			[](const string& codeLine) { return codeLine == "// auto generate start"; },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }))
		{
			return;
		}
		for (const string& line : generateCodes)
		{
			codeList.insert(++lineStart, line);
		}
		writeFile(headerFullPath, codeList);
	}
	else
	{
		myVector<string> codeList;
		codeList.push_back("#pragma once");
		codeList.push_back("");
		codeList.push_back("#include \"PacketTCP.h\"");
		codeList.push_back("#include \"GamePacketDefine.h\"");
		codeList.push_back("");
		codeList.push_back("// auto generate start");
		codeList.addRange(generateCodes);
		codeList.push_back("\t// auto generate end");
		codeList.push_back("\tvoid debugInfo(MyString<1024>& buffer) override");
		codeList.push_back("\t{");
		codeList.push_back("\t\tdebug(buffer, \"\");");
		codeList.push_back("\t}");
		codeList.push_back("};");
		writeFile(headerFullPath, codeList);
	}
}

void CodeNetPacket_Server::generateCppStruct(const PacketStruct& structInfo, const string& filePath)
{
	const string& structName = structInfo.mStructName;

	// 是否需要移动构造,当有列表,或者字符串等可移动的变量时,就需要有移动构造
	bool hasMoveConstruct = false;
	bool hasOptional = false;
	for (const PacketMember& member : structInfo.mMemberList)
	{
		if (!hasMoveConstruct && (member.mTypeName == "string" || startWith(member.mTypeName, "Vector<")))
		{
			hasMoveConstruct = true;
		}
		if (!hasOptional && member.mOptional)
		{
			hasOptional = true;
		}
	}

	// PacketStruct.h
	const string headerFullPath = filePath + structName + ".h";
	myVector<string> headerCodeList;
	headerCodeList.push_back("#pragma once");
	headerCodeList.push_back("");
	headerCodeList.push_back("#include \"SerializableBitData.h\"");
	headerCodeList.push_back("");
	headerCodeList.push_back(structInfo.mComment);
	headerCodeList.push_back("class " + structName + " : public SerializableBitData");
	headerCodeList.push_back("{");
	headerCodeList.push_back("\tBASE(" + structName + ", SerializableBitData);");
	// 是否有可选字段
	if (hasOptional)
	{
		headerCodeList.push_back("public:");
		headerCodeList.push_back("\tenum class Field : byte");
		headerCodeList.push_back("\t{");
		FOR_I (structInfo.mMemberList.size())
		{
			const PacketMember& member = structInfo.mMemberList[i];
			if (member.mOptional)
			{
				if (i >= 64)
				{
					ERROR("可选字段的下标不能超过63");
					break;
				}
				headerCodeList.push_back("\t\t" + member.mMemberNameNoPrefix + " = " + IToS(i) + ",");
			}
		}
		headerCodeList.push_back("\t};");
	}
	headerCodeList.push_back("public:");
	generateCppPacketMemberDeclare(structInfo.mMemberList, headerCodeList);
	headerCodeList.push_back("public:");
	headerCodeList.push_back("\t" + structName + "() = default;");
	string constructParams;
	string constructMoveParams;
	const int memberCount = structInfo.mMemberList.size();
	// 当结构体成员数量不超过6时,提供带参构造和带可移动参的构造
	if (memberCount <= 6)
	{
		FOR_I(memberCount)
		{
			const PacketMember& member = structInfo.mMemberList[i];
			// 成员变量的命名格式都是以m开头,且后面的第一个字母是大写,所以需要去除m,将大写字母变为小写
			string tempParamName = member.mMemberName.substr(2);
			tempParamName.insert(0, 1, toLower(member.mMemberName[1]));
			if (member.mTypeName == "string" || 
				startWith(member.mTypeName, "Vector<") || 
				member.mTypeName == "Vector2" || 
				member.mTypeName == "Vector2UShort" || 
				member.mTypeName == "Vector2UInt" || 
				member.mTypeName == "Vector2Int" || 
				member.mTypeName == "Vector3" || 
				member.mTypeName == "Vector4")
			{
				constructParams += "const " + member.mTypeName + "& " + tempParamName;
			}
			else
			{
				constructParams += member.mTypeName + " " + tempParamName;
			}
			if (i != memberCount - 1)
			{
				constructParams += ", ";
			}
		}
		headerCodeList.push_back("\t" + structName + "(" + constructParams + ");");
		if (hasMoveConstruct)
		{
			FOR_I(memberCount)
			{
				const PacketMember& member = structInfo.mMemberList[i];
				// 成员变量的命名格式都是以m开头,且后面的第一个字母是大写,所以需要去除m,将大写字母变为小写
				string tempParamName = member.mMemberName.substr(2);
				tempParamName.insert(0, 1, toLower(member.mMemberName[1]));
				if (member.mTypeName == "string" || startWith(member.mTypeName, "Vector<"))
				{
					constructMoveParams += "" + member.mTypeName + "&& " + tempParamName;
				}
				else
				{
					constructMoveParams += member.mTypeName + " " + tempParamName;
				}
				if (i != memberCount - 1)
				{
					constructMoveParams += ", ";
				}
			}
			headerCodeList.push_back("\t" + structName + "(" + constructMoveParams + ");");
		}		
	}
	if (hasMoveConstruct)
	{
		headerCodeList.push_back("\t" + structName + "(const " + structName + "& other);");
		headerCodeList.push_back("\t" + structName + "(" + structName + "&& other) noexcept;");
		headerCodeList.push_back("\t" + structName + "& operator=(" + structName + "&& other) noexcept;");
	}
	headerCodeList.push_back("\t" + structName + "& operator=(const " + structName + "& other);");
	headerCodeList.push_back("\tbool readFromBuffer(SerializerBitRead* reader, const bool needReadSign) override;");
	headerCodeList.push_back("\tbool writeToBuffer(SerializerBitWrite* writer, const bool needWriteSign) const override;");
	headerCodeList.push_back("\tbool hasSign() const override");
	headerCodeList.push_back("\t{");
	for (const PacketMember& item : structInfo.mMemberList)
	{
		const string& cppType = item.mTypeName;
		// 无符号的基础数据类型
		if (cppType == "string" || cppType == "bool" || cppType == "byte" || cppType == "ushort" || cppType == "uint" || cppType == "ullong" || cppType == "Vector2UShort" || cppType == "Vector2UInt")
		{
			// 无符号的不做判断
		}
		// 有符号的基础数据类型
		else if (cppType == "char" || cppType == "short" || cppType == "int" || cppType == "llong" || cppType == "float" || cppType == "double" ||
				 cppType == "Vector2" || cppType == "Vector3" || cppType == "Vector2Short" || cppType == "Vector2Int" || cppType == "Vector3Int")
		{
			if (item.mOptional)
			{
				headerCodeList.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && " + item.mMemberName + " < 0)");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
			else
			{
				headerCodeList.push_back("\t\tif (" + item.mMemberName + " < 0)");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
		}
		// 有符号的基础数据类型
		else if (cppType == "Vector2" ||cppType == "Vector2Short" || cppType == "Vector2Int")
		{
			if (item.mOptional)
			{
				headerCodeList.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0))");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
			else
			{
				headerCodeList.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0)");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
		}
		// 有符号的基础数据类型
		else if (cppType == "Vector3" || cppType == "Vector3Int")
		{
			if (item.mOptional)
			{
				headerCodeList.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0))");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
			else
			{
				headerCodeList.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0)");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
		}
		// 有符号的基础数据类型
		else if (cppType == "Vector4")
		{
			if (item.mOptional)
			{
				headerCodeList.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0 || " + item.mMemberName + ".w < 0))");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
			else
			{
				headerCodeList.push_back("\t\tif (" + item.mMemberName + ".x < 0 || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0 || " + item.mMemberName + ".w < 0)");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
		}
		// 有符号的基础数据类型的列表
		else if (cppType == "Vector<char>" || cppType == "Vector<short>" || cppType == "Vector<int>" || cppType == "Vector<llong>" ||
				 cppType == "Vector<float>" || cppType == "Vector<double>")
		{
			string elementType = getElementTypeCpp(cppType);
			if (item.mOptional)
			{
				headerCodeList.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0)");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\tfor (const " + elementType + " item : " + item.mMemberName + ")");
				headerCodeList.push_back("\t\t\t{");
				headerCodeList.push_back("\t\t\t\tif (item < 0)");
				headerCodeList.push_back("\t\t\t\t{");
				headerCodeList.push_back("\t\t\t\t\treturn true;");
				headerCodeList.push_back("\t\t\t\t}");
				headerCodeList.push_back("\t\t\t}");
				headerCodeList.push_back("\t\t}");
			}
			else
			{
				headerCodeList.push_back("\t\tfor (const " + elementType + " item : " + item.mMemberName + ")");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\tif (item < 0)");
				headerCodeList.push_back("\t\t\t{");
				headerCodeList.push_back("\t\t\t\treturn true;");
				headerCodeList.push_back("\t\t\t}");
				headerCodeList.push_back("\t\t}");
			}
		}
		// 无符号的基础数据类型的列表
		else if (cppType == "Vector<string>" || cppType == "Vector<bool>" || cppType == "Vector<byte>" || cppType == "Vector<ushort>" || cppType == "Vector<uint>" || cppType == "Vector<ullong>")
		{
			// 无符号的不做处理
		}
		// 自定义数据类型
		else
		{
			if (item.mOptional)
			{
				headerCodeList.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && " + item.mMemberName + ".hasSign())");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
			else
			{
				headerCodeList.push_back("\t\tif (" + item.mMemberName + ".hasSign())");
				headerCodeList.push_back("\t\t{");
				headerCodeList.push_back("\t\t\treturn true;");
				headerCodeList.push_back("\t\t}");
			}
		}
	}
	headerCodeList.push_back("\t\treturn false;");
	headerCodeList.push_back("\t}");
	headerCodeList.push_back("\tvoid resetProperty() override;");
	if (hasOptional)
	{
		headerCodeList.push_back("\tstatic constexpr ullong fullOptionFlag()");
		headerCodeList.push_back("\t{");
		headerCodeList.push_back("\t\tullong fieldFlag = 0;");
		for (const PacketMember& item : structInfo.mMemberList)
		{
			if (item.mOptional)
			{
				headerCodeList.push_back("\t\tfieldFlag |= 1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ";");
			}
		}
		headerCodeList.push_back("\t\treturn fieldFlag;");
		headerCodeList.push_back("\t}");
	}
	
	headerCodeList.push_back("};");
	writeFile(headerFullPath, headerCodeList);
	
	myVector<myVector<PacketMember>> memberGroupList;
	generateMemberGroup(structInfo.mMemberList, memberGroupList);

	// PacketStruct.cpp
	string sourceFullPath = filePath + structName + ".cpp";
	myVector<string> sourceCodeList;
	sourceCodeList.push_back("#include \"GameHeader.h\"");
	if (constructParams.length() > 0)
	{
		sourceCodeList.push_back("");
		sourceCodeList.push_back(structName + "::" + structName + "(" + constructParams + ") :");
		FOR_I(memberCount)
		{
			const PacketMember& member = structInfo.mMemberList[i];
			const string endComma = i != memberCount - 1 ? "," : "";
			// 成员变量的命名格式都是以m开头,且后面的第一个字母是大写,所以需要去除m,将大写字母变为小写
			string tempParamName = member.mMemberName.substr(2);
			tempParamName.insert(0, 1, toLower(member.mMemberName[1]));
			sourceCodeList.push_back("\t" + member.mMemberName + "(" + tempParamName + ")" + endComma);
		}
		sourceCodeList.push_back("{}");
	}
	if (constructMoveParams.length() > 0)
	{
		sourceCodeList.push_back("");
		sourceCodeList.push_back(structName + "::" + structName + "(" + constructMoveParams + ") :");
		FOR_I(memberCount)
		{
			const PacketMember& member = structInfo.mMemberList[i];
			const string endComma = i != memberCount - 1 ? "," : "";
			// 成员变量的命名格式都是以m开头,且后面的第一个字母是大写,所以需要去除m,将大写字母变为小写
			string tempParamName = member.mMemberName.substr(2);
			tempParamName.insert(0, 1, toLower(member.mMemberName[1]));
			if (member.mTypeName == "string" || startWith(member.mTypeName, "Vector<"))
			{
				sourceCodeList.push_back("\t" + member.mMemberName + "(move(" + tempParamName + "))" + endComma);
			}
			else
			{
				sourceCodeList.push_back("\t" + member.mMemberName + "(" + tempParamName + ")" + endComma);
			}
		}
		sourceCodeList.push_back("{}");
	}
	if (hasMoveConstruct)
	{
		sourceCodeList.push_back("");
		sourceCodeList.push_back(structName + "::" + structName + "(const " + structName + "& other) :");
		FOR_I(memberCount)
		{
			const PacketMember& member = structInfo.mMemberList[i];
			const string endComma = i != memberCount - 1 ? "," : "";
			sourceCodeList.push_back("\t" + member.mMemberName + "(other." + member.mMemberName + ")" + endComma);
		}
		sourceCodeList.push_back("{}");
		sourceCodeList.push_back("");
		sourceCodeList.push_back(structName + "::" + structName + "(" + structName + "&& other) noexcept :");
		FOR_I(memberCount)
		{
			const PacketMember& member = structInfo.mMemberList[i];
			const string endComma = i != memberCount - 1 ? "," : "";
			if (member.mTypeName == "string" || startWith(member.mTypeName, "Vector<"))
			{
				sourceCodeList.push_back("\t" + member.mMemberName + "(move(other." + member.mMemberName + "))" + endComma);
			}
			else
			{
				sourceCodeList.push_back("\t" + member.mMemberName + "(other." + member.mMemberName + ")" + endComma);
			}
		}
		sourceCodeList.push_back("{}");
		sourceCodeList.push_back("");
		sourceCodeList.push_back(structName + "& " + structName + "::operator=(" + structName + "&& other) noexcept");
		sourceCodeList.push_back("{");
		FOR_I(memberCount)
		{
			const PacketMember& member = structInfo.mMemberList[i];
			if (member.mTypeName == "string" || startWith(member.mTypeName, "Vector<"))
			{
				sourceCodeList.push_back("\t" + member.mMemberName + " = move(other." + member.mMemberName + ");");
			}
			else
			{
				sourceCodeList.push_back("\t" + member.mMemberName + " = other." + member.mMemberName + ";");
			}
		}
		sourceCodeList.push_back("\treturn *this;");
		sourceCodeList.push_back("}");
	}
	sourceCodeList.push_back("");
	sourceCodeList.push_back(structName + "& " + structName + "::operator=(const " + structName + "& other)");
	sourceCodeList.push_back("{");
	for (const PacketMember& member : structInfo.mMemberList)
	{
		sourceCodeList.push_back("\t" + member.mMemberName + " = other." + member.mMemberName + ";");
	}
	sourceCodeList.push_back("\treturn *this;");
	sourceCodeList.push_back("}");

	// readFromBuffer
	sourceCodeList.push_back("");
	sourceCodeList.push_back("bool " + structName + "::readFromBuffer(SerializerBitRead* reader, const bool needReadSign)");
	sourceCodeList.push_back("{");
	if (hasOptional)
	{
		sourceCodeList.push_back("\t// 从缓冲区读取位标记");
		sourceCodeList.push_back("\tbool useFlag = false;");
		sourceCodeList.push_back("\treader->readBool(useFlag);");
		sourceCodeList.push_back("\tullong fieldFlag = FrameDefine::FULL_FIELD_FLAG;");
		sourceCodeList.push_back("\tif (useFlag)");
		sourceCodeList.push_back("\t{");
		sourceCodeList.push_back("\t\treader->readUnsigned(fieldFlag);");
		sourceCodeList.push_back("\t}");
		sourceCodeList.push_back("");
		sourceCodeList.push_back("\t// 再根据位标记读取字段数据");
	}
	sourceCodeList.push_back("\tbool success = true;");
	FOR_VECTOR(memberGroupList)
	{
		const myVector<PacketMember>& memberGroup = memberGroupList[i];
		if (memberGroup.size() == 1)
		{
			const PacketMember& item = memberGroup[0];
			// 可选字段需要特别判断一下
			if (item.mOptional)
			{
				sourceCodeList.push_back("\tif (hasBit(fieldFlag, (byte)Field::" + item.mMemberNameNoPrefix + "))");
				sourceCodeList.push_back("\t{");
				sourceCodeList.push_back("\t\t" + singleMemberReadLine(item.mMemberName, item.mTypeName, false));
				sourceCodeList.push_back("\t}");
			}
			else
			{
				sourceCodeList.push_back("\t" + singleMemberReadLine(item.mMemberName, item.mTypeName, false));
			}
		}
		else
		{
			myVector<string> nameList;
			string groupTypeName = expandMembersInGroup(memberGroup, nameList);
			myVector<string> list = multiMemberReadLine(nameList, groupTypeName, false);
			FOR_VECTOR(list)
			{
				sourceCodeList.push_back("\t" + list[i]);
			}
		}
	}
	sourceCodeList.push_back("\treturn success;");
	sourceCodeList.push_back("}");

	// writeToBuffer
	sourceCodeList.push_back("");
	sourceCodeList.push_back("bool " + structName + "::writeToBuffer(SerializerBitWrite* writer, const bool needWriteSign) const");
	sourceCodeList.push_back("{");
	if (hasOptional)
	{
		sourceCodeList.push_back("\t// 将位标记写入到缓冲区");
		sourceCodeList.push_back("\t// 如果没有可选字段,则不使用位标记(在生成代码时会进行判断,所以不需要运行时再判断)");
		sourceCodeList.push_back("\t// 如果有可选字段,但是所有字段都需要同步,则也不使用位标记");
		sourceCodeList.push_back("\tbool useFlag = fullOptionFlag() != mFieldFlag;");
		sourceCodeList.push_back("\twriter->writeBool(useFlag);");
		sourceCodeList.push_back("\tif (useFlag)");
		sourceCodeList.push_back("\t{");
		sourceCodeList.push_back("\t\twriter->writeUnsigned(mFieldFlag);");
		sourceCodeList.push_back("\t}");
		sourceCodeList.push_back("\t");
		sourceCodeList.push_back("\t// 再根据位标记将字段数据写入缓冲区");
	}
	sourceCodeList.push_back("\tbool success = true;");
	FOR_VECTOR(memberGroupList)
	{
		const myVector<PacketMember>& memberGroup = memberGroupList[i];
		if (memberGroup.size() == 1)
		{
			const PacketMember& item = memberGroup[0];
			// 可选字段需要特别判断一下
			if (item.mOptional)
			{
				sourceCodeList.push_back("\tif (isFieldValid(Field::" + item.mMemberNameNoPrefix + "))");
				sourceCodeList.push_back("\t{");
				sourceCodeList.push_back("\t\t" + singleMemberWriteLine(item.mMemberName, item.mTypeName, false));
				sourceCodeList.push_back("\t}");
			}
			else
			{
				sourceCodeList.push_back("\t" + singleMemberWriteLine(item.mMemberName, item.mTypeName, false));
			}
		}
		else
		{
			myVector<string> nameList;
			string groupTypeName = expandMembersInGroup(memberGroup, nameList);
			myVector<string> list = multiMemberWriteLine(nameList, groupTypeName, false);
			FOR_VECTOR(list)
			{
				sourceCodeList.push_back("\t" + list[i]);
			}
		}
	}
	sourceCodeList.push_back("\treturn success;");
	sourceCodeList.push_back("}");
	
	// resetProperty
	sourceCodeList.push_back("");
	sourceCodeList.push_back("void " + structName + "::resetProperty()");
	sourceCodeList.push_back("{");
	sourceCodeList.push_back("\tbase::resetProperty();");
	for (const PacketMember& item : structInfo.mMemberList)
	{
		if (item.mTypeName == "Vector<bool>")
		{
			ERROR("不支持Vector<bool>类型,请使用Vector<byte>代替,packetType:" + structName);
		}
		if (item.mTypeName == "string" || 
			startWith(item.mTypeName, "Vector<") || 
			item.mTypeName == "Vector2" || 
			item.mTypeName == "Vector2UShort" || 
			item.mTypeName == "Vector2UInt" || 
			item.mTypeName == "Vector2Int" || 
			item.mTypeName == "Vector3" || 
			item.mTypeName == "Vector4")
		{
			sourceCodeList.push_back("\t" + item.mMemberName + ".clear();");
		}
		else if (item.mTypeName == "bool")
		{
			sourceCodeList.push_back("\t" + item.mMemberName + " = false;");
		}
		else if (isPodInteger(item.mTypeName))
		{
			sourceCodeList.push_back("\t" + item.mMemberName + " = 0;");
		}
		else if (item.mTypeName == "float")
		{
			sourceCodeList.push_back("\t" + item.mMemberName + " = 0.0f;");
		}
		else if (item.mTypeName == "double")
		{
			sourceCodeList.push_back("\t" + item.mMemberName + " = 0.0;");
		}
		else
		{
			ERROR("结构体中不支持自定义结构体:" + item.mTypeName);
		}
	}
	sourceCodeList.push_back("}");

	writeFile(sourceFullPath, sourceCodeList);
}

// CSPacket.cpp
void CodeNetPacket_Server::generateCppCSPacketFileSource(const PacketInfo& packetInfo, const string& filePath)
{
	const string& packetName = packetInfo.mPacketName;
	if (!startWith(packetName, "CS"))
	{
		return;
	}

	const string cppFullPath = filePath + packetName + ".cpp";
	if (isFileExist(cppFullPath))
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(cppFullPath, codeList, lineStart,
			[](const string& codeLine) { return codeLine == "// auto generate start"; },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }, false))
		{
			// 找不到就认为第一行是include,在第一行的下面插入
			lineStart = 0;
			codeList.insert(++lineStart, "");
			codeList.insert(++lineStart, "// auto generate start");
			codeList.insert(++lineStart, "string " + packetName + "::mPacketName = STR(" + packetName + ");");
			codeList.insert(++lineStart, "// auto generate end");
			writeFile(cppFullPath, codeList);
			return;
		}
		codeList.insert(++lineStart, packetName + " " + packetName + "::mStaticObject;");
		codeList.insert(++lineStart, "string " + packetName + "::mPacketName = STR(" + packetName + ");");
		writeFile(cppFullPath, codeList);
	}
	else
	{
		string source;
		line(source, "#include \"GameHeader.h\"");
		line(source, "");
		line(source, "// auto generate start");
		line(source, packetName + " " + packetName + "::mStaticObject;");
		line(source, "string " + packetName + "::mPacketName = STR(" + packetName + ");");
		line(source, "// auto generate end");
		line(source, "");
		line(source, "void " + packetName + "::execute()");
		line(source, "{");
		line(source, "\tCharacterPlayer* player = getPlayer(mClient->getPlayerGUID());");
		line(source, "\tif (player == nullptr)");
		line(source, "\t{");
		line(source, "\t\treturn;");
		line(source, "\t}");
		line(source, "}", false);
		writeFile(cppFullPath, source);
	}
}

void CodeNetPacket_Server::generateCppPacketMemberDeclare(const myVector<PacketMember>& memberList, myVector<string>& generateCodes)
{
	for (const PacketMember& item : memberList)
	{
		string line;
		if (item.mTypeName == "byte" ||
			item.mTypeName == "char" ||
			item.mTypeName == "short" ||
			item.mTypeName == "ushort" ||
			item.mTypeName == "int" ||
			item.mTypeName == "uint" ||
			item.mTypeName == "llong" ||
			item.mTypeName == "ullong")
		{
			line = "\t" + item.mTypeName + " " + item.mMemberName + " = 0;";
		}
		else if (item.mTypeName == "float" ||
				 item.mTypeName == "double")
		{
			line = "\t" + item.mTypeName + " " + item.mMemberName + " = 0.0f;";
		}
		else if (item.mTypeName == "bool")
		{
			line = "\t" + item.mTypeName + " " + item.mMemberName + " = false;";
		}
		else
		{
			line = "\t" + item.mTypeName + " " + item.mMemberName + ";";
		}
		if (!item.mComment.empty())
		{
			appendWithAlign(line, "// " + item.mComment, 52);
		}
		generateCodes.push_back(line);
	}
}

void CodeNetPacket_Server::generateCppPacketReadWrite(const PacketInfo& packetInfo, myVector<string>& generateCodes)
{
	if (packetInfo.mMemberList.size() > 0)
	{
		myVector<myVector<PacketMember>> memberGroupList;
		generateMemberGroup(packetInfo.mMemberList, memberGroupList);

		// readFromBuffer
		generateCodes.push_back("\tbool readFromBuffer(SerializerBitRead* reader, const bool needReadSign) override");
		generateCodes.push_back("\t{");
		generateCodes.push_back("\t\tbool success = true;");
		FOR_VECTOR(memberGroupList)
		{
			const myVector<PacketMember>& memberList = memberGroupList[i];
			// 该组中只有一个成员,才有可能是optional
			if (memberList.size() == 1)
			{
				const PacketMember& item = memberList[0];
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (isFieldValid(Field::" + item.mMemberNameNoPrefix + "))");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\t" + singleMemberReadLine(item.mMemberName, item.mTypeName, true));
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\t" + singleMemberReadLine(item.mMemberName, item.mTypeName, true));
				}
			}
			else
			{
				myVector<string> nameList;
				string groupTypeName = expandMembersInGroup(memberList, nameList);
				myVector<string> list = multiMemberReadLine(nameList, groupTypeName, true);
				FOR_VECTOR(list)
				{
					generateCodes.push_back("\t\t" + list[i]);
				}
			}
		}
		generateCodes.push_back("\t\treturn success;");
		generateCodes.push_back("\t}");

		// writeToBuffer
		generateCodes.push_back("\tbool writeToBuffer(SerializerBitWrite* writer, const bool needWriteSign) const override");
		generateCodes.push_back("\t{");
		generateCodes.push_back("\t\tbool success = true;");
		FOR_VECTOR(memberGroupList)
		{
			const myVector<PacketMember>& memberList = memberGroupList[i];
			// 该组中只有一个成员,才有可能是optional
			if (memberList.size() == 1)
			{
				const PacketMember& item = memberList[0];
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif (isFieldValid(Field::" + item.mMemberNameNoPrefix + "))");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\t" + singleMemberWriteLine(item.mMemberName, item.mTypeName, true));
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\t" + singleMemberWriteLine(item.mMemberName, item.mTypeName, true));
				}
			}
			else
			{
				myVector<string> nameList;
				string groupTypeName = expandMembersInGroup(memberList, nameList);
				myVector<string> list = multiMemberWriteLine(nameList, groupTypeName, true);
				FOR_VECTOR(list)
				{
					generateCodes.push_back("\t\t" + list[i]);
				}
			}
		}
		generateCodes.push_back("\t\treturn success;");
		generateCodes.push_back("\t}");

		// generateHasSignInternal
		generateCodes.push_back("\tbool generateHasSignInternal() const override");
		generateCodes.push_back("\t{");
		for (const PacketMember& item : packetInfo.mMemberList)
		{
			const string& cppType = item.mTypeName;
			// 无符号的基础数据类型
			if (cppType == "string" || cppType == "bool" || cppType == "byte" || cppType == "ushort" || cppType == "uint" || cppType == "ullong" || cppType == "Vector2UShort" || cppType == "Vector2UInt")
			{
				// 无符号的不做判断
			}
			// 有符号的基础数据类型
			else if (cppType == "char" || cppType == "short" || cppType == "int" || cppType == "llong" || cppType == "float" || cppType == "double")
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && " + item.mMemberName + " < 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
				else
				{
					generateCodes.push_back("\t\tif (" + item.mMemberName + " < 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\treturn true;");
					generateCodes.push_back("\t\t}");
				}
			}
			else if (cppType == "Vector2" || cppType == "Vector2Int" || cppType == "Vector2Short")
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && (" + item.mMemberName + ".x || " + item.mMemberName + ".y < 0))");
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
			else if (cppType == "Vector3" || cppType == "Vector3Int")
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && (" + item.mMemberName + ".x || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0))");
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
			else if (cppType == "Vector4")
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && (" + item.mMemberName + ".x || " + item.mMemberName + ".y < 0 || " + item.mMemberName + ".z < 0 || " + item.mMemberName + ".w < 0))");
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
			// 有符号的基础数据类型的列表
			else if (cppType == "Vector<char>" || cppType == "Vector<short>" || cppType == "Vector<int>" || cppType == "Vector<llong>" ||
					 cppType == "Vector<float>" || cppType == "Vector<double>")
			{
				string elementType = getElementTypeCpp(cppType);
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tfor (const " + elementType + " item : " + item.mMemberName + ")");
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
					generateCodes.push_back("\t\tfor (const " + elementType + " item : " + item.mMemberName + ")");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tif (item < 0)");
					generateCodes.push_back("\t\t\t{");
					generateCodes.push_back("\t\t\t\treturn true;");
					generateCodes.push_back("\t\t\t}");
					generateCodes.push_back("\t\t}");
				}
			}
			// 无符号的基础数据类型的列表
			else if (cppType == "Vector<string>" || cppType == "Vector<byte>" || cppType == "Vector<ushort>" || cppType == "Vector<uint>" || cppType == "Vector<ullong>")
			{
				// 无符号的不做处理
			}
			// 自定义数据类型的列表
			else if (startWith(cppType, "Vector<"))
			{
				string elementType = getElementTypeCpp(cppType);
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0)");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tfor (const " + elementType + "& item : " + item.mMemberName + ")");
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
					generateCodes.push_back("\t\tfor (const " + elementType + "& item : " + item.mMemberName + ")");
					generateCodes.push_back("\t\t{");
					generateCodes.push_back("\t\t\tif (item.hasSign())");
					generateCodes.push_back("\t\t\t{");
					generateCodes.push_back("\t\t\t\treturn true;");
					generateCodes.push_back("\t\t\t}");
					generateCodes.push_back("\t\t}");
				}
			}
			// 自定义数据类型
			else
			{
				if (item.mOptional)
				{
					generateCodes.push_back("\t\tif ((mFieldFlag & (1ULL << (byte)Field::" + item.mMemberNameNoPrefix + ")) != 0 && " + item.mMemberName + ".hasSign())");
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

		// resetProperty
		generateCodes.push_back("\tvoid resetProperty() override");
		generateCodes.push_back("\t{");
		generateCodes.push_back("\t\tbase::resetProperty();");
		int startLineCount = generateCodes.size();
		for (const PacketMember& item : packetInfo.mMemberList)
		{
			if (item.mTypeName == "string" || 
				startWith(item.mTypeName, "Vector<") || 
				item.mTypeName == "Vector2" || 
				item.mTypeName == "Vector2UShort" || 
				item.mTypeName == "Vector2UInt" || 
				item.mTypeName == "Vector2Int" || 
				item.mTypeName == "Vector3" || 
				item.mTypeName == "Vector4")
			{
				generateCodes.push_back("\t\t" + item.mMemberName + ".clear();");
			}
			else if (item.mTypeName == "bool")
			{
				generateCodes.push_back("\t\t" + item.mMemberName + " = false;");
			}
			else if (item.mTypeName == "float" || item.mTypeName == "double")
			{
				generateCodes.push_back("\t\t" + item.mMemberName + " = 0.0f;");
			}
			else if (isPodInteger(item.mTypeName))
			{
				generateCodes.push_back("\t\t" + item.mMemberName + " = 0;");
			}
			else
			{
				generateCodes.push_back("\t\t" + item.mMemberName + ".resetProperty();");
			}
		}
		generateCodes.push_back("\t}");
	}
	else
	{
		generateCodes.push_back("\tbool readFromBuffer(SerializerBitRead* reader, const bool needReadSign) override { return true; }");
		generateCodes.push_back("\tbool writeToBuffer(SerializerBitWrite* writer, const bool needWriteSign) const override { return true; }");
		generateCodes.push_back("\tbool generateHasSignInternal() const override { return false; }");
		generateCodes.push_back("\tvoid resetProperty() override");
		generateCodes.push_back("\t{");
		generateCodes.push_back("\t\tbase::resetProperty();");
		generateCodes.push_back("\t}");
	}
}

// SCPacket.h文件
void CodeNetPacket_Server::generateCppSCPacketFileHeader(const PacketInfo& packetInfo, const string& filePath)
{
	const string& packetName = packetInfo.mPacketName;
	if (!startWith(packetName, "SC"))
	{
		return;
	}

	bool hasOptional = false;
	for (const auto& item : packetInfo.mMemberList)
	{
		if (item.mOptional)
		{
			hasOptional = true;
			break;
		}
	}

	myVector<string> generateCodes;
	generateCodes.push_back(packetInfo.mComment);
	generateCodes.push_back("class " + packetName + " : public PacketTCP");
	generateCodes.push_back("{");
	generateCodes.push_back("\tBASE(" + packetName + ", PacketTCP);");
	if (hasOptional)
	{
		generateCodes.push_back("public:");
		generateCodes.push_back("\tenum class Field : byte");
		generateCodes.push_back("\t{");
		FOR_I(packetInfo.mMemberList.size())
		{
			const auto& item = packetInfo.mMemberList[i];
			if (item.mOptional)
			{
				if (i >= 64)
				{
					ERROR("可选字段的下标不能超过63");
					break;
				}
				generateCodes.push_back("\t\t" + item.mMemberNameNoPrefix + " = " + IToS(i) + ",");
			}
		}
		generateCodes.push_back("\t};");
	}
	generateCodes.push_back("public:");
	generateCppPacketMemberDeclare(packetInfo.mMemberList, generateCodes);
	generateCodes.push_back("private:");
	generateCodes.push_back("\tstatic " + packetName + " mStaticObject;");
	generateCodes.push_back("\tstatic string mPacketName;");
	generateCodes.push_back("public:");
	generateCodes.push_back("\t" + packetName + "()");
	generateCodes.push_back("\t{");
	generateCodes.push_back("\t\tmType = PACKET_TYPE::" + packetName + ";");
	generateCodes.push_back("\t\tmShowInfo = " + boolToString(packetInfo.mShowInfo) + ";");
	generateCodes.push_back("\t}");
	generateCodes.push_back("\tstatic " + packetName + "& get()");
	generateCodes.push_back("\t{");
	generateCodes.push_back("\t\tmStaticObject.resetProperty();");
	generateCodes.push_back("\t\treturn mStaticObject;");
	generateCodes.push_back("\t}");
	generateCodes.push_back("\tconst string& getPacketName() override { return mPacketName; }");
	generateCodes.push_back("\tstatic const string& getStaticPacketName() { return mPacketName; }");
	generateCodes.push_back("\tstatic constexpr ushort getStaticType() { return PACKET_TYPE::" + packetName + "; }");
	if (packetInfo.mMemberList.size() > 0)
	{
		generateCodes.push_back("\tstatic constexpr bool hasMember() { return true; }");
	}
	else
	{
		generateCodes.push_back("\tstatic constexpr bool hasMember() { return false; }");
	}
	generateCppPacketReadWrite(packetInfo, generateCodes);

	string headerFullPath = filePath + packetName + ".h";
	if (isFileExist(headerFullPath))
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(headerFullPath, codeList, lineStart,
			[](const string& codeLine) { return codeLine == "// auto generate start"; },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }))
		{
			return;
		}
		for (const string& line : generateCodes)
		{
			codeList.insert(++lineStart, line);
		}
		writeFile(headerFullPath, codeList);
	}
	else
	{
		myVector<string> codeList;
		codeList.push_back("#pragma once");
		codeList.push_back("");
		codeList.push_back("#include \"PacketTCP.h\"");
		codeList.push_back("#include \"GamePacketDefine.h\"");
		codeList.push_back("");
		codeList.push_back("// auto generate start");
		codeList.addRange(generateCodes);
		codeList.push_back("\t// auto generate end");
		codeList.push_back("\tvoid debugInfo(MyString<1024>& buffer) override");
		codeList.push_back("\t{");
		codeList.push_back("\t\tdebug(buffer, \"\");");
		codeList.push_back("\t}");
		codeList.push_back("\tstatic void send(CharacterPlayer* player);");
		codeList.push_back("};");
		writeFile(headerFullPath, codeList);
	}
}

// SCPacket.cpp
void CodeNetPacket_Server::generateCppSCPacketFileSource(const PacketInfo& packetInfo, const string& filePath)
{
	const string& packetName = packetInfo.mPacketName;
	if (!startWith(packetName, "SC"))
	{
		return;
	}
	string cppFullPath = filePath + packetName + ".cpp";
	if (isFileExist(cppFullPath))
	{
		myVector<string> codeList;
		int lineStart = -1;
		if (!findCustomCode(cppFullPath, codeList, lineStart,
			[](const string& codeLine) { return codeLine == "// auto generate start"; },
			[](const string& codeLine) { return endWith(codeLine, "// auto generate end"); }))
		{
			return;
		}
		codeList.insert(++lineStart, packetName + " " + packetName + "::mStaticObject;");
		codeList.insert(++lineStart, "string " + packetName + "::mPacketName = STR(" + packetName + ");");
		writeFile(cppFullPath, codeList);
	}
	else
	{
		myVector<string> codeList;
		codeList.push_back("#include \"GameHeader.h\"");
		codeList.push_back("");
		codeList.push_back("// auto generate start");
		codeList.push_back(packetName + " " + packetName + "::mStaticObject;");
		codeList.push_back("string " + packetName + "::mPacketName = STR(" + packetName + ");");
		codeList.push_back("// auto generate end");
		codeList.push_back("");
		codeList.push_back("void " + packetName + "::send(CharacterPlayer* player)");
		codeList.push_back("{");
		codeList.push_back("\t" + packetName + "& packet = get();");
		codeList.push_back("\tsendPacketTCP(&packet, player->getClient());");
		codeList.push_back("}");
		writeFile(cppFullPath, codeList);
	}
}

myVector<string> CodeNetPacket_Server::multiMemberReadLine(const myVector<string>& memberNameList, const string& memberType, bool supportCustom)
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

	myVector<string> list;
	if (memberType == "string")
	{
		FOR_VECTOR(memberNameList)
		{
			list.push_back("success = success && reader->readString(" + memberNameList[i] + ");");
		}
	}
	else if (startWith(memberType, "Vector<"))
	{
		const string elementType = getElementTypeCpp(memberType);
		FOR_VECTOR(memberNameList)
		{
			if (elementType == "string")
			{
				list.push_back("success = success && reader->readStringList(" + memberNameList[i] + ");");
			}
			else if (elementType == "bool")
			{
				ERROR("不支持bool的列表");
			}
			else if (elementType == "char" || elementType == "short" || elementType == "int" || elementType == "llong")
			{
				list.push_back("success = success && reader->readSignedList(needReadSign, " + memberNameList[i] + ");");
			}
			else if (elementType == "byte" || elementType == "ushort" || elementType == "uint" || elementType == "ullong")
			{
				list.push_back("success = success && reader->readUnsignedList(" + memberNameList[i] + ");");
			}
			else if (elementType == "float")
			{
				list.push_back("success = success && reader->readFloatList(needReadSign, " + memberNameList[i] + ");");
			}
			else if (elementType == "double")
			{
				list.push_back("success = success && reader->readDoubleList(needReadSign, " + memberNameList[i] + ");");
			}
			else
			{
				if (supportCustom)
				{
					list.push_back("success = success && reader->readCustomList(needReadSign, " + memberNameList[i] + ");");
				}
				else
				{
					ERROR("不支持自定义结构体:" + memberType);
				}
			}
		}
	}
	else if (memberType == "bool")
	{
		FOR_VECTOR(memberNameList)
		{
			list.push_back("success = success && reader->readBool(" + memberNameList[i] + ");");
		}
	}
	else if (memberType == "char" || memberType == "short" || memberType == "int" || memberType == "llong")
	{
		list.push_back("success = success && reader->readSigned(needReadSign, " + members + ");");
	}
	else if (memberType == "byte" || memberType == "ushort" || memberType == "uint" || memberType == "ullong")
	{
		list.push_back("success = success && reader->readUnsigned(" + members + ");");
	}
	else if (memberType == "float")
	{
		list.push_back("success = success && reader->readFloat(needReadSign, " + members + ");");
	}
	else if (memberType == "double")
	{
		list.push_back("success = success && reader->readDouble(needReadSign, " + members + ");");
	}
	else
	{
		if (supportCustom)
		{
			FOR_VECTOR(memberNameList)
			{
				list.push_back("success = success && reader->readCustom(needReadSign, " + memberNameList[i] + ");");
			}
		}
		else
		{
			ERROR("不支持自定义结构体:" + memberType);
		}
	}
	return list;
}

myVector<string> CodeNetPacket_Server::multiMemberWriteLine(const myVector<string>& memberNameList, const string& memberType, bool supportCustom)
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

	myVector<string> list;
	if (memberType == "string")
	{
		FOR_VECTOR(memberNameList)
		{
			list.push_back("success = success && writer->writeString(" + memberNameList[i] + ");");
		}
	}
	else if (startWith(memberType, "Vector<"))
	{
		const string elementType = getElementTypeCpp(memberType);
		FOR_VECTOR(memberNameList)
		{
			if (elementType == "string")
			{
				list.push_back("success = success && writer->writeStringList(" + memberNameList[i] + ");");
			}
			else if (elementType == "bool")
			{
				ERROR("不支持bool的列表");
			}
			else if (elementType == "char" || elementType == "short" || elementType == "int" || elementType == "llong")
			{
				list.push_back("success = success && writer->writeSignedList(needWriteSign, " + memberNameList[i] + ");");
			}
			else if (elementType == "byte" || elementType == "ushort" || elementType == "uint" || elementType == "ullong")
			{
				list.push_back("success = success && writer->writeUnsignedList(" + memberNameList[i] + ");");
			}
			else if (elementType == "float")
			{
				list.push_back("success = success && writer->writeFloatList(needWriteSign, " + memberNameList[i] + ");");
			}
			else if (elementType == "double")
			{
				list.push_back("success = success && writer->writeDoubleList(needWriteSign, " + memberNameList[i] + ");");
			}
			else
			{
				if (supportCustom)
				{
					list.push_back("success = success && writer->writeCustomList(needWriteSign, " + memberNameList[i] + ");");
				}
				else
				{
					ERROR("不支持自定义结构体:" + memberType);
				}
			}
		}
	}
	else if (memberType == "bool")
	{
		FOR_VECTOR(memberNameList)
		{
			list.push_back("success = success && writer->writeBool(" + memberNameList[i] + ");");
		}
	}
	else if (memberType == "char" || memberType == "short" || memberType == "int" || memberType == "llong")
	{
		list.push_back("success = success && writer->writeSigned(needWriteSign, " + members + ");");
	}
	else if (memberType == "byte" || memberType == "ushort" || memberType == "uint" || memberType == "ullong")
	{
		list.push_back("success = success && writer->writeUnsigned(" + members + ");");
	}
	else if (memberType == "float")
	{
		list.push_back("success = success && writer->writeFloat(needWriteSign, " + members + ");");
	}
	else if (memberType == "double")
	{
		list.push_back("success = success && writer->writeDouble(needWriteSign, " + members + ");");
	}
	else
	{
		if (supportCustom)
		{
			FOR_VECTOR(memberNameList)
			{
				list.push_back("success = success && writer->writeCustom(needWriteSign, " + memberNameList[i] + ");");
			}
		}
		else
		{
			ERROR("不支持自定义结构体:" + memberType);
		}
	}
	return list;
}

string CodeNetPacket_Server::singleMemberReadLine(const string& memberName, const string& memberType, bool supportCustom)
{
	if (memberType == "string")
	{
		return "success = success && reader->readString(" + memberName + ");";
	}
	else if (startWith(memberType, "Vector<"))
	{
		const string elementType = getElementTypeCpp(memberType);
		if (elementType == "string")
		{
			return "success = success && reader->readStringList(" + memberName + ");";
		}
		else if (elementType == "bool")
		{
			ERROR("不支持bool的列表");
			return "";
		}
		else if (elementType == "char" || elementType == "short" || elementType == "int" || elementType == "llong")
		{
			return "success = success && reader->readSignedList(needReadSign, " + memberName + ");";
		}
		else if (elementType == "byte" || elementType == "ushort" || elementType == "uint" || elementType == "ullong")
		{
			return "success = success && reader->readUnsignedList(" + memberName + ");";
		}
		else if (elementType == "float")
		{
			return "success = success && reader->readFloatList(needReadSign, " + memberName + ");";
		}
		else if (elementType == "double")
		{
			return "success = success && reader->readDoubleList(needReadSign, " + memberName + ");";
		}
		else
		{
			if (supportCustom)
			{
				return "success = success && reader->readCustomList(needReadSign, " + memberName + ");";
			}
			else
			{
				ERROR("不支持自定义结构体:" + memberType);
			}
		}
	}
	else if (memberType == "bool")
	{
		return "success = success && reader->readBool(" + memberName + ");";
	}
	else if (memberType == "char" || memberType == "short" || memberType == "int" || memberType == "llong")
	{
		return "success = success && reader->readSigned(needReadSign, " + memberName + ");";
	}
	else if (memberType == "byte" || memberType == "ushort" || memberType == "uint" || memberType == "ullong")
	{
		return "success = success && reader->readUnsigned(" + memberName + ");";
	}
	else if (memberType == "float")
	{
		return "success = success && reader->readFloat(needReadSign, " + memberName + ");";
	}
	else if (memberType == "double")
	{
		return "success = success && reader->readDouble(needReadSign, " + memberName + ");";
	}
	else if (memberType == "Vector2")
	{
		return "success = success && reader->readVector2(needReadSign, " + memberName + ");";
	}
	else if (memberType == "Vector2UShort")
	{
		return "success = success && reader->readVector2UShort(" + memberName + ");";
	}
	else if (memberType == "Vector2UInt")
	{
		return "success = success && reader->readVector2UInt(" + memberName + ");";
	}
	else if (memberType == "Vector2Int")
	{
		return "success = success && reader->readVector2Int(needReadSign, " + memberName + ");";
	}
	else if (memberType == "Vector3")
	{
		return "success = success && reader->readVector3(needReadSign, " + memberName + ");";
	}
	else if (memberType == "Vector4")
	{
		return "success = success && reader->readVector4(needReadSign, " + memberName + ");";
	}
	if (supportCustom)
	{
		return "success = success && reader->readCustom(needReadSign, " + memberName + ");";
	}
	else
	{
		ERROR("不支持自定义结构体:" + memberType);
		return "";
	}
}

string CodeNetPacket_Server::singleMemberWriteLine(const string& memberName, const string& memberType, bool supportCustom)
{
	if (memberType == "string")
	{
		return "success = success && writer->writeString(" + memberName + ");";
	}
	else if (startWith(memberType, "Vector<"))
	{
		const string elementType = getElementTypeCpp(memberType);
		if (elementType == "string")
		{
			return "success = success && writer->writeStringList(" + memberName + ");";
		}
		else if (elementType == "bool")
		{
			ERROR("不支持bool的列表");
			return "";
		}
		else if (elementType == "char" || elementType == "short" || elementType == "int" || elementType == "llong")
		{
			return "success = success && writer->writeSignedList(needWriteSign, " + memberName + ");";
		}
		else if (elementType == "byte" || elementType == "ushort" || elementType == "uint" || elementType == "ullong")
		{
			return "success = success && writer->writeUnsignedList(" + memberName + ");";
		}
		else if (elementType == "float")
		{
			return "success = success && writer->writeFloatList(needWriteSign, " + memberName + ");";
		}
		else if (elementType == "double")
		{
			return "success = success && writer->writeDoubleList(needWriteSign, " + memberName + ");";
		}
		else
		{
			if (supportCustom)
			{
				return "success = success && writer->writeCustomList(needWriteSign, " + memberName + ");";
			}
			else
			{
				ERROR("不支持自定义结构体:" + memberName);
			}
		}
	}
	else if (memberType == "bool")
	{
		return "success = success && writer->writeBool(" + memberName + ");";
	}
	else if (memberType == "char" || memberType == "short" || memberType == "int" || memberType == "llong")
	{
		return "success = success && writer->writeSigned(needWriteSign, " + memberName + ");";
	}
	else if (memberType == "byte" || memberType == "ushort" || memberType == "uint" || memberType == "ullong")
	{
		return "success = success && writer->writeUnsigned(" + memberName + ");";
	}
	else if (memberType == "float")
	{
		return "success = success && writer->writeFloat(needWriteSign, " + memberName + ");";
	}
	else if (memberType == "double")
	{
		return "success = success && writer->writeDouble(needWriteSign, " + memberName + ");";
	}
	else if (memberType == "Vector2")
	{
		return "success = success && writer->writeVector2(needWriteSign, " + memberName + ");";
	}
	else if (memberType == "Vector2UInt")
	{
		return "success = success && writer->writeVector2UInt(" + memberName + ");";
	}
	else if (memberType == "Vector2Int")
	{
		return "success = success && writer->writeVector2Int(needWriteSign, " + memberName + ");";
	}
	else if (memberType == "Vector2UShort")
	{
		return "success = success && writer->writeVector2UShort(" + memberName + ");";
	}
	else if (memberType == "Vector3")
	{
		return "success = success && writer->writeVector3(needWriteSign, " + memberName + ");";
	}
	else if (memberType == "Vector4")
	{
		return "success = success && writer->writeVector4(needWriteSign, " + memberName + ");";
	}
	if (supportCustom)
	{
		return "success = success && writer->writeCustom(needWriteSign, " + memberName + ");";
	}
	else
	{
		ERROR("不支持自定义结构体:" + memberName);
		return "";
	}
}

string CodeNetPacket_Server::expandMembersInGroup(const myVector<PacketMember>& memberList, myVector<string>& memberNameList)
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
			memberNameList.push_back(memberName + ".x");
			memberNameList.push_back(memberName + ".y");
		}
		else if (typeName == "Vector3" || typeName == "Vector3Int")
		{
			memberNameList.push_back(memberName + ".x");
			memberNameList.push_back(memberName + ".y");
			memberNameList.push_back(memberName + ".z");
		}
		else if (typeName == "Vector4")
		{
			memberNameList.push_back(memberName + ".x");
			memberNameList.push_back(memberName + ".y");
			memberNameList.push_back(memberName + ".z");
			memberNameList.push_back(memberName + ".w");
		}
		else
		{
			memberNameList.push_back(memberName);
		}
	}
	return toPODType(memberList[0].mTypeName);
}