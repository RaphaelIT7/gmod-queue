#include <GarrysMod/Lua/LuaInterface.h>
#include <GarrysMod/FactoryLoader.hpp>
#include "lua.h"
#include <GarrysMod/InterfacePointers.hpp>
#include "filesystem.h"
#include "unordered_map"
#include "iclient.h"

IServer* Server;
int iSlot = 0;
std::unordered_map<int, IClient*> pClients;
void FreeID(int playerSlot)
{
	auto it = pClients.find(playerSlot);
	if ( it != pClients.end() )
	{
		pClients.erase(playerSlot);
	}
}

bool Lua::Hooks::OnSetSignonState(IClient* cl, int state, int spawncount) // Return true to block it. You would need to block SIGNONSTATE_PRESPAWN to block it from spawning the player.
{
	if (Lua::PushHook("PlayerQueue:OnSetSignonState"))
	{
		++iSlot;
		pClients[iSlot] = cl;

		g_Lua->PushNumber(iSlot);
		g_Lua->PushNumber(state);
		g_Lua->PushNumber(spawncount);
		if (g_Lua->CallFunctionProtected(4, 1, true)) // Arg1 = Arguments, Arg2 = Returns, Arg3 = Show Error
		{
			bool ret = false;
			if ( g_Lua->GetType(-1) != GarrysMod::Lua::Type::Nil )
				ret = g_Lua->GetBool(-1);

			if ( !ret )
				FreeID( iSlot );

			g_Lua->Pop(1);

			return ret;
		}
	}

	return false;
}

LUA_FUNCTION_STATIC(SetSignOnState)
{
	int playerSlot = LUA->CheckNumber(1);
	int state = LUA->CheckNumber(2);

	auto it = pClients.find(playerSlot);
	if ( it == pClients.end() )
	{
		LUA->PushBool(false);
	} else {
		Detours::Function::SetSignOnState(it->second, state, -1);
		FreeID(it->first);
		LUA->PushBool(true);
	}

	return 1;
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
				g_Lua->PushString(hook);
			}

	return true;
}

void Lua::Init(GarrysMod::Lua::ILuaBase* LUA)
{
	g_Lua = (GarrysMod::Lua::ILuaInterface*)LUA;

	Start_Table();
		Add_Func(SetSignOnState, "SetSignOnState"); // NOTE: After calling this, the ID the hook gave you cannot be used again!
	Finish_Table("PlayerQueue");
	Msg("Pushed PlayerQueue\n");
}

void Lua::ServerInit()
{
	if (g_Lua == nullptr) { Msg("Lua is Null?\n"); return; }

	Server = InterfacePointers::Server();

	if (PushHook("PlayerQueue:Initialize"))
	{
		g_Lua->CallFunctionProtected(1, 0, true);
		Msg("Init PlayerQueue\n");
	} else { Msg("failed PlayerQueue\n"); }
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