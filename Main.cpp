#include "RUSTInternals.h"

IDebugLog					gLog("RuntimeScriptProfiler.log");

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
std::string					g_AppPath, g_INIPath;

extern "C"
{
bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "RUST";
	info->version = 1;

	if(!obse->isEditor)
	{
		if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			_ERROR("OBSE version too old (got %08X expected at least %08X)", obse->obseVersion, OBSE_VERSION_INTEGER);
			return false;
		}

		if(obse->oblivionVersion != OBLIVION_VERSION)
		{
			_ERROR("incorrect Oblivion version (got %08X need %08X)", obse->oblivionVersion, OBLIVION_VERSION);
			return false;
		}
	}
	else
		return false;

	if (IsRUDEPresent())
	{
		_MESSAGE("RuntimeScriptProfiler cannot run alongside the RuntimeDebugger OBSE plugin!");
		return false;
	}

	return true;
}

bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	g_pluginHandle = obse->GetPluginHandle();
	_MESSAGE("RuntimeScriptProfiler Initializing...");
	gLog.Indent();

	gLog.SetAutoFlush(false);
	g_AppPath = obse->GetOblivionDirectory();
	g_INIPath = g_AppPath + "Data\\OBSE\\Plugins\\RuntimeScriptProfiler.ini";

	g_INIManager->SetINIPath(g_INIPath);
	g_INIManager->Initialize();

	PatchScriptRunnerPerformanceCounter();
	PatchTESExecuteScriptsMainLoop();

	gLog.Outdent();
	_MESSAGE("RuntimeScriptProfiler Initialized!\n\n");

	_MESSAGE("Script\t\t\t\t\t\t\t\t\t\tElapsed Time\n");
	return true;
}
};