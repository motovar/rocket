#ifndef __PLATFORM_VFS_H__
#define __PLATFORM_VFS_H__

// Virtual file system used by every asset load in the port.
//
// - Paths are game-relative ("images/gemsheet6", "data\\diff.txt"); '\\' and '/' are interchangeable.
// - Lookups are case-insensitive, because the game's code and its files disagree on case
//   ("music\\1980-autonomus.s3m" vs "1980-Autonomus.s3m") and other systems' file names are case-sensitive.

#include <map>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdint>

namespace Platform
{

class VFS
{
public:
	static VFS& Get();

	// Base game data directory (the folder that contains images/, data/, sounds/, music/, properties/).
	void SetBaseDir(const std::string& theDir);
	const std::string& GetBaseDir() const { return mBaseDir; }

	// Resolves a game-relative path to a real host path in the base dir (the user dir for "userdata/").
	// Returns an empty string if the file does not exist.
	std::string Resolve(const std::string& theGamePath) const;

	// Resolves inside a single root only (used for "@2x" and alpha-mask lookups that must come from
	// the same folder as their colour image).
	std::string ResolveIn(const std::string& theRoot, const std::string& theGamePath) const;

	bool Exists(const std::string& theGamePath) const { return !Resolve(theGamePath).empty(); }
	FILE* Open(const std::string& theGamePath, const char* theMode) const;
	// Reads a whole file; a generated file (below) is read here too.
	bool ReadAll(const std::string& theGamePath, std::vector<uint8_t>& theData) const;

	// Generated files: contents a game makes at start-up (its code-drawn fonts' descriptors), read under a game path
	// as if the game folder held them. ReadAll, and so p_fopen, serves one ahead of the base dir;
	// Resolve, Open and Exists do not see it, since it has no host path. Empty contents remove the file. Set them
	// before loading starts: the loading thread reads them without a lock.
	void SetGeneratedFile(const std::string& theGamePath, const std::string& theContents);
	void ClearGeneratedFiles() { mGenerated.clear(); }

	// Normalizes separators to '/', strips "./" and leading slashes, collapses "a/../" (Platform::NormalizePath).
	static std::string Normalize(const std::string& thePath);

	// Case-insensitive lookup of a relative path under a real directory. Returns the real path or "".
	static std::string FindCaseInsensitive(const std::string& theRoot, const std::string& theRelPath);
	// Like FindCaseInsensitive, but from the cached directory listings alone (no stat call per path part): for the many probes
	// of an image's variants. A file made after its folder was listed is not seen until the cache is cleared.
	static std::string FindIndexed(const std::string& theRoot, const std::string& theRelPath);

	// Writable location for saves/settings ("userdata" in the original).
	void SetUserDir(const std::string& theDir) { mUserDir = theDir; }
	const std::string& GetUserDir() const { return mUserDir; }

private:
	std::string mBaseDir = ".";
	std::string mUserDir = "userdata";
	std::map<std::string, std::string> mGenerated;  // key: lower-case normalized game path
};

// Directory helpers.
bool IsDirectory(const std::string& thePath);
bool IsFile(const std::string& thePath);
std::vector<std::string> ListDirectory(const std::string& thePath);  // names only, no "." / ".."
bool MakeDirs(const std::string& thePath);

}

#endif
