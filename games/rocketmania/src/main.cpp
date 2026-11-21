// Entry point for the Rocket Mania Deluxe port.
//
// Arguments (all optional):
//   -gamedir=<dir>   folder containing the game's images/, data/, sounds/, music/, properties/ and RocketMania.exe (read for the
//                    game's texts, never run)
//   -userdir=<dir>   writable folder for the saved game, high scores and registry.ini
// Remaining "-key=value" arguments are passed to the app (SexyAppBase::HandleCmdLineParam), e.g. the
// framework's test hooks -seed=N, -script=<file>, -fast and -trace (see SexyAppBase.h).

#include "RocketApp.h"
#include "platform/SaveImport.h"
#include "platform/VFS.h"

#include <SDL.h>
#ifdef __ANDROID__
#include <SDL_system.h>
#endif
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace Sexy;

static std::string ArgValue(int argc, char** argv, const char* theKey)
{
	size_t n = strlen(theKey);
	for (int i = 1; i < argc; i++)
	{
		const char* a = argv[i];
		while (*a == '-')
			a++;
		if (!strncmp(a, theKey, n) && a[n] == '=')
			return a + n + 1;
	}
	return "";
}

// The game's texts are read from the original's RocketMania.exe (GameText.h), so it is part of the game folder
static bool LooksLikeGameDir(const std::string& theDir)
{
	return !Platform::VFS::FindCaseInsensitive(theDir, "rocketmania.exe").empty() &&
		!Platform::VFS::FindCaseInsensitive(theDir, "sounds/tubebump.ogg").empty() &&
		!Platform::VFS::FindCaseInsensitive(theDir, "images/daysky.jpg").empty();
}

static std::string ExeDir()
{
	char* base = SDL_GetBasePath();
	std::string dir = base ? base : "./";
	SDL_free(base);
	while (!dir.empty() && (dir.back() == '/' || dir.back() == '\\'))
		dir.pop_back();
	return dir;
}

static std::string FindGameDir()
{
	std::string exe = ExeDir();
	std::vector<std::string> candidates = {exe, exe + "/assets/rocketmania", exe + "/../assets/rocketmania",
		exe + "/../../assets/rocketmania", "assets/rocketmania"};
#ifdef __ANDROID__
	// Android: the game's files are copied here by the launcher (SetupActivity) or by hand (adb push, a file manager) into
	// the app's own folder, /sdcard/Android/data/<package>/files/rocketmania
	if (const char* ext = SDL_AndroidGetExternalStoragePath())
	{
		std::string e = ext;
		while (!e.empty() && e.back() == '/')
			e.pop_back();
		candidates.insert(candidates.begin(), e + "/rocketmania");
		candidates.insert(candidates.begin() + 1, e);
	}
	// where the launcher puts them when the device has no external files folder
	if (const char* in = SDL_AndroidGetInternalStoragePath())
		candidates.push_back(std::string(in) + "/rocketmania");
#endif
#ifdef _WIN32
	// The retail install (the installer's default folder)
	for (const char* env : {"ProgramFiles", "ProgramFiles(x86)"})
	{
		const char* pf = SDL_getenv(env);
		if (pf)
			candidates.push_back(std::string(pf) + "/PopCap Games/Rocket Mania Deluxe");
	}
#endif
	for (const std::string& c : candidates)
		if (LooksLikeGameDir(c))
			return c;
	return "";
}

int main(int argc, char** argv)
{
	std::string gameDir = ArgValue(argc, argv, "gamedir");
	std::string userDir = ArgValue(argc, argv, "userdir");

	if (gameDir.empty())
		gameDir = FindGameDir();
	if (userDir.empty())
	{
		char* pref = SDL_GetPrefPath("byo-pop", "Rocket Mania Deluxe");
		userDir = pref ? pref : "userdata";
		SDL_free(pref);
		while (!userDir.empty() && (userDir.back() == '/' || userDir.back() == '\\'))
			userDir.pop_back();
	}

	// -importsaves[=<install folder>]: brings the saves, high scores and settings of the original game installed on this computer into the
	// user dir (platform/SaveImport.h), then exits without starting the game. Without it nothing changes.
	{
		int anImportResult = 0;
		if (Platform::RunSaveImportIfRequested(argc, argv, "rocketmania", gameDir, userDir, &anImportResult))
			return anImportResult;
	}

	if (gameDir.empty() || !LooksLikeGameDir(gameDir))
	{
#ifdef __ANDROID__
		{
			std::string aMsg = "Could not find the Rocket Mania Deluxe game files.\n\nClose the game and start it again to choose "
				"the folder that holds your copy of the game.";
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Rocket Mania Deluxe", aMsg.c_str(), nullptr);
			SDL_Log("%s", aMsg.c_str());
			return 1;
		}
#endif
		std::string msg = "Could not find the Rocket Mania Deluxe game files.\n\n"
			"Run with -gamedir=\"<your Rocket Mania Deluxe folder>\" or copy the images, data, sounds, music "
			"and properties folders and the original RocketMania.exe (the game's texts are read from it) into an assets/rocketmania "
			"folder next to the executable.";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Rocket Mania Deluxe", msg.c_str(), nullptr);
		SDL_Log("%s", msg.c_str());
		return 1;
	}

	Platform::MakeDirs(userDir);
	Platform::VFS::Get().SetBaseDir(gameDir);
	Platform::VFS::Get().SetUserDir(userDir);
	SDL_Log("game: %s | user: %s", gameDir.c_str(), userDir.c_str());

	// The original keeps the app as a global object and calls Init, Start and Shutdown on it.
	static RocketApp* anApp = new RocketApp();
	anApp->SetCmdLine(argc, argv);
	anApp->Init();
	anApp->Start();
	anApp->Shutdown();
	delete anApp;
	return 0;
}
