#!/usr/bin/env bash
#
# Builds the Android APK locally.
#
#   tools/build-apk.sh              # debug build
#   tools/build-apk.sh --release    # release build (unsigned unless a keystore is configured)
#   tools/build-apk.sh --clean      # wipe build output first
#
# Requires the Android SDK with NDK 26.2.11394342 and JDK 17+. If you would
# rather not install any of that, push the branch instead and let
# .github/workflows/android.yml build it — the APK lands as a CI artifact.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT="$ROOT/client/android"
JNILIBS="$PROJECT/app/src/main/jniLibs/arm64-v8a"

VARIANT=debug
CLEAN=0

for arg in "$@"; do
	case "$arg" in
		--release) VARIANT=release ;;
		--debug)   VARIANT=debug ;;
		--clean)   CLEAN=1 ;;
		-h|--help) sed -n '2,13p' "${BASH_SOURCE[0]}"; exit 0 ;;
		*) echo "unknown option: $arg" >&2; exit 2 ;;
	esac
done

say()  { printf '\033[1;36m==>\033[0m %s\n' "$*"; }
fail() { printf '\033[1;31m[x]\033[0m %s\n' "$*" >&2; exit 1; }

SDK="${ANDROID_HOME:-${ANDROID_SDK_ROOT:-}}"
[ -n "$SDK" ] || fail "ANDROID_HOME (or ANDROID_SDK_ROOT) is not set — point it at your Android SDK"
[ -d "$SDK" ] || fail "Android SDK not found at $SDK"

# The native build links against the vendored sources but the APK will not run
# without the prebuilt libraries, so fetch them before Gradle packages anything.
if [ ! -f "$JNILIBS/libGame.so" ]; then
	say "prebuilt libraries missing, fetching"
	"$ROOT/tools/fetch-prebuilt.sh"
fi

cd "$PROJECT"

if [ "$CLEAN" = 1 ]; then
	say "cleaning"
	./gradlew clean --no-daemon
fi

case "$VARIANT" in
	debug)   TASK=assembleDebug ;;
	release) TASK=assembleRelease ;;
esac

say "running $TASK"
./gradlew "$TASK" --no-daemon

apk="$(find "$PROJECT/app/build/outputs/apk/$VARIANT" -name '*.apk' -print -quit 2>/dev/null || true)"
if [ -n "$apk" ]; then
	say "built: $apk"
	ls -lh "$apk"
else
	fail "Gradle reported success but no APK was found under app/build/outputs/apk/$VARIANT"
fi
