#include "RUSTInternals.h"

TESGame**					g_TESGame = (TESGame**)0x00B33398;
SME::INI::INIManager*		g_INIManager = new RUSTINIManager();

const UInt32				kScriptRunner_LookupCommandByOpcode = 0x004FCA30;

_DefineNopHdlr(ScriptRunnerExecuteProlog, 0x00517394, 2);
_DefineNopHdlr(ScriptRunnerExecuteEpilog, 0x005175B1, 2);
_DefineHookHdlr(ScriptRunnerExecuteTimeElapsed, 0x005175E2);
_DefineHookHdlr(TESExecuteScriptsMainLoop, 0x0040DBDC);

void PatchScriptRunnerPerformanceCounter(void)
{
	_MemoryHandler(ScriptRunnerExecuteProlog).WriteNop();
	_MemoryHandler(ScriptRunnerExecuteEpilog).WriteNop();
	_MemoryHandler(ScriptRunnerExecuteTimeElapsed).WriteJump();
}

void PatchTESExecuteScriptsMainLoop(void)
{
	_MemoryHandler(TESExecuteScriptsMainLoop).WriteJump();
}

void RUSTINIManager::Initialize()
{
	_MESSAGE("INI Path: %s", INIFile.c_str());
	std::fstream INIStream(INIFile.c_str(), std::fstream::in);
	bool CreateINI = false;

	if (INIStream.fail())
	{
		_MESSAGE("INI File not found; Creating one...");
		CreateINI = true;
	}

	INIStream.close();
	INIStream.clear();

	RegisterSetting(new SME::INI::INISetting(this, "SkipVanillaScripts", "Logging", "1", "Skips the calculation of elapsed time of vanilla script executions"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "FilterByModIndex", "Logging", "00", "Only scripts of the corresponding mod index are processed"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "FilterByFormID", "Logging", "00000000", "Only the script with the corresponding formID will be processed"), (CreateINI == false));

	if (CreateINI)		SaveSettingsToINI();
	else				ReadSettingsFromINI();
}

bool IsRUDEPresent()
{
	for (IDirectoryIterator Itr((std::string(g_AppPath + "Data\\OBSE\\Plugins\\")).c_str(), "RuntimeDebugger.dll"); !Itr.Done(); Itr.Next())
		return true;

	return false;
}

void __stdcall DoScriptRunnerExecuteTimeElapsedHook(Script* ExecutingScript, float ElapsedTime)
{
	static bool INIValueCached = false;
	static UInt32 FilterFormID = 0;
	static UInt16 FilterModID = 0;

	if (!INIValueCached)
	{
		sscanf_s(g_INIManager->GET_INI_STR("FilterByFormID"), "%08X", &FilterFormID);
		if (errno == ERANGE || errno == EINVAL)
			FilterFormID = 0;

		sscanf_s(g_INIManager->GET_INI_STR("FilterByModIndex"), "%02X", &FilterModID);
		if (errno == ERANGE || errno == EINVAL)
			FilterModID = 0;

		INIValueCached = true;
	}

	if (GetCurrentThreadId() != (*g_TESGame)->mainThreadID)
		return;

	if (g_INIManager->GET_INI_INT("SkipVanillaScripts") && (ExecutingScript->refID >> 24) == 0)
		return;
	else if (FilterModID && (ExecutingScript->refID >> 24) != FilterModID)
		return;
	else if (FilterFormID && ExecutingScript->refID != FilterFormID)
		return;

	_MESSAGE("Script '%08X' executed in %.10f milliseconds", ExecutingScript->refID, ElapsedTime);
}

_BeginHookHdlrFn(ScriptRunnerExecuteTimeElapsed)
{
	_DeclareHookHdlrFnVariable(ScriptRunnerExecuteTimeElapsed, Retn, 0x005175E9);
	__asm
	{
		fstp    dword ptr [esp + 0x2C]
		mov		eax, [esp + 0x2C]
		pushad
		push	eax
		push	edi
		call	DoScriptRunnerExecuteTimeElapsedHook
		popad
		fld     dword ptr [edi + 0x34]
		jmp		[_HookHdlrFnVariable(ScriptRunnerExecuteTimeElapsed, Retn)]
	}
}

void __stdcall DoTESExecuteScriptsMainLoopHook(bool State)
{
	static DWORD TickCount = 0;

	if (State)
	{
		_MESSAGE("\nBegin Frame:");
		gLog.Indent();
		TickCount = GetTickCount();
	}
	else
	{
		gLog.SetAutoFlush(true);
		gLog.Outdent();
		_MESSAGE("End Frame: Elapsed Time = %d milliseconds", GetTickCount() - TickCount);
		gLog.SetAutoFlush(false);
	}
}

_BeginHookHdlrFn(TESExecuteScriptsMainLoop)
{
	_DeclareHookHdlrFnVariable(TESExecuteScriptsMainLoop, Retn, 0x0040DBE1);
	_DeclareHookHdlrFnVariable(TESExecuteScriptsMainLoop, Call, 0x004402F0);
	__asm
	{
		pushad
		push	1
		call	DoTESExecuteScriptsMainLoopHook
		popad

		call	[_HookHdlrFnVariable(TESExecuteScriptsMainLoop, Call)]

		pushad
		push	0
		call	DoTESExecuteScriptsMainLoopHook
		popad

		jmp		[_HookHdlrFnVariable(TESExecuteScriptsMainLoop, Retn)]
	}
}