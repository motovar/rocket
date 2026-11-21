#include "Placeholders.h"
#include "VFS.h"

#include <SDL.h>
#include <cstdio>
#include <cstring>

namespace Platform
{

static bool gPlaceholders = false;
static std::vector<std::string> gMissing;

// The whole list, so the file is current whenever the run ends
static void WriteMissingList()
{
	std::string aFileName = VFS::Get().GetUserDir() + "/missing-files.txt";
	FILE* aFile = fopen(aFileName.c_str(), "w");
	if (aFile == NULL)
		return;
	for (const std::string& aMissing : gMissing)
		fprintf(aFile, "%s\n", aMissing.c_str());
	fclose(aFile);
}

bool InitPlaceholders(int argc, char* argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-placeholders") == 0)
			SetPlaceholders(true);
	}
	return gPlaceholders;
}

void SetPlaceholders(bool on)
{
	if (on == gPlaceholders)
		return;
	gPlaceholders = on;
}

bool Placeholders()
{
	return gPlaceholders;
}

void NoteMissing(const std::string& theGamePath)
{
	std::string aPath = theGamePath;
	for (char& c : aPath)
	{
		if (c == '\\')
			c = '/';
	}
	for (const std::string& aKnown : gMissing)
	{
		if (aKnown == aPath)
			return;
	}
	gMissing.push_back(aPath);
	SDL_Log("placeholder: %s", aPath.c_str());

	WriteMissingList();
}

const std::vector<std::string>& GetMissing()
{
	return gMissing;
}

void ReportMissing()
{
	if (!gPlaceholders)
		return;
	SDL_Log("placeholders: %d game file%s missing", (int) gMissing.size(), (gMissing.size() == 1) ? "" : "s");
	WriteMissingList();
}

}
