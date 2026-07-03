#pragma once

#include "CodeNetPacket.h"

class CodeNetPacket_Client : public CodeNetPacket
{
public:
	static void generateCSharp(const myVector<PacketStruct>& structInfoList, const myVector<PacketInfo>& packetInfoList);
	static void generateCSharpVirtualClient(const myVector<PacketStruct>& structInfoList, const myVector<PacketInfo>& packetInfoList);
	static void generateCSharpPacketDefineFile(const myVector<PacketInfo>& packetList, const string& filePath);
	static void generateCSharpPacketRegisteFile(const myVector<PacketInfo>& packetList, const myVector<PacketStruct>& structInfoList, const string& filePath);
	static void generateCSharpPacketFile(const PacketInfo& packetInfo, const string& csFileHotfixPath, const string& scFileHotfixPath);
	static void generateCSharpStruct(const PacketStruct& structInfo, const string& hotFixPath);
protected:
	static string singleMemberReadLineCSharp(const string& memberName, const string& memberType);
	static string singleMemberWriteLineCSharp(const string& memberName, const string& memberType);
	static myVector<string> multiMemberReadLineCSharp(const myVector<string>& memberNameList, const string& memberType, bool supportCustom);
	static myVector<string> multiMemberWriteLineCSharp(const myVector<string>& memberNameList, const string& memberType, bool supportCustom);
	static string expandMembersInGroupCSharp(const myVector<PacketMember>& memberList, myVector<string>& memberNameList, bool supportSimplify);
};