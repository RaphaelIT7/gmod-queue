#pragma once

#include <GarrysMod/Lua/LuaInterface.h>
#include <GarrysMod/Symbol.hpp>
#include <detouring/hook.hpp>
#include <scanning/symbolfinder.hpp>
#include <vector>
#include <server.h>
#define PL_DEBUG 1

extern CGameServer* gsv;

//---------------------------------------------------------------------------------
// Purpose: Base Detours
//---------------------------------------------------------------------------------
typedef bool (*CBaseClient_SendServerInfo)(void* client);
const Symbol CBaseClient_SendServerInfoSym = Symbol::FromName("_ZN11CBaseClient14SendServerInfoEv");

typedef bool (*CBaseClient_SetSignonState)(void* client, int state, int spawncount);
const Symbol CBaseClient_SetSignonStateSym = Symbol::FromName("_ZN11CBaseClient14SetSignonStateEii");

typedef void (*CBaseServer_SendPendingServerInfo)(void* srv);
const Symbol CBaseServer_SendPendingServerInfoSym = Symbol::FromName("_ZN11CBaseServer21SendPendingServerInfoEv");

//---------------------------------------------------------------------------------
// Purpose: Expose/Create everything else
//---------------------------------------------------------------------------------

namespace Detours
{
	extern void Init();
	extern void Shutdown();
	extern void Think();
	extern void CreateDetour(Detouring::Hook*, const char*, Detouring::Hook::Target, void*);
}

template<class T>
void CheckFunction(T func, const char* name)
{
#ifdef PL_DEBUG
	if (func == nullptr) {
		Msg("				Could not locate %s symbol!\n", name);
	} else {
		Msg("			located %s symbol!\n", name);
	}
#endif
}

#ifdef PL_DEBUG
#define DPrint Msg;
#else
#define DPrint {}
#endif