#pragma once

#include <GarrysMod/Lua/LuaShared.h>
#include "util.h"

namespace Lua {
	namespace Hooks {
		extern bool OnSetSignonState(int userID, int state, int spawncount);
	}

	extern bool PushHook(const char* pName);
	extern void Init(GarrysMod::Lua::ILuaBase* LUA);
	extern void ServerInit();
	extern void Shutdown();
	extern GarrysMod::Lua::ILuaInterface* GetRealm(unsigned char);
}