#pragma once

#include <LuaInterface.h>
#include "Platform.hpp"
#include "vprof.h"
#include <unordered_map>
#include <algorithm>

#define VPROF_BUDGETGROUP_PLAYERQUEUE _T("PlayerQueue")

extern GarrysMod::Lua::ILuaInterface* g_Lua;

class CBaseClient;
class IServer;
namespace Util
{
	#define LUA_REGISTRYINDEX	(-10000)
	#define LUA_ENVIRONINDEX	(-10001)
	#define LUA_GLOBALSINDEX	(-10002)

	inline void StartTable() {
		g_Lua->CreateTable();
	}

	inline void AddFunc(GarrysMod::Lua::CFunc Func, const char* Name) {
		g_Lua->PushString(Name);
		g_Lua->PushCFunction(Func);
		g_Lua->RawSet(-3);
	}

	inline void AddValue(double value, const char* Name) {
		g_Lua->PushString(Name);
		g_Lua->PushNumber(value);
		g_Lua->RawSet(-3);
	}

	inline void FinishTable(const char* Name) {
		g_Lua->SetField(LUA_GLOBALSINDEX, Name);
	}

	inline void NukeTable(const char* pName)
	{
		g_Lua->PushNil();
		g_Lua->SetField(LUA_GLOBALSINDEX, pName);
	}

	inline bool PushTable(const char* pName)
	{
		g_Lua->GetField(LUA_GLOBALSINDEX, pName);
		if (g_Lua->IsType(-1, GarrysMod::Lua::Type::Table))
			return true;

		g_Lua->Pop(1);
		return false;
	}

	inline void PopTable()
	{
		g_Lua->Pop(1);
	}

	inline void RemoveField(const char* pName)
	{
		g_Lua->PushNil();
		g_Lua->SetField(-2, pName);
	}

	inline bool HasField(const char* pName, int iType)
	{
		g_Lua->GetField(-1, pName);
		return g_Lua->IsType(-1, iType);
	}

	extern void AddDetour(); // We load Gmod's functions in there.

	extern CBaseClient* GetClientByUserID(int userID);
	extern CBaseClient* GetClientByIndex(int index);
	extern std::vector<CBaseClient*> GetClients();

	extern IServer* server;
}

/*
	ToDo: Implement a proper class like gmod has with CLuaCLass/CLuaLibrary & use thoes instead for everything.
*/

struct LuaUserData {
	~LuaUserData() {
		if (!ThreadInMainThread())
		{
			Warning("holylib: Tried to delete usetdata from another thread!\n");
			return;
		}

		if (iReference != -1)
		{
			if (g_Lua)
			{
				g_Lua->ReferencePush(iReference);
				g_Lua->SetUserType(-1, NULL);
				g_Lua->Pop(1);
				g_Lua->ReferenceFree(iReference);
			}
			iReference = -1;
		}

		if (iTableReference != -1)
		{
			if (g_Lua)
				g_Lua->ReferenceFree(iTableReference);

			iTableReference = -1;
		}

		pAdditionalData = NULL;
	}

	inline void Init(GarrysMod::Lua::ILuaInterface* LUA)
	{
		if (iTableReference == -1)
		{
			LUA->CreateTable();
			iTableReference = LUA->ReferenceCreate();
		}
	}

	inline void CreateReference()
	{
		if (iReference != -1)
			Warning("holylib: something went wrong when pushing userdata! (Reference leak!)\n");

		g_Lua->Push(-1); // When Init is called this object was already pushed onto the stack and sits at -1!
		iReference = g_Lua->ReferenceCreate();
	}

	inline void* GetData()
	{
		return pData;
	}

	inline bool Push()
	{
		if (iReference == -1)
			return false;

		g_Lua->ReferencePush(iReference);
		return true;
	}

	void* pData = NULL;
	int iReference = -1;
	int iTableReference = -1;
	int pAdditionalData = NULL; // Used by HLTVClient.
};

// BUG: This LuaClass function and all others were made in mind to support a single Lua Instance. Now we got multiple.
#define MakeString( str1, str2, str3 ) ((std::string)str1).append(str2).append(str3)
#define Get_LuaClass( className, luaType, strName ) \
static std::string invalidType_##className = MakeString("Tried to use something that wasn't a ", strName, "!"); \
static std::string triedNull_##className = MakeString("Tried to use a NULL ", strName, "!"); \
LuaUserData* Get_##className##_Data(int iStackPos, bool bError) \
{ \
	if (!g_Lua->IsType(iStackPos, luaType)) \
	{ \
		if (bError) \
			g_Lua->ThrowError(invalidType_##className.c_str()); \
\
		return NULL; \
	} \
\
	LuaUserData* pVar = g_Lua->GetUserType<LuaUserData>(iStackPos, luaType); \
	if ((!pVar || !pVar->GetData()) && bError) \
		g_Lua->ThrowError(triedNull_##className.c_str()); \
\
	return pVar; \
} \
\
className* Get_##className(int iStackPos, bool bError) \
{ \
	LuaUserData* pLuaData = Get_##className##_Data(iStackPos, bError); \
	if (!pLuaData) \
		return NULL; \
 \
	return (className*)pLuaData->GetData(); \
}

#define Push_LuaClass( className, luaType ) \
void Push_##className(className* var) \
{ \
	if (!var) \
	{ \
		g_Lua->PushNil(); \
		return; \
	} \
\
	LuaUserData* userData = new LuaUserData; \
	userData->pData = var; \
	userData->Init(g_Lua); \
	g_Lua->PushUserType(userData, luaType); \
}

// This one is special, the GC WONT free the LuaClass meaning this "could" (and did in the past) cause a memory/reference leak
#define PushReferenced_LuaClass( className, luaType ) \
static std::unordered_map<className*, LuaUserData*> g_pPushed##className; \
void Push_##className(className* var) \
{ \
	if (!var) \
	{ \
		g_Lua->PushNil(); \
		return; \
	} \
\
	auto it = g_pPushed##className.find(var); \
	if (it != g_pPushed##className.end()) \
	{ \
		g_Lua->ReferencePush(it->second->iReference); \
	} else { \
		LuaUserData* userData = new LuaUserData; \
		userData->pData = var; \
		g_Lua->PushUserType(userData, luaType); \
		userData->Init(g_Lua); \
		userData->CreateReference(); \
		g_pPushed##className[var] = userData; \
	} \
} \
\
static void Delete_##className(className* var) \
{ \
	auto it = g_pPushed##className.find(var); \
	if (it != g_pPushed##className.end()) \
	{ \
		delete it->second; \
		g_pPushed##className.erase(it); \
	} \
}

#define Vector_RemoveElement(vec, element) \
{ \
    auto _it = std::find((vec).begin(), (vec).end(), (element)); \
    if (_it != (vec).end()) \
        (vec).erase(_it); \
}

#define Default__index(className) \
LUA_FUNCTION_STATIC(className ## __index) \
{ \
	if (LUA->FindOnObjectsMetaTable(1, 2)) \
		return 1; \
\
	LUA->Pop(1); \
	LUA->ReferencePush(Get_##className##_Data(1, true)->iTableReference); \
	if (!LUA->FindObjectOnTable(-1, 2)) \
		LUA->PushNil(); \
\
	LUA->Remove(-2); \
\
	return 1; \
}

#define Default__gc(className, func) \
LUA_FUNCTION_STATIC(className ## __gc) \
{ \
	LuaUserData* pData = Get_##className##_Data(1, false); \
	if (pData) \
	{ \
		func \
		delete pData; \
		LUA->SetUserType(1, NULL); \
	} \
 \
	return 0; \
} \

#define Default__newindex(className) \
LUA_FUNCTION_STATIC(className ## __newindex) \
{ \
	LUA->ReferencePush(Get_##className##_Data(1, true)->iTableReference); \
	LUA->Push(2); \
	LUA->Push(3); \
	LUA->RawSet(-3); \
	LUA->Pop(1); \
\
	return 0; \
}

#define Default__GetTable(className) \
LUA_FUNCTION_STATIC(className ## _GetTable) \
{ \
	LUA->ReferencePush(Get_##className##_Data(1, true)->iTableReference); \
	return 1; \
}