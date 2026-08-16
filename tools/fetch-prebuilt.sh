#!/usr/bin/env bash
#
# Fetches the prebuilt native libraries the APK needs at runtime.
#
# These are deliberately not committed. libGame.so is the GTA:SA Android engine
# (Rockstar's, patched by the upstream SA-MP Mobile authors) and the rest are
# third-party audio/compression libraries — none of them are ours to republish,
# so the build pulls them from upstream at a pinned commit instead.
#
# You still need a legally obtained copy of GTA: San Andreas for Android to get
# the game data itself; see docs/BUILD.md.
#
# Usage:
#   tools/fetch-prebuilt.sh            # fetch what is missing
#   tools/fetch-prebuilt.sh --force    # re-download everything
#   tools/fetch-prebuilt.sh --verify   # only check what is already on disk

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="$ROOT/client/android/app/src/main/jniLibs/arm64-v8a"

# Pinned to kuzia15/SAMP-Mobile @ GTA-2.11. Bump this together with the hashes
# below when pulling in a newer upstream client.
UPSTREAM_REPO="kuzia15/SAMP-Mobile"
UPSTREAM_COMMIT="5e531514a0bbb3d56b1a72e51fa44b18cfaf2ee2"
BASE_URL="https://raw.githubusercontent.com/${UPSTREAM_REPO}/${UPSTREAM_COMMIT}/app/src/main/jniLibs/arm64-v8a"

# filename:sha256
#
# Only libraries that nothing else in the build produces belong here. Anything
# cpp/samp/CMakeLists.txt links by path — libbass.so, libbass_ssl.so,
# libGlossHook.so — is copied into the APK by the Android Gradle Plugin itself,
# and fetching a second copy into jniLibs makes packaging fail with
# "DuplicateRelativeFileException: 2 files found with path lib/arm64-v8a/...".
#
# libc++_shared.so is safe to ship: ANDROID_STL is unset, so AGP builds against
# the default c++_static and never emits a shared libc++ of its own. If that
# ever changes, this is the first entry to drop.
LIBS=(
	"libGame.so:4c6a7445e30b27afdda781302e4db9bac89c28fc1181b68b1eef16f84d6a282e"
	"libc++_shared.so:c4c2fe5cbcb1fba0003a31fc7ab29a9bb12df6cc187ec45a806462540e83d93b"
	"libopenal.so:d49804fa74d9c4d5e311251e1b8f2428ea25ea6c2dd98121a8aaf5b49533695c"
	"libVendor_mpg123.so:932ed9c1fa1df1d68283c019b0ef1f411e8732d77c1dabde0e358f956b6c784e"
	"libz.so:0b6cb59b4f41d735234a175302824416fe0146ec5b7c0f5873b25ef583f5cefc"
)

FORCE=0
VERIFY_ONLY=0
for arg in "$@"; do
	case "$arg" in
		--force)   FORCE=1 ;;
		--verify)  VERIFY_ONLY=1 ;;
		-h|--help) sed -n '2,17p' "${BASH_SOURCE[0]}"; exit 0 ;;
		*) echo "unknown option: $arg" >&2; exit 2 ;;
	esac
done

say()  { printf '\033[1;36m==>\033[0m %s\n' "$*"; }
fail() { printf '\033[1;31m[x]\033[0m %s\n' "$*" >&2; exit 1; }

hash_of() { sha256sum "$1" | cut -d' ' -f1; }

mkdir -p "$DEST"

missing=0
for entry in "${LIBS[@]}"; do
	name="${entry%%:*}"
	want="${entry##*:}"
	path="$DEST/$name"

	if [ -f "$path" ] && [ "$FORCE" = 0 ]; then
		got="$(hash_of "$path")"
		if [ "$got" = "$want" ]; then
			continue
		fi
		if [ "$VERIFY_ONLY" = 1 ]; then
			fail "$name has an unexpected checksum (got $got, want $want)"
		fi
		say "$name checksum mismatch, re-downloading"
	elif [ "$VERIFY_ONLY" = 1 ]; then
		fail "$name is missing — run tools/fetch-prebuilt.sh"
	fi

	say "downloading $name"
	if ! curl -fsSL --retry 3 --retry-delay 2 -o "$path.tmp" "$BASE_URL/$name"; then
		rm -f "$path.tmp"
		fail "could not download $name from $BASE_URL"
	fi

	got="$(hash_of "$path.tmp")"
	if [ "$got" != "$want" ]; then
		rm -f "$path.tmp"
		fail "$name failed verification (got $got, want $want) — upstream may have been re-tagged"
	fi

	mv -f "$path.tmp" "$path"
	missing=$((missing + 1))
done

# Stale copies of the CMake-provided libraries would collide with the ones the
# Android Gradle Plugin packages, so an older checkout that ran the previous
# version of this script gets cleaned up rather than failing the next build.
for stale in libbass.so libbass_ssl.so libGlossHook.so; do
	if [ -f "$DEST/$stale" ]; then
		if [ "$VERIFY_ONLY" = 1 ]; then
			fail "$stale is in jniLibs but the build packages it from CMake — run tools/fetch-prebuilt.sh"
		fi
		say "removing $stale (packaged from the CMake build, not from jniLibs)"
		rm -f "$DEST/$stale"
	fi
done

if [ "$VERIFY_ONLY" = 1 ]; then
	say "all prebuilt libraries present and verified"
elif [ "$missing" -eq 0 ]; then
	say "prebuilt libraries already up to date"
else
	say "fetched $missing librar$([ "$missing" -eq 1 ] && echo y || echo ies) into ${DEST#"$ROOT"/}"
fi
