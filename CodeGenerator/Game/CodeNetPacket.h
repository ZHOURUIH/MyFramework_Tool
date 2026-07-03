#pragma once

#include "CodeUtility.h"

class CodeNetPacket : public CodeUtility
{
public:
	static void generate();
	static void generateVirtualClient();
public:
	// ¹¤¾ßº¯Êý
	static bool isSameType(const string& sourceType, const string& curType);
	static string toPODType(const string& type);
	static bool isCustomStructType(const string& type);
	static void generateMemberGroup(const myVector<PacketMember>& memberList, myVector<myVector<PacketMember>>& memberNameList);
	static void parsePacketConfig(myVector<PacketStruct>& structInfoList, myVector<PacketInfo>& packetInfoList);
	static string generatePacketVersion(const myVector<PacketInfo>& packetList, const myVector<PacketStruct>& structInfoList);
};