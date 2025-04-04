#include <GarrysMod/Symbol.hpp>
#include "Platform.hpp"
#include <vector>

class SVC_ServerInfo;

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
 * The symbols will have this order:
 * 0 - Linux 32x
 * 1 - Linux 64x
 * 2 - Windows 32x
 * 3 - Windows 64x
 */

namespace Symbols
{
	// The ENTIRE SignOnState stuff seems to be different on 64x....
	// Idk if it's even possible to implement this on 64x
	typedef bool (*CBaseClient_SetSignonState)(void* client, int state, int spawncount);
	const std::vector<Symbol> CBaseClient_SetSignonStateSym = {
		Symbol::FromName("_ZN11CBaseClient14SetSignonStateEii"),
	};

	typedef void (*CServerGameClients_GetPlayerLimit)(void* dll, int& minplayers, int& mayplayers, int& defaultmaxplayers);
	const std::vector<Symbol> CServerGameClients_GetPlayerLimitSym = {
		Symbol::FromName("_ZNK18CServerGameClients15GetPlayerLimitsERiS0_S0_"),
	};

	typedef void (*CBaseServer_FillServerInfo)(void* srv, SVC_ServerInfo& info);
	const std::vector<Symbol> CBaseServer_FillServerInfoSym = {
		Symbol::FromName("_ZN11CBaseServer14FillServerInfoER14SVC_ServerInfo"),
	};

	typedef void (*CGameClient_SpawnPlayer)(void* client);
	const std::vector<Symbol> CGameClient_SpawnPlayerSym = {
		Symbol::FromName("_ZN11CGameClient11SpawnPlayerEv"),
	};

	typedef void (GMCOMMON_CALLING_CONVENTION* CSteam3Server_NotifyClientDisconnect)(void* server, CBaseClient* client);
	const std::vector<Symbol> CSteam3Server_NotifyClientDisconnectSym = { // 64x = Search for "S3" and then go through every function upwards till you find one that somewhat matches the ASM of the 32x version.
		Symbol::FromName("_ZN13CSteam3Server22NotifyClientDisconnectEP11CBaseClient"),
		Symbol::FromSignature("\x55\x48\x89\xE5\x41\x54\x53\x48\x89\xF3\x48\x83\xEC\x20\x48\x85\xF6"), // 55 48 89 E5 41 54 53 48 89 F3 48 83 EC 20 48 85 F6
	};
}