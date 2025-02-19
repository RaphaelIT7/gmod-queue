#include <GarrysMod/Symbol.hpp>
#include "Platform.hpp"
#include <vector>

class SVC_ServerInfo;

/*
 * The symbols will have this order:
 * 0 - Linux 32x
 * 1 - Linux 64x
 * 2 - Windows 32x
 * 3 - Windows 64x
 */

#if defined SYSTEM_WINDOWS
#if defined ARCHITECTURE_X86_64
#define GMCOMMON_CALLING_CONVENTION __fastcall
#else
#define GMCOMMON_CALLING_CONVENTION __thiscall
#endif
#else
#define GMCOMMON_CALLING_CONVENTION
#endif

 /*
  * I need to figure out how to hook into shit without breaking it on windows.
  * Currently every single hook seems to break gmod and every call to a hook seems broken.
  * It's probably some shit with calling convention or so but I have no fking Idea how to solve that :<
  * If someone knows how I could fix this, please let me know.
  * 
  * Update: First, class hooks, don't have "this"/the first argument meaning you require a ClassProxy
  */

namespace Symbols
{
	typedef bool (*CBaseClient_SendServerInfo)(void* client);
	const Symbol CBaseClient_SendServerInfoSym = Symbol::FromName("_ZN11CBaseClient14SendServerInfoEv");

	typedef bool (*CBaseClient_SetSignonState)(void* client, int state, int spawncount);
	const Symbol CBaseClient_SetSignonStateSym = Symbol::FromName("_ZN11CBaseClient14SetSignonStateEii");

	typedef const char* (*CBaseClient_GetNetworkIDString)(void* client);
	const Symbol CBaseClient_GetNetworkIDStringSym = Symbol::FromName("_ZNK11CBaseClient18GetNetworkIDStringEv");

	typedef void (*CBaseServer_SendPendingServerInfo)(void* srv);
	const Symbol CBaseServer_SendPendingServerInfoSym = Symbol::FromName("_ZN11CBaseServer21SendPendingServerInfoEv");

	typedef void (*CServerGameClients_GetPlayerLimit)(void* dll, int& minplayers, int& mayplayers, int& defaultmaxplayers);
	const Symbol CServerGameClients_GetPlayerLimitSym = Symbol::FromName("_ZNK18CServerGameClients15GetPlayerLimitsERiS0_S0_");

	typedef void (*CBaseServer_FillServerInfo)(void* srv, SVC_ServerInfo& info);
	const Symbol CBaseServer_FillServerInfoSym = Symbol::FromName("_ZN11CBaseServer14FillServerInfoER14SVC_ServerInfo");

	typedef void (*CGameClient_SpawnPlayer)(void* client);
	extern const std::vector<Symbol> CGameClient_SpawnPlayerSym;
}