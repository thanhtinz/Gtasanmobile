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
VENDOR="$ROOT/client/android/app/src/main/cpp/samp/vendor"

# Pinned to kuzia15/SAMP-Mobile @ GTA-2.11. Bump this together with the hashes
# below when pulling in a newer upstream client.
UPSTREAM_REPO="kuzia15/SAMP-Mobile"
UPSTREAM_COMMIT="5e531514a0bbb3d56b1a72e51fa44b18cfaf2ee2"
BASE_URL="https://raw.githubusercontent.com/${UPSTREAM_REPO}/${UPSTREAM_COMMIT}/app/src/main/jniLibs/arm64-v8a"

# filename:sha256
LIBS=(
	"libGame.so:4c6a7445e30b27afdda781302e4db9bac89c28fc1181b68b1eef16f84d6a282e"
	"libbass.so:0d55ab1e670842904b29a6bf3643e8259a34814e81c2cc53db53dde3fc3291df"
	"libbass_ssl.so:5deae657d58d15c39e658ae42442b8659773f2814132f5099f45fa0f14ea714a"
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

# GlossHook is linked by cpp/samp/CMakeLists.txt as a shared library, so
# libsamp.so records a DT_NEEDED entry for it — but upstream never copies it
# into jniLibs, which leaves the APK one dlopen away from an
# UnsatisfiedLinkError. It ships in the source tree under an ABI directory
# named "ARM64" rather than "arm64-v8a", so Gradle cannot pick it up on its
# own and it is copied here instead.
GLOSS_SRC="$VENDOR/GlossHook/libs/ARM64/libGlossHook.so"
if [ -f "$GLOSS_SRC" ]; then
	if [ "$VERIFY_ONLY" = 1 ]; then
		[ -f "$DEST/libGlossHook.so" ] || fail "libGlossHook.so is missing — run tools/fetch-prebuilt.sh"
	elif [ ! -f "$DEST/libGlossHook.so" ] || [ "$FORCE" = 1 ] \
		|| [ "$(hash_of "$GLOSS_SRC")" != "$(hash_of "$DEST/libGlossHook.so")" ]; then
		say "copying libGlossHook.so from the vendored source tree"
		cp -f "$GLOSS_SRC" "$DEST/libGlossHook.so"
	fi
else
	echo "warning: $GLOSS_SRC not found; skipping" >&2
fi

if [ "$VERIFY_ONLY" = 1 ]; then
	say "all prebuilt libraries present and verified"
elif [ "$missing" -eq 0 ]; then
	say "prebuilt libraries already up to date"
else
	say "fetched $missing librar$([ "$missing" -eq 1 ] && echo y || echo ies) into ${DEST#"$ROOT"/}"
fi
