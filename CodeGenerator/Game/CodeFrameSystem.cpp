#include "CodeFrameSystem.h"
#include "CodeFrameSystem_Server.h"

void CodeFrameSystem::generate()
{
	if (cppGamePath.empty() || cppFramePath.empty())
	{
		return;
	}
	print("正在生成框架组件");
	CodeFrameSystem_Server::generateFrameSystem(cppGamePath, "Common/GameBase.h", "Game/Game.cpp", "GameBase", "");
	CodeFrameSystem_Server::generateFrameSystem(cppFramePath, "Common/FrameBase.h", "ServerFramework/ServerFramework.cpp", "FrameBase", "MICRO_LEGEND_FRAME_API ");
	print("完成生成框架组件");
	print("");
}