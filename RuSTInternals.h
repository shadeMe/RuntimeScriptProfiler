#pragma once

#include "obse/PluginAPI.h"
#include "obse/GameAPI.h"
#include "obse/GameForms.h"
#include "obse/GameTypes.h"
#include "obse/Script.h"
#include "common/IDirectoryIterator.h"

#include "[Libraries]\MemoryHandler\MemoryHandler.h"
#include "[Libraries]\INI Manager\INIManager.h"

#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

extern IDebugLog gLog;
extern std::string g_AppPath;

void PatchScriptRunnerPerformanceCounter(void);
void PatchTESExecuteScriptsMainLoop(void);

using namespace SME::MemoryHandler;

_DeclareNopHdlr(ScriptRunnerExecuteProlog, "query performance counter regardless of debug text state");
_DeclareNopHdlr(ScriptRunnerExecuteEpilog, "");
_DeclareMemHdlr(ScriptRunnerExecuteTimeElapsed, "record time elapsed for each script execution");
_DeclareMemHdlr(TESExecuteScriptsMainLoop, "record time taken to execute all scripts for a single frame");
_DeclareMemHdlr(ScriptRunnerExecuteCommand, "record time taked to execute each script command");

extern const UInt32			kScriptRunner_LookupCommandByOpcode;

// 28
class TESGame
{
public:
	TESGame();
	~TESGame();

	UInt8			quitGameFlag;				// 00
	UInt8			quitToMainMenuFlag;			// 01
	UInt8			unk02;						// 02
	UInt8			unk03;						// 03
	UInt8			unk04;						// 04
	UInt8			pad05[3];					// 05
	HWND			mainWindowHandle;			// 08
	HINSTANCE		processInstance;			// 0C
	DWORD			mainThreadID;				// 10
	HANDLE			mainThreadHandle;			// 14
	UInt32			unk18;						// 18
	UInt32			unk1C;						// 1C
	void*			directInputStruct;			// 20	size = 0x1BD8
	void*			directSoundStruct;			// 24	size = 0x328
};

extern TESGame**		g_TESGame;

class RUSTINIManager : public SME::INI::INIManager
{
public:
	void				Initialize();
};

extern SME::INI::INIManager*		g_INIManager;

bool IsRUDEPresent(void);