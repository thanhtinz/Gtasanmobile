#!/usr/bin/env bash
#
# Fetches everything needed to build and run the game server:
#   - the Pawn compiler (built from source; the release binaries are 32-bit only
#     and awkward on a modern host)
#   - open.mp standard library includes
#   - the MySQL plugin include (only needed for the MySQL build)
#   - the open.mp server binary
#
# Third-party sources land in vendor/, which is git-ignored, so the repository
# stays free of downloaded binaries.
#
# Usage: tools/server-setup.sh [--no-server] [--no-mysql]

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENDOR="$ROOT/vendor"
WANT_SERVER=1
WANT_MYSQL=1

for arg in "$@"; do
	case "$arg" in
		--no-server) WANT_SERVER=0 ;;
		--no-mysql)  WANT_MYSQL=0 ;;
		-h|--help)   sed -n '2,13p' "${BASH_SOURCE[0]}"; exit 0 ;;
		*) echo "unknown option: $arg" >&2; exit 2 ;;
	esac
done

say()  { printf '\033[1;36m==>\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m[!]\033[0m %s\n' "$*" >&2; }

mkdir -p "$VENDOR/include" "$ROOT/server/gamemodes"

fetch_repo() {
	local url="$1" dir="$VENDOR/$2"
	if [ -d "$dir/.git" ]; then
		say "updating $2"
		git -C "$dir" fetch --depth 1 -q origin HEAD && git -C "$dir" reset --hard -q FETCH_HEAD
	else
		say "cloning $2"
		git clone --depth 1 -q "$url" "$dir"
	fi
}

# --- includes ---------------------------------------------------------------
fetch_repo https://github.com/openmultiplayer/omp-stdlib.git omp-stdlib
cp -f "$VENDOR/omp-stdlib"/*.inc "$VENDOR/include/"

if [ "$WANT_MYSQL" = 1 ]; then
	fetch_repo https://github.com/pBlueG/SA-MP-MySQL.git sa-mp-mysql
	# The repo ships a CMake template whose only placeholder is a version string
	# inside a comment, so a plain substitution yields a usable include.
	sed 's/@MYSQL_PLUGIN_VERSION@/R41-4/' \
		"$VENDOR/sa-mp-mysql/a_mysql.inc.in" > "$VENDOR/include/a_mysql.inc"
	say "generated a_mysql.inc"
fi

# --- pawn compiler ----------------------------------------------------------
if [ -x "$VENDOR/pawncc/bin/pawncc" ]; then
	say "pawncc already built"
else
	fetch_repo https://github.com/pawn-lang/compiler.git pawn-compiler
	say "building pawncc"
	cmake -S "$VENDOR/pawn-compiler/source/compiler" -B "$VENDOR/pawn-compiler/build" \
		-DCMAKE_BUILD_TYPE=Release >/dev/null
	# Only the pawncc target: the sibling pawnruns interpreter does not build on
	# 64-bit hosts (a static_assert in amx.h) and nothing here needs it.
	cmake --build "$VENDOR/pawn-compiler/build" --target pawncc \
		-j "$(nproc 2>/dev/null || echo 2)" >/dev/null
	mkdir -p "$VENDOR/pawncc/bin"
	cp -f "$VENDOR/pawn-compiler/build/pawncc" "$VENDOR/pawncc/bin/"
	cp -f "$VENDOR/pawn-compiler/build/libpawnc.so" "$VENDOR/pawncc/bin/" 2>/dev/null || true
	say "pawncc ready"
fi

# --- open.mp server ---------------------------------------------------------
if [ "$WANT_SERVER" = 1 ]; then
	if [ -x "$ROOT/server/omp-server" ]; then
		say "open.mp server already present"
	else
		say "downloading open.mp server"
		url="$(curl -fsSL https://api.github.com/repos/openmultiplayer/open.mp/releases/latest 2>/dev/null \
			| grep -o 'https://[^"]*open\.mp-linux-x86\.tar\.gz' | head -1 || true)"
		if [ -z "$url" ]; then
			warn "could not resolve the open.mp download URL (no network, or GitHub unreachable)."
			warn "Download the Linux package from https://open.mp/download and extract"
			warn "'omp-server' and 'components/' into $ROOT/server/."
		else
			tmp="$(mktemp -d)"
			curl -fsSL "$url" -o "$tmp/omp.tar.gz"
			tar -xzf "$tmp/omp.tar.gz" -C "$tmp"
			src="$(find "$tmp" -name omp-server -type f | head -1)"
			cp -f "$src" "$ROOT/server/omp-server"
			chmod +x "$ROOT/server/omp-server"
			cp -rf "$(dirname "$src")/components" "$ROOT/server/" 2>/dev/null || true
			rm -rf "$tmp"
			say "open.mp server installed"
		fi
	fi
fi

say "done — run tools/server-build.sh next"
