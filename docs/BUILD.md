# Build hướng dẫn — Android

## Yêu cầu

| Thứ | Phiên bản | Ghi chú |
|---|---|---|
| JDK | 17+ | Gradle 8.13 chạy tốt trên 17 và 21 |
| Android SDK | compileSdk 36 | Cài qua Android Studio hoặc `cmdline-tools` |
| Android NDK | **26.2.11394342** | Phải đúng phiên bản, khớp `ndkVersion` trong `app/build.gradle.kts` |
| CMake | 3.22.1 | Cài qua `sdkmanager "cmake;3.22.1"` |

Cài NDK và CMake bằng dòng lệnh:

```bash
sdkmanager --install "ndk;26.2.11394342" "cmake;3.22.1"
```

## Build

```bash
tools/fetch-prebuilt.sh          # bắt buộc chạy trước lần build đầu
tools/build-apk.sh               # debug
tools/build-apk.sh --release     # release (chưa ký)
tools/build-apk.sh --clean       # dọn build cũ trước
```

APK nằm ở `client/android/app/build/outputs/apk/<variant>/`.

Hoặc gọi thẳng Gradle:

```bash
cd client/android
./gradlew assembleDebug
```

## Build bằng CI

Push bất kỳ nhánh nào lên GitHub, workflow `Android` sẽ chạy và đính APK debug
vào artifact `apk-debug` của run đó. Muốn có bản release thì chạy workflow thủ
công (`Run workflow`) và bật `build_release`.

Cách này tiện khi bạn không muốn cài Android SDK, hoặc máy không phải Linux/Mac.

## `tools/fetch-prebuilt.sh` làm gì

Tải 7 thư viện native dựng sẵn từ upstream (pin theo commit, verify SHA-256) và
chép thêm `libGlossHook.so` từ cây source vào `jniLibs/arm64-v8a/`:

| File | Vai trò |
|---|---|
| `libGame.so` | Engine GTA:SA Android đã patch — mọi hook trong `cpp/samp/game/hooks.cpp` tính offset dựa trên nó |
| `libbass.so`, `libbass_ssl.so` | Audio (BASS), được CMake link trực tiếp |
| `libGlossHook.so` | Thư viện hook, `libsamp.so` có `DT_NEEDED` trỏ tới |
| `libopenal.so`, `libVendor_mpg123.so` | Audio |
| `libc++_shared.so`, `libz.so` | Runtime mà `libGame.so` cần |

Các file này **không nằm trong repo** và bị `.gitignore` chặn. Lý do: `libGame.so`
là engine của Rockstar đã bị patch, không phải thứ chúng ta có quyền phát tán lại.

Cờ hữu ích:

```bash
tools/fetch-prebuilt.sh --verify    # chỉ kiểm tra checksum, không tải
tools/fetch-prebuilt.sh --force     # tải lại toàn bộ
```

## Lỗi thường gặp

**`NDK at ... did not have a source.properties file` hoặc sai version NDK**
→ Cài đúng `26.2.11394342`. Đổi `ndkVersion` sang bản khác có thể build được
nhưng chưa được kiểm chứng với `-Wl,-z,max-page-size=16384` (hỗ trợ 16 KB page
size cho máy Android mới).

**`UnsatisfiedLinkError: dlopen failed: library "libGame.so" not found`**
→ Chưa chạy `tools/fetch-prebuilt.sh`, hoặc APK build ra trước khi có `jniLibs`.
Chạy script rồi build lại.

**App mở lên rồi tắt ngay**
→ Nhiều khả năng thiếu game data ở `/storage/emulated/0/GTA/`. Xem log ở
`/storage/emulated/0/GTA/logcat.txt`.

**Gradle không resolve được dependency**
→ Kiểm tra mạng tới `dl.google.com`, `repo1.maven.org` và `jitpack.io`. Một số
mạng doanh nghiệp chặn `dl.google.com`, mà đó là nơi chứa Android Gradle Plugin
và toàn bộ AndroidX.

## Ghi chú kỹ thuật

**Firebase đã gỡ.** Upstream apply plugin `com.google.gms.google-services` và
`com.google.firebase.crashlytics`, khiến `assembleRelease` fail ở task
`processGoogleServices` khi thiếu `google-services.json`. Nhưng mọi lời gọi
Firebase trong Java đều đã bị comment sẵn, và `cpp/samp/crashlytics.h` là header
tự `dlopen("libcrashlytics.so")` — không có file đó thì mọi hàm thành no-op,
không phát sinh phụ thuộc lúc link. Nên đã bỏ sạch plugin, dependency và
permission FCM. Dòng `firebase::crashlytics::Log()` trong `cpp/samp/main.cpp`
được giữ nguyên vì nó vô hại.

**Chỉ build ARM64.** `abiFilters` giới hạn `arm64-v8a`. Nhánh `GTA-2.10` của
upstream có hỗ trợ ARM32 nếu bạn cần máy cũ.

**Đừng đổi `applicationId`.** Tên hàm JNI bám theo package: ví dụ
`Java_com_gta_game_SAMP_initializeSAMP` trong `cpp/samp/main.cpp`. Đổi
`com.gta.game` là phải rename đồng loạt cả phía C++ trong `cpp/samp/java/`.
