#include "util.h"
#include "lua.h"
#include "detours.h"
#include "symbols.h"
#include "sourcesdk/baseserver.h"
#include "sv_client.h"
#include "sourcesdk/net_chan.h"
#include "queue.h"

int CBaseClient_TypeID = -1;
PushReferenced_LuaClass(CBaseClient, CBaseClient_TypeID)
Get_LuaClass(CBaseClient, CBaseClient_TypeID, "CBaseClient")

Default__index(CBaseClient);
Default__newindex(CBaseClient);

LUA_FUNCTION_STATIC(CBaseClient_GetTable)
{
	LuaUserData* data = Get_CBaseClient_Data(1, true);
	CBaseClient* pClient = (CBaseClient*)data->pData;
	if (data->pAdditionalData != pClient->GetUserID())
	{
		data->pAdditionalData = pClient->GetUserID();
		LUA->ReferenceFree(data->iTableReference);
		LUA->CreateTable();
		data->iTableReference = LUA->ReferenceCreate();
	}

	LUA->ReferencePush(data->iTableReference); // This should never crash so no safety checks.

	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_GetPlayerSlot)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushNumber(pClient->GetPlayerSlot());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_GetUserID)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushNumber(pClient->GetUserID());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_GetName)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushString(pClient->GetClientName());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_GetSteamID)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushString(pClient->GetNetworkIDString());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_Reconnect)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	pClient->Reconnect();
	return 0;
}

LUA_FUNCTION_STATIC(CBaseClient_ClientPrint)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	pClient->ClientPrintf(LUA->CheckString(2));
	return 0;
}

LUA_FUNCTION_STATIC(CBaseClient_IsValid)
{
	CBaseClient* pClient = Get_CBaseClient(1, false);
	
	LUA->PushBool(pClient != NULL && pClient->IsConnected());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_Disconnect)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);
	const char* strReason = LUA->CheckString(2);

	pClient->Disconnect(strReason);
	return 0;
}

LUA_FUNCTION_STATIC(CBaseClient_IsConnected)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushBool(pClient->IsConnected());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_IsSpawned)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushBool(pClient->IsSpawned());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_IsActive)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushBool(pClient->IsActive());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_GetSignonState)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushNumber(pClient->m_nSignonState);
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_IsFakeClient)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);

	LUA->PushBool(pClient->IsFakeClient());
	return 1;
}

LUA_FUNCTION_STATIC(CBaseClient_SetSignonState)
{
	CBaseClient* pClient = Get_CBaseClient(1, true);
	int iSignOnState = (int)LUA->CheckNumber(2);
	int iSpawnCount = (int)LUA->GetNumber(3);
	bool bRawSet = LUA->GetBool(4);

	if (!pClient)
	{
		LUA->PushBool(false);
		return 1;
	}

	if (bRawSet)
	{
		pClient->m_nSignonState = iSignOnState;
		LUA->PushBool(true);
		return 1;
	}

	LUA->PushBool(pClient->SetSignonState(iSignOnState, iSpawnCount));
	return 1;
}

void Push_CBaseClientMeta()
{
	Util::AddFunc(CBaseClient__newindex, "__newindex");
	Util::AddFunc(CBaseClient__index, "__index");
	Util::AddFunc(CBaseClient_GetTable, "GetTable");
	Util::AddFunc(CBaseClient_GetPlayerSlot, "GetPlayerSlot");
	Util::AddFunc(CBaseClient_GetUserID, "GetUserID");
	Util::AddFunc(CBaseClient_GetName, "GetName");
	Util::AddFunc(CBaseClient_GetSteamID, "GetSteamID");
	Util::AddFunc(CBaseClient_Reconnect, "Reconnect");
	Util::AddFunc(CBaseClient_ClientPrint, "ClientPrint");
	Util::AddFunc(CBaseClient_IsValid, "IsValid");
	Util::AddFunc(CBaseClient_Disconnect, "Disconnect");
	Util::AddFunc(CBaseClient_IsConnected, "IsConnected");
	Util::AddFunc(CBaseClient_IsSpawned, "IsSpawned");
	Util::AddFunc(CBaseClient_IsActive, "IsActive");
	Util::AddFunc(CBaseClient_GetSignonState, "GetSignonState");
	Util::AddFunc(CBaseClient_IsFakeClient, "IsFakeClient");
	Util::AddFunc(CBaseClient_SetSignonState, "SetSignonState");
}

LUA_FUNCTION_STATIC(CGameClient__tostring)
{
	CBaseClient* pClient = Get_CBaseClient(1, false);
	if (!pClient || !pClient->IsConnected())
	{
		LUA->PushString("GameClient [NULL]");
	} else {
		char szBuf[128] = {};
		V_snprintf(szBuf, sizeof(szBuf),"GameClient [%i][%s]", pClient->GetPlayerSlot(), pClient->GetClientName());
		LUA->PushString(szBuf);
	}

	return 1;
}

LUA_FUNCTION_STATIC(gameserver_GetClient)
{
	if (!Util::server || !Util::server->IsActive())
		return 0;

	int iClientIndex = (int)LUA->CheckNumber(1);
	if (iClientIndex >= Util::server->GetClientCount())
		return 0;

	CBaseClient* pClient = (CBaseClient*)((IServer*)Util::server)->GetClient(iClientIndex);
	if (pClient && !pClient->IsConnected())
		pClient = NULL;

	Push_CBaseClient(pClient);
	return 1;
}

LUA_FUNCTION_STATIC(gameserver_GetAll)
{
	LUA->CreateTable();
		if (!Util::server || !Util::server->IsActive())
			return 1;

		int idx = 0;
		for (int iClientIndex=0; iClientIndex<Util::server->GetClientCount(); ++iClientIndex)
		{
			CBaseClient* pClient = (CBaseClient*)Util::server->GetClient(iClientIndex);
			if (!pClient->IsConnected())
				continue;

			LUA->PushNumber(++idx);
			Push_CBaseClient(pClient);
			LUA->RawSet(-3);
		}

	return 1;
}

static Detouring::Hook detour_CServerGameClients_GetPlayerLimit;
static void hook_CServerGameClients_GetPlayerLimit(void* funkyClass, int& minPlayers, int& maxPlayers, int& defaultMaxPlayers)
{
	minPlayers = 1;
	maxPlayers = 255; // Allows one to go up to 255 slots.
	defaultMaxPlayers = 255;
}

static Detouring::Hook detour_CBaseServer_FillServerInfo;
static void hook_CBaseServer_FillServerInfo(void* srv, SVC_ServerInfo& info)
{
	detour_CBaseServer_FillServerInfo.GetTrampoline<Symbols::CBaseServer_FillServerInfo>()(srv, info);

	// Fixes a crash("UpdatePlayerName with bogus slot 129") when joining a server which has more than 128 slots / is over MAX_PLAYERS
	if ( info.m_nMaxClients > 128 )
		info.m_nMaxClients = 128;
}

static Detouring::Hook detour_CBaseClient_SetSignonState;
static bool hook_CBaseClient_SetSignonState(CBaseClient* cl, int state, int spawncount)
{
	if (Lua::PushHook("PlayerQueue:OnSetSignonState"))
	{
		Push_CBaseClient(cl);
		g_Lua->PushNumber(state);
		g_Lua->PushNumber(spawncount);
		if (g_Lua->CallFunctionProtected(4, 1, true))
		{
			bool ret = g_Lua->GetBool(-1);
			g_Lua->Pop(1);

			if (ret)
				return false;
		}
	}

	return detour_CBaseClient_SetSignonState.GetTrampoline<Symbols::CBaseClient_SetSignonState>()(cl, state, spawncount);
}

class CExtentedNetMessage : public CNetMessage
{
public:
	INetMessageHandler *m_pMessageHandler;
};

/*
 * Moving a entire CGameClient into another CGameClient to hopefully not make the engine too angry.
 * This is required to preserve the logic of m_nEntityIndex = m_nClientSlot + 1
 * We don't copy everything, like the baseline and such.
 * 
 * Current State: The Client's LocalPlayer is a NULL Entity.....
 */
static void MoveCGameClientIntoCGameClient(CGameClient* origin, CGameClient* target)
{
	DevMsg(PROJECT_NAME ": Reassigned client to from slot %i to %i\n", origin->m_nClientSlot, target->m_nClientSlot);

	target->Inactivate();
	target->Clear();

	/*
	 * NOTE: This will fire the player_connect and player_connect_client gameevents.
	 * BUG: Their Name will have (1) at the beginning because of some funny engine behavior.
	 */
	target->Connect( origin->m_Name, origin->m_UserID, origin->m_NetChannel, origin->m_bFakePlayer, origin->m_clientChallenge );

	/*
	 * Basic CBaseClient::Connect setup
	 */

	//target->m_ConVars = origin->m_ConVars;
	//target->m_bInitialConVarsSet = origin->m_bInitialConVarsSet;
	//target->m_UserID = origin->m_UserID;
	//target->m_bFakePlayer = origin->m_bFakePlayer;
	//target->m_NetChannel = origin->m_NetChannel;
	//target->m_clientChallenge = origin->m_clientChallenge;
	//target->edict = Util::engineserver->PEntityOfEntIndex( target->m_nEntityIndex );
	//target->m_PackInfo.m_pClientEnt = target->edict;
	//target->m_PackInfo.m_nPVSSize = sizeof( target->m_PackInfo.m_PVS );

	target->SetName( origin->m_Name ); // Required thingy
	target->m_nSignonState = origin->m_nSignonState;

	/*
	 * Copy over other things
	 */

	//for (int i = 0; i < MAX_CUSTOM_FILES; ++i)
	//	target->m_nCustomFiles[i] = origin->m_nCustomFiles[i];

	target->m_SteamID = origin->m_SteamID;
	target->m_nFriendsID = origin->m_nFriendsID;
	target->m_nFilesDownloaded = origin->m_nFilesDownloaded;
	target->m_nSignonTick = origin->m_nSignonTick;
	target->m_nStringTableAckTick = origin->m_nStringTableAckTick;
	target->m_nDeltaTick = origin->m_nDeltaTick;
	target->m_nSendtableCRC = origin->m_nSendtableCRC;
	target->m_fNextMessageTime = origin->m_fNextMessageTime;
	target->m_fSnapshotInterval = origin->m_fSnapshotInterval;
	target->m_nForceWaitForTick = origin->m_nForceWaitForTick;
	target->m_bReportFakeClient = origin->m_bReportFakeClient;
	target->m_bReceivedPacket = origin->m_bReceivedPacket;
	target->m_bFullyAuthenticated = origin->m_bFullyAuthenticated;

	memcpy(target->m_FriendsName, origin->m_FriendsName, sizeof(origin->m_FriendsName));
	memcpy(target->m_GUID, origin->m_GUID, sizeof(origin->m_GUID));

	target->m_fTimeLastNameChange = origin->m_fTimeLastNameChange;
	target->m_bPlayerNameLocked = origin->m_bPlayerNameLocked;
	memcpy(target->m_szPendingNameChange, origin->m_szPendingNameChange, sizeof(origin->m_szPendingNameChange));

	/*
	 * Update CNetChan and CNetMessage's properly to not crash.
	 */

	CNetChan* chan = (CNetChan*)target->m_NetChannel;
	chan->m_MessageHandler = target;

	FOR_EACH_VEC(chan->m_NetMessages, i)
	{
		CExtentedNetMessage* msg = (CExtentedNetMessage*)chan->m_NetMessages[i];
		msg->m_pMessageHandler = target;

		if (msg->GetType() == clc_CmdKeyValues)
		{
			Base_CmdKeyValues* keyVal = (Base_CmdKeyValues*)msg;
			if (keyVal->m_pKeyValues)
			{
				keyVal->m_pKeyValues = NULL; // Will leak memory but we can't safely delete it currently.
				// ToDo: Fix this small memory leak.
			}
		}
	}

	/*
	 * Nuke the origin client
	 */

	origin->m_NetChannel = NULL; // Nuke the net channel or else it might touch it.
	//origin->m_ConVars = NULL; // Same here
	origin->Inactivate();
	origin->Clear();

	if (Lua::PushHook("PlayerQueue:OnPlayerChangedSlot"))
	{
		g_Lua->PushNumber(origin->m_nClientSlot);
		g_Lua->PushNumber(target->m_nClientSlot);
		g_Lua->CallFunctionProtected(3, 0, true);
	}

	/*
	 * Update Client about it's playerSlot or else it will see the wrong entity as it's local player.
	 */

	SVC_ServerInfo info;
	CBaseServer* pServer = (CBaseServer*)target->GetServer();
	pServer->FillServerInfo(info);

	info.m_nPlayerSlot = target->m_nClientSlot;

	target->SendNetMsg(info, true);

	/*
	 * Reconnecting the client to let it go through the loading process again since it became unstable when we sent the ServerInfo.
	 */

	target->Reconnect();
}

#define MAX_PLAYERS 128
static int FindFreeClientSlot()
{
	int nextFreeEntity = 255;
	int count = Util::server->GetClientCount();
	if (count > MAX_PLAYERS)
		count = MAX_PLAYERS;

	for (int iClientIndex=0; iClientIndex<count; ++iClientIndex)
	{
		CBaseClient* pClient = (CBaseClient*)Util::server->GetClient(iClientIndex);

		if (pClient->m_nSignonState != SIGNONSTATE_NONE)
			continue;

		if (pClient->m_nEntityIndex < nextFreeEntity)
			nextFreeEntity = pClient->m_nEntityIndex;
	}

	return nextFreeEntity;
}

static Detouring::Hook detour_CGameClient_SpawnPlayer;
void hook_CGameClient_SpawnPlayer(CGameClient* client)
{
	if (client->m_nClientSlot < MAX_PLAYERS)
	{
		detour_CGameClient_SpawnPlayer.GetTrampoline<Symbols::CGameClient_SpawnPlayer>()(client);
		return;
	}

	int nextFreeEntity = FindFreeClientSlot();
	if (nextFreeEntity > MAX_PLAYERS)
	{
		Warning(PROJECT_NAME ": Failed to find a valid player slot to use! Stopping client spawn! (%i, %i, %i)\n", client->m_nClientSlot, client->GetUserID(), nextFreeEntity);
		return;
	}

	CGameClient* pClient = (CGameClient*)Util::GetClientByIndex(nextFreeEntity - 1);
	if (pClient->m_nSignonState != SIGNONSTATE_NONE)
	{
		// It really didn't like what we had planned.
		Warning(PROJECT_NAME ": Client collision! fk. Client will be refused to spawn! (%i - %s, %i - %s)\n", pClient->m_nClientSlot, pClient->GetClientName(), client->m_nClientSlot, client->GetClientName());
		return;
	}

	MoveCGameClientIntoCGameClient(client, pClient);
	//detour_CGameClient_SpawnPlayer.GetTrampoline<Symbols::CGameClient_SpawnPlayer>()(pClient);
}

void GameServer_OnClientDisconnect(CBaseClient* pClient)
{
	if (pClient->GetServer() != Util::server)
		return;

	if (g_Lua && Lua::PushHook("PlayerQueue:OnClientDisconnect"))
	{
		Push_CBaseClient(pClient);
		g_Lua->CallFunctionProtected(2, 0, true);
	}

	auto it = g_pPushedCBaseClient.find(pClient);
	if (it != g_pPushedCBaseClient.end())
	{
		Delete_CBaseClient(it->first);
	}
}

static Detouring::Hook detour_CSteam3Server_NotifyClientDisconnect;
static void hook_CSteam3Server_NotifyClientDisconnect(void* pServer, CBaseClient* pClient)
{
	GameServer_OnClientDisconnect(pClient);
	detour_CSteam3Server_NotifyClientDisconnect.GetTrampoline<Symbols::CSteam3Server_NotifyClientDisconnect>()(pServer, pClient);
}

void Queue::Init()
{
	CBaseClient_TypeID = g_Lua->CreateMetaTable("CGameClient");
		Push_CBaseClientMeta();

		Util::AddFunc(CGameClient__tostring, "__tostring");
	g_Lua->Pop(1);

	Util::StartTable();
		Util::AddFunc(gameserver_GetClient, "GetClient");
		Util::AddFunc(gameserver_GetAll, "GetAll");
	Util::FinishTable("playerqueue");

	SourceSDK::FactoryLoader engine_loader("engine");
	Detour::Create(
		&detour_CBaseServer_FillServerInfo, "CBaseServer::FillServerInfo",
		engine_loader.GetModule(), Symbols::CBaseServer_FillServerInfoSym,
		(void*)hook_CBaseServer_FillServerInfo
	);

	Detour::Create(
		&detour_CBaseClient_SetSignonState, "CBaseClient::SetSignonState",
		engine_loader.GetModule(), Symbols::CBaseClient_SetSignonStateSym,
		(void*)hook_CBaseClient_SetSignonState
	);

	Detour::Create(
		&detour_CGameClient_SpawnPlayer, "CGameClient::SpawnPlayer",
		engine_loader.GetModule(), Symbols::CGameClient_SpawnPlayerSym,
		(void*)hook_CGameClient_SpawnPlayer
	);

	Detour::Create(
		&detour_CSteam3Server_NotifyClientDisconnect, "CSteam3Server::NotifyClientDisconnect",
		engine_loader.GetModule(), Symbols::CSteam3Server_NotifyClientDisconnectSym,
		(void*)hook_CSteam3Server_NotifyClientDisconnect
	);

	SourceSDK::FactoryLoader server_loader("server");
	Detour::Create(
		&detour_CServerGameClients_GetPlayerLimit, "CServerGameClients::GetPlayerLimit",
		server_loader.GetModule(), Symbols::CServerGameClients_GetPlayerLimitSym,
		(void*)hook_CServerGameClients_GetPlayerLimit
	);
}