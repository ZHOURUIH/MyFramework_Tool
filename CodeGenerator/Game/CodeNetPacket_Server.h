#pragma once

#include "CodeNetPacket.h"

class CodeNetPacket_Server : public CodeNetPacket
{
public:
	static void generateCpp(const myVector<PacketStruct>& structInfoList, const myVector<PacketInfo>& packetInfoList);
	static void generateCppGamePacketDefineFile(const myVector<PacketInfo>& packetList, const string& filePath);
	static void generateCppGamePacketRegisteFile(const myVector<PacketInfo>& packetList, const myVector<PacketStruct>& structInfoList, const string& filePath);
	static void generateCppCSPacketFileHeader(const PacketInfo& packetInfo, const string& filePath);
	static void generateCppCSPacketFileSource(const PacketInfo& packetInfo, const string& filePath);
	static void generateCppSCPacketFileHeader(const PacketInfo& packetInfo, const string& filePath);
	static void generateCppSCPacketFileSource(const PacketInfo& packetInfo, const string& filePath);
	static void generateCppPacketMemberDeclare(const myVector<PacketMember>& memberList, myVector<string>& generateCodes);
	static void generateCppPacketReadWrite(const PacketInfo& packetInfo, myVector<string>& generateCodes);
	static void generateCppStruct(const PacketStruct& structInfo, const string& filePath);
protected:
	static myVector<string> multiMemberReadLine(const myVector<string>& memberNameList, const string& memberType, bool supportCustom);
	static myVector<string> multiMemberWriteLine(const myVector<string>& memberNameList, const string& memberType, bool supportCustom);
	static string singleMemberReadLine(const string& memberName, const string& memberType, bool supportCustom);
	static string singleMemberWriteLine(const string& memberName, const string& memberType, bool supportCustom);
	static string expandMembersInGroup(const myVector<PacketMember>& memberList, myVector<string>& memberNameList);
};