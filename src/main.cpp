#include "GarrysMod/Lua/Interface.h"
#include "vmthook.h"
#include "util.h"
#include "windows.h"

#define CREATELUAINTERFACE 4
#define CLOSELUAINTERFACE 5
#define RUNSTRINGEX 111

typedef unsigned char uchar;

VMTHook* luaSharedVMTHook;
VMTHook* luaClientVMTHook;

using namespace GarrysMod;

Lua::ILuaBase* MENU;
lua_State* clientState;
void* rexthisptr;

typedef void* (__thiscall *RunStringExHookFn)(void* thisptr, const char* fileName, const char* path, const char* stringToRun, bool run, bool showErrors, bool, bool);
void* __fastcall RunStringExHook(void* thisptr, int edx, const char* fileName, const char* path, const char* stringToRun, bool run, bool showErrors, bool a, bool b)
{
	if (rexthisptr == nullptr)
		rexthisptr = thisptr;

	MENU->PushSpecial(Lua::SPECIAL_GLOB);
	MENU->GetField(-1, "hook");
		MENU->GetField(-1, "Call");
			MENU->PushString("RunOnClient");
			MENU->PushNil();
			MENU->PushString(fileName);
			MENU->PushString(stringToRun);
		MENU->Call(4, 1);
		if (!MENU->IsType(-1, Lua::Type::NIL))
			stringToRun = MENU->CheckString();
	MENU->Pop(3);

	return luaClientVMTHook->GetOriginalFunction<RunStringExHookFn>(RUNSTRINGEX)(thisptr, fileName, path, stringToRun, run, showErrors, a, b);
}

typedef lua_State* (__thiscall *CreateLuaInterfaceHookFn)(void* thisptr, unsigned char stateType, bool renew);
void* __fastcall CreateLuaInterfaceHook(void* thisptr, int edx, unsigned char stateType, bool renew)
{
	lua_State* state = luaSharedVMTHook->GetOriginalFunction<CreateLuaInterfaceHookFn>(CREATELUAINTERFACE)(thisptr, stateType, renew);

	MENU->PushSpecial(Lua::SPECIAL_GLOB);
	MENU->GetField(-1, "hook");
		MENU->GetField(-1, "Call");
			MENU->PushString("ClientStateCreated");
			MENU->PushNil();
		MENU->Call(2, 0);
	MENU->Pop(2);

	if (stateType != 0)
		return state;

	clientState = state;

	luaClientVMTHook = new VMTHook(state);
	luaClientVMTHook->HookFunction(&RunStringExHook, RUNSTRINGEX);

	return clientState;
}

typedef void* (__thiscall *CloseLuaInterfaceHookFn)(void* thisptr, void* state);
void* __fastcall CloseLuaInterfaceHook(void* thisptr, int edx, lua_State* state)
{
	if (state == clientState) {
		clientState = NULL;
		rexthisptr = nullptr;
	}

	return luaSharedVMTHook->GetOriginalFunction<CloseLuaInterfaceHookFn>(CLOSELUAINTERFACE)(thisptr, state);
}

int RunOnClient(lua_State* state)
{
	LUA->CheckType(1, GarrysMod::Lua::Type::STRING);

	if (!clientState || rexthisptr == nullptr)
		LUA->ThrowError("Not in game");

	luaClientVMTHook->GetOriginalFunction<RunStringExHookFn>(RUNSTRINGEX)(rexthisptr, "", "", LUA->CheckString(1), true, true, false, false);

	return 0;
}

GMOD_MODULE_OPEN()
{
	MENU = LUA;

	auto luaShared = util::GetInterfaceSingle<void* >("lua_shared.dll", "LUASHARED003");
	
	if (!luaShared)
		MessageBoxA(NULL, "gay", "gay", NULL);

	luaSharedVMTHook = new VMTHook(luaShared);
	luaSharedVMTHook->HookFunction(&CreateLuaInterfaceHook, CREATELUAINTERFACE);
	luaSharedVMTHook->HookFunction(&CloseLuaInterfaceHook, CLOSELUAINTERFACE);

	LUA->PushSpecial(Lua::SPECIAL_GLOB);
	LUA->PushString("RunOnClient");
	LUA->PushCFunction(RunOnClient);
	LUA->SetTable(-3);
	LUA->Pop();

	return 0;
}