#include <Windows.h>
#include <thread>
#include "Exploit/Roblox/TaskScheduler/TaskScheduler.hpp"
#include <lua.h>
#include <Luau/Compiler.h>
#include "Exploit/Execution/Execution.hpp"
#include <lualib.h>
#include "Exploit/Rendering/Rendering.hpp"

void MainThread()
{
	lua_State* L = reinterpret_cast<lua_State*>(Radium::TaskScheduler::GetLuaState());

	Radium::Execution::Init();

Radium::Rendering::Hook();

}

int __stdcall DllMain(HMODULE HModule, DWORD ReasonForCall, void*)
{
	if (ReasonForCall == DLL_PROCESS_ATTACH)
	{
		std::thread(MainThread).detach();
	}	

	return 1;
}