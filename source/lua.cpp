#include <GarrysMod/Lua/LuaInterface.h>
#include <GarrysMod/FactoryLoader.hpp>
#include "lua.h"
#include <GarrysMod/InterfacePointers.hpp>
#include "filesystem.h"

IServer* Server;
bool Lua::Hooks::OnSetSignonState(int userID, int state, int spawncount) // Return true to block it. You would need to block SIGNONSTATE_PRESPAWN to block it from spawning the player.
{
	if (Lua::PushHook("PlayerQueue:OnSetSignonState"))
	{
		if (g_Lua->CallFunctionProtected(0, 1, true)) // Arg1 = Arguments, Arg2 = Returns, Arg3 = Show Error
		{
			bool ret = g_Lua->GetBool(-1);
			g_Lua->Pop(1);

			return ret;
		}
	}

	return false;
}

LUA_FUNCTION_STATIC(SetSignOnState)
{
	int userID = LUA->CheckNumber(1);
	int state = LUA->CheckNumber(2);
	IClient* cl = (IClient*)Server->GetClient(userID);
	if ( cl != NULL )
	{
		Detours::Function::SetSignOnState(cl, state, -1); // It could crash :/ Had a solution for it somewhere. I think it was in the HttpServer repo in the receive whitelist
	}

	return 0;
}

bool Lua::PushHook(const char* hook)
{
	g_Lua->PushSpecial(SPECIAL_GLOB);
		g_Lua->GetField(-1, "hook");
		if (g_Lua->GetType(-1) != Type::Table)
		{
			g_Lua->Pop(2);
			DevMsg("Missing hook table!\n");
			return false;
		}

			g_Lua->GetField(-1, "Run");
			if (g_Lua->GetType(-1) != Type::Function)
			{
				g_Lua->Pop(3);
				DevMsg("Missing hook.Run function!\n");
				return false;
			} else {
				int reference = g_Lua->ReferenceCreate();
				g_Lua->Pop(2);
				g_Lua->ReferencePush(reference);
				g_Lua->ReferenceFree(reference);
			}

			g_Lua->PushString(hook);

	return true;
}


void Lua::Init(GarrysMod::Lua::ILuaBase* LUA)
{
	g_Lua = (GarrysMod::Lua::ILuaInterface*)LUA;

	Start_Table();
		Add_Func(SetSignOnState, "SetSignOnState");
	Finish_Table("PlayerQueue");
}

void Lua::ServerInit()
{
	if (g_Lua == nullptr) { return; }

	Server = InterfacePointers::Server();

	if (PushHook("PlayerQueue:Initialize"))
	{
		g_Lua->CallFunctionProtected(0, 0, true);
	}
}

void Lua::Shutdown()
{
	// Do we need anything?
}

static SourceSDK::FactoryLoader luashared_loader("lua_shared");
GarrysMod::Lua::ILuaInterface* Lua::GetRealm(unsigned char realm) {
	GarrysMod::Lua::ILuaShared* LuaShared = (GarrysMod::Lua::ILuaShared*)luashared_loader.GetFactory()(GMOD_LUASHARED_INTERFACE, nullptr);
	if (LuaShared == nullptr) {
		Msg("failed to get ILuaShared!\n");
		return nullptr;
	}

	return LuaShared->GetLuaInterface(realm);
}