# GTA:SA Mobile

Client SA-MP cho GTA: San Andreas trên di động, kèm launcher riêng.

Dự án dựa trên [`kuzia15/SAMP-Mobile`](https://github.com/kuzia15/SAMP-Mobile) nhánh `GTA-2.11`
(tác giả gốc: [bkuzn2](https://github.com/bkuzn2) và [kuzia15](https://github.com/kuzia15)).
Phần source client được đưa vào `client/android/`, pin ở commit `5e53151`.

## Trạng thái

| Phần | Trạng thái |
|---|---|
| Server + 25 nghề nghiệp | ✅ Biên dịch được, cả SQLite lẫn MySQL — xem [`docs/SERVER.md`](docs/SERVER.md) |
| Panel cho player (`/menu` + nút MENU trong game) | ✅ Xong |
| Android — hạ tầng build | ✅ Build qua GitHub Actions hoặc `tools/build-apk.sh` |
| Android — launcher (danh sách server) | 🚧 Mới có nickname; danh sách server làm sau |
| Mic in-game | 🚧 Client đã có sẵn code SampVoice nhưng đang bị tắt — đang bật lại |
| iOS | ⚠️ Chưa chạy được. Đã tách lớp nền tảng để mở đường — xem [`docs/IOS.md`](docs/IOS.md) |

## Bạn cần gì để chơi

1. **Máy Android ARM64, Android 8.0 trở lên.** Client này chỉ build cho `arm64-v8a`.
2. **Bản GTA: San Andreas cho Android của chính bạn.** Repo này không chứa và
   không phát tán game data của Rockstar. Bạn cần tự trích xuất từ bản game hợp
   pháp mà bạn sở hữu, rồi chép vào `/storage/emulated/0/GTA/`.
3. **File APK.** Tải từ artifact của GitHub Actions, hoặc tự build theo
   [`docs/BUILD.md`](docs/BUILD.md).

## Bố cục repo

```
client/android/     Source client + launcher (Java + C++/NDK, Gradle)
server/             Server open.mp + gamemode Pawn (nghề nghiệp, panel, kinh tế)
tools/              Script build cho cả client lẫn server
docs/               Hướng dẫn build và ghi chú kỹ thuật
.github/workflows/  CI build APK
```

## Build nhanh

Server:

```bash
tools/server-setup.sh       # tải pawncc, include, open.mp server
tools/server-build.sh       # biên dịch gamemode
cd server && ./omp-server
```

Client Android:

```bash
tools/fetch-prebuilt.sh     # tải các thư viện native dựng sẵn
tools/build-apk.sh          # build APK debug
```

Chi tiết và cách xử lý lỗi thường gặp: [`docs/BUILD.md`](docs/BUILD.md).

Không muốn cài Android SDK thì push nhánh lên, CI sẽ build và đính APK vào
artifact của workflow run.

## Khác gì so với upstream

- **Gỡ Firebase.** Upstream không build được nếu thiếu `google-services.json`
  của tác giả gốc, trong khi mọi lời gọi Firebase phía Java đều đã bị comment
  từ trước. Đã bỏ plugin, dependency và các permission liên quan.
- **Không commit thư viện native dựng sẵn.** `libGame.so` là engine GTA:SA của
  Rockstar; `tools/fetch-prebuilt.sh` tải về lúc build thay vì để trong repo.
- **Sửa 2 lỗi `#include` sai hoa/thường** (`playerTabList.h` → `playertablist.h`,
  `RGBA.h` → `rgba.h`). Upstream code trên hệ thống không phân biệt hoa thường
  nên không thấy; trên Linux là build hỏng ngay từ file đầu tiên.
- **Sửa icon app.** Manifest đang trỏ `android:icon` vào lớp nền của adaptive
  icon thay vì chính icon, và thiếu `roundIcon`.
- **Dọn repository Gradle chết** (Splunk MINT, AppLovin, Sonatype snapshots).

## Giấy phép và bản quyền

Mã nguồn launcher và tooling trong repo này viết cho dự án. Phần client SA-MP
kế thừa từ upstream — giữ nguyên credit tác giả gốc.

Grand Theft Auto: San Andreas là sản phẩm của Rockstar Games / Take-Two
Interactive. Dự án này **không** liên kết với họ, **không** phát tán game của
họ, và **không** thay thế việc mua game. Bạn phải tự sở hữu bản game hợp pháp.
