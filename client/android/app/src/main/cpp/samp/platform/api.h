#pragma once

#include <cstdint>

/**
 * Platform abstraction.
 *
 * About 60% of this client is platform-neutral C++ — RakNet, ImGui, the widget
 * tree, the voice stack, the network protocol. What ties it to Android is a
 * small surface: finding the loaded game image, knowing where game data lives,
 * writing to the log, and talking to the Java UI. Sixteen files out of 756.
 *
 * Everything in that surface goes through this header, so the portable code
 * never has to know which platform it is on. Each platform supplies its own
 * implementation under platform/<name>/.
 *
 * This is not speculative generality: the whole point is that an iOS port has
 * to replace exactly these functions and nothing else in the portable third of
 * the tree. What it does NOT solve is the hard part — game/ is a binary
 * patching layer keyed to hundreds of offsets into the Android build of
 * libGame.so, and those have to be re-derived per platform. See docs/IOS.md.
 */
namespace Platform
{
	// ---------------------------------------------------------------------
	// Module lookup
	// ---------------------------------------------------------------------

	/**
	 * Base address of a loaded shared image, or 0 if it is not loaded.
	 *
	 * On Android this is a dlopen/dladdr pair against a real .so. On iOS the
	 * game is a statically linked Mach-O and the equivalent is a walk of the
	 * dyld image list — same answer, completely different mechanism, which is
	 * why callers must not assume a file name maps to a file on disk.
	 */
	uintptr_t ModuleBase(const char* name);

	/// Base address of the GTA:SA engine image. Every game hook offsets from
	/// this, so it is worth having a name of its own.
	uintptr_t GameBase();

	// ---------------------------------------------------------------------
	// Filesystem
	// ---------------------------------------------------------------------

	/**
	 * Root the game reads its data from, with a trailing slash.
	 *
	 * Android puts this in shared external storage so players can drop their
	 * game files in by hand. iOS has no such place — an app can only read its
	 * own sandbox — so the two will never be the same path, and no caller
	 * should hardcode one.
	 */
	const char* DataPath();

	/// Directory holding the client's own settings, with a trailing slash.
	const char* ConfigPath();

	/// Creates `path` and any missing parents. Returns false on failure.
	bool EnsureDirectory(const char* path);

	// ---------------------------------------------------------------------
	// Logging
	// ---------------------------------------------------------------------

	enum class LogLevel
	{
		Info,
		Warning,
		Error
	};

	/// Writes one already-formatted line to the platform log.
	void LogLine(LogLevel level, const char* tag, const char* message);
}
