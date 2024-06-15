#include "lua.h"
#include "detours.h"
#include <GarrysMod/InterfacePointers.hpp>
#include <unordered_map>
#include <vprof.h>
#include "iclient.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// Detours.

Detouring::Hook detour_CBaseClient_SetSignonState;
//Detouring::Hook detour_CBaseServer_SendPendingServerInfo;
Detouring::Hook detour_CServerGameClients_GetPlayerLimit;

void Detours::Function::SetSignOnState(IClient* cl, int state, int spawncount)
{
	detour_CBaseClient_SetSignonState.GetTrampoline<CBaseClient_SetSignonState>()(cl, state, spawncount);
}

bool hook_CBaseClient_SetSignonState(IClient* cl, int state, int spawncount)
{
	if (Lua::Hooks::OnSetSignonState(cl->GetUserID(), state, spawncount))
		return true;

	return detour_CBaseClient_SetSignonState.GetTrampoline<CBaseClient_SetSignonState>()(cl, state, spawncount);
}

/*CBaseClient_SendServerInfo func_CBaseClient_SendServerInfo;
void hook_CBaseServer_SendPendingServerInfo(IServer* srv)
{
	if (!srv->IsHLTV())
		return;

	detour_CBaseServer_SendPendingServerInfo.GetTrampoline<CBaseServer_SendPendingServerInfo>()(srv);
}*/

void hook_CServerGameClients_GetPlayerLimit(void* funkyClass, int& minPlayers, int& maxPlayers, int& defaultMaxPlayers)
{
	minPlayers = 1;
	maxPlayers = 255;
	defaultMaxPlayers = 255;
}

void Detours::Think()
{
}

bool pre = false;
std::vector<Detouring::Hook*> detours = {};
void Detours::CreateDetour(Detouring::Hook* hook, const char* name, Detouring::Hook::Target target, void* func)
{
	hook->Create(target, func);
	hook->Enable();

	detours.push_back(hook);

#ifdef PL_DEBUG
	if (hook->IsValid()) {
		Msg("			Valid detour for %s!\n", name);
	} else {
		Msg("				Invalid detour for %s!\n", name);
	}
#endif
}

SymbolFinder symfinder;
void Detours::Init()
{
	Msg("	--- Starting Detours ---\n");

	SourceSDK::ModuleLoader engine_loader("engine");
	void* sv_CBaseClient_SetSignonState = symfinder.Resolve(engine_loader.GetModule(), CBaseClient_SetSignonStateSym.name.c_str(), CBaseClient_SetSignonStateSym.length);
	CheckFunction(sv_CBaseClient_SetSignonState, "CBaseClient::SetSignonState");
	CreateDetour(&detour_CBaseClient_SetSignonState, "CBaseClient::SetSignonState", reinterpret_cast<void*>(sv_CBaseClient_SetSignonState), reinterpret_cast<void*>(&hook_CBaseClient_SetSignonState));

	//void* sv_CBaseServer_SendPendingServerInfo = symfinder.Resolve(engine_loader.GetModule(), CBaseServer_SendPendingServerInfoSym.name.c_str(), CBaseServer_SendPendingServerInfoSym.length);
	//CheckFunction(sv_CBaseServer_SendPendingServerInfo, "CBaseServer::SendPendingServerInfo");
	//CreateDetour(&detour_CBaseServer_SendPendingServerInfo, "CBaseServer::SendPendingServerInfo", reinterpret_cast<void*>(sv_CBaseServer_SendPendingServerInfo), reinterpret_cast<void*>(&hook_CBaseServer_SendPendingServerInfo));

	SourceSDK::ModuleLoader server_loader("server");
	void* sv_CServerGameClients_GetPlayerLimit = symfinder.Resolve(server_loader.GetModule(), CServerGameClients_GetPlayerLimitSym.name.c_str(), CServerGameClients_GetPlayerLimitSym.length);
	CheckFunction(sv_CServerGameClients_GetPlayerLimit, "CServerGameClients::GetPlayerLimit");
	CreateDetour(&detour_CServerGameClients_GetPlayerLimit, "CServerGameClients::GetPlayerLimit", reinterpret_cast<void*>(sv_CServerGameClients_GetPlayerLimit), reinterpret_cast<void*>(&hook_CServerGameClients_GetPlayerLimit));

	Msg("	--- Finished Detours ---\n");

	// Function loader
	Msg("	--- Function loader ---\n");

	Msg("	--- Finished loading functions ---\n");
}

void Detours::Shutdown()
{
	for (Detouring::Hook* hook : detours) {
		hook->Disable();
		hook->Destroy();
	}
}