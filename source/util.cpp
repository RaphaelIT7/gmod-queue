#include "util.h"
#include <string>
#include "edict.h"

GarrysMod::Lua::ILuaInterface* g_Lua;
IVEngineServer* engine;

void Start_Table() {
	g_Lua->PushSpecial(SPECIAL_GLOB);
	g_Lua->CreateTable();
}

void Add_Func(CFunc Func, const char* Name) {
	g_Lua->PushCFunction(Func);
	g_Lua->SetField(-2, Name);
}

void Add_Func(CFunc Func, std::string Name) {
	g_Lua->PushCFunction(Func);
	g_Lua->SetField(-2, Name.c_str());
}

void Finish_Table(const char* Name) {
	g_Lua->SetField(-2, Name);
	g_Lua->Pop();
}

void Finish_Table(std::string Name) {
	g_Lua->SetField(-2, Name.c_str());
	g_Lua->Pop();
}

// should never be used outside of main thread!!! what happends: memory access violation
void LuaPrint(const char* Text) {
	g_Lua->PushSpecial(SPECIAL_GLOB);
	g_Lua->GetField(-1, "print");
	g_Lua->PushString(Text);
	g_Lua->Call(1, 0);
}

// should never be used outside of main thread!!! what happends: memory access violation
void LuaPrint(std::string Text) {
	g_Lua->PushSpecial(SPECIAL_GLOB);
	g_Lua->GetField(-1, "print");
	g_Lua->PushString(Text.c_str());
	g_Lua->Call(1, 0);
}