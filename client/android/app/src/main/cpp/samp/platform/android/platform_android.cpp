/**
 * Android implementation of the platform abstraction.
 *
 * This is the only file in the portable half of the tree that is allowed to
 * include an Android header.
 */

#include "platform/api.h"

#include <android/log.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include <cerrno>
#include <cstring>
#include <string>

namespace Platform
{
	// ---------------------------------------------------------------------
	// Module lookup
	// ---------------------------------------------------------------------

	uintptr_t ModuleBase(const char* name)
	{
		void* handle = dlopen(name, RTLD_LAZY);
		if (handle == nullptr)
		{
			return 0;
		}

		// dlopen hands back a handle, not an address. Resolving any symbol the
		// image is known to export and asking dladdr which file it came from is
		// what actually yields the load address.
		uintptr_t base = 0;
		void* symbol = dlsym(handle, "JNI_OnLoad");
		if (symbol != nullptr)
		{
			Dl_info info;
			if (dladdr(symbol, &info) != 0)
			{
				base = reinterpret_cast<uintptr_t>(info.dli_fbase);
			}
		}

		dlclose(handle);
		return base;
	}

	uintptr_t GameBase()
	{
		// Cached: this runs from hook installation paths that are called often
		// enough that repeating a dlopen/dlclose cycle would be wasteful.
		static uintptr_t base = 0;
		if (base == 0)
		{
			base = ModuleBase("libGame.so");
		}
		return base;
	}

	// ---------------------------------------------------------------------
	// Filesystem
	// ---------------------------------------------------------------------

	const char* DataPath()
	{
		// Shared external storage, so a player can copy their own GTA:SA files
		// in without root. The app asks for MANAGE_EXTERNAL_STORAGE to reach it.
		return "/storage/emulated/0/GTA/";
	}

	const char* ConfigPath()
	{
		return "/storage/emulated/0/GTA/SAMP/";
	}

	bool EnsureDirectory(const char* path)
	{
		if (path == nullptr || path[0] == '\0')
		{
			return false;
		}

		std::string work(path);
		// A trailing slash would make the final mkdir a no-op on an empty name.
		if (!work.empty() && work.back() == '/')
		{
			work.pop_back();
		}

		for (std::size_t i = 1; i < work.size(); ++i)
		{
			if (work[i] != '/')
			{
				continue;
			}
			work[i] = '\0';
			mkdir(work.c_str(), 0777);
			work[i] = '/';
		}

		// EEXIST is the normal outcome on every launch after the first.
		return mkdir(work.c_str(), 0777) == 0 || errno == EEXIST;
	}

	// ---------------------------------------------------------------------
	// Logging
	// ---------------------------------------------------------------------

	void LogLine(LogLevel level, const char* tag, const char* message)
	{
		int priority = ANDROID_LOG_INFO;
		switch (level)
		{
			case LogLevel::Warning: priority = ANDROID_LOG_WARN;  break;
			case LogLevel::Error:   priority = ANDROID_LOG_ERROR; break;
			case LogLevel::Info:    priority = ANDROID_LOG_INFO;  break;
		}

		__android_log_write(priority, tag, message);
	}
}
