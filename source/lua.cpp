#include <GarrysMod/Lua/LuaInterface.h>
#include <GarrysMod/FactoryLoader.hpp>
#include "lua.h"
#include <GarrysMod/InterfacePointers.hpp>

bool Lua::PushHook(const char* hook)
{
	if ( !g_Lua )
	{
		Warning("playerqueue: Lua::PushHook was called while g_Lua was NULL! (%s)\n", hook);
		return false;
	}

	if (!ThreadInMainThread())
	{
		Warning("playerqueue: Lua::PushHook was called ouside of the main thread! (%s)\n", hook);
		return false;
	}

	g_Lua->GetField(LUA_GLOBALSINDEX, "hook");
		if (g_Lua->GetType(-1) != GarrysMod::Lua::Type::Table)
		{
			g_Lua->Pop(1);
			DevMsg("playerqueue: Missing hook table!\n");
			return false;
		}

		g_Lua->GetField(-1, "Run");
			if (g_Lua->GetType(-1) != GarrysMod::Lua::Type::Function)
			{
				g_Lua->Pop(2);
				DevMsg("playerqueue: Missing hook.Run function!\n");
				return false;
			} else {
				g_Lua->Remove(-2);
				g_Lua->PushString(hook);
			}

	return true;
}

void Lua::Init(GarrysMod::Lua::ILuaBase* LUA)
{
	g_Lua = (GarrysMod::Lua::ILuaInterface*)LUA;
}

void Lua::ServerInit()
{
	if (g_Lua == nullptr)
	{
		Warning("playerqueue: g_Lua is NULL!\n");
		return;
	}
}

void Lua::Shutdown()
{
}

static SourceSDK::FactoryLoader luashared_loader("lua_shared");
GarrysMod::Lua::ILuaInterface* Lua::GetRealm(unsigned char realm) {
	GarrysMod::Lua::ILuaShared* LuaShared = (GarrysMod::Lua::ILuaShared*)luashared_loader.GetFactory()(GMOD_LUASHARED_INTERFACE, nullptr);
	if (LuaShared == nullptr) {
		Msg("playerqueue: failed to get ILuaShared!\n");
		return nullptr;
	}

	return LuaShared->GetLuaInterface(realm);
}