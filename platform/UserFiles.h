#ifndef __PLATFORM_USERFILES_H__
#define __PLATFORM_USERFILES_H__

// Files in the user dir (VFS::GetUserDir()) that the port writes and removes outside the game's own save code. Every
// write goes through a temporary file and an atomic replace, so a crash or a second process never sees half a file.
//
// Windows paths are UTF-8 (SDL's argv and SDL_GetPrefPath) and these functions open them with the wide
// API. Elsewhere they use the C library directly.

#include <string>

namespace Platform
{

// Reads a whole file (a host path). Returns false if it does not exist or cannot be read.
bool ReadHostFile(const std::string& thePath, std::string* theData);

// Writes theData to <user dir>/<theRelPath>: <name>.tmp first, then replaces the file in one step. Returns false (and
// leaves the old file) on failure.
bool WriteUserFileAtomic(const std::string& theRelPath, const std::string& theData);

// Renames <user dir>/<theFrom> to <user dir>/<theTo>, replacing it.
bool RenameUserFile(const std::string& theFrom, const std::string& theTo);

// Removes <user dir>/<theRelPath> if it exists. Returns false if nothing was removed.
bool RemoveUserFile(const std::string& theRelPath);

}

#endif
