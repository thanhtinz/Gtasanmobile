# Port sang iOS — hiện trạng và đường đi

Tài liệu này ghi lại kết quả khảo sát thật trên mã nguồn client, để ai muốn làm
tiếp không phải đo lại từ đầu.

**Tóm tắt: chưa có client iOS chạy được, và chặng còn lại rất dài.** Nhưng phần
lớn mã nguồn thì dùng lại được, và phần khó nằm ở đúng một chỗ có thể chỉ tên rõ
ràng.

## Con số

| | |
|---|---|
| Tổng mã nguồn client | 756 file, ~185.000 dòng |
| File đụng thẳng API Android | **16** |
| Dùng lại được cho iOS | ~110.000 dòng (~60%) |
| Offset cứng vào `libGame.so` | **805 vị trí, 777 giá trị khác nhau** |
| Hook móc theo tên hàm qua `dlsym` | ~274 vị trí |

Con số 16 file làm việc này trông dễ hơn thực tế rất nhiều.

## Phần dùng lại được

| Thành phần | File | Dòng | Ghi chú |
|---|---|---|---|
| `vendor/raknet` | 136 | 41.860 | **Đã có sẵn nhánh `__APPLE__`** trong mã |
| `vendor/imgui` | 12 | 38.289 | Chỉ có phần lõi, không kèm backend nào |
| `game/Enums/` | 28 | 20.179 | Dữ liệu thuần, **0 offset** |
| `net/` | 28 | 10.938 | Chỉ 3 file dính JNI |
| `vendor/` linh tinh | 27 | 7.159 | SimpleIni, inih, speex, encoding |
| `game/Core`, `Tasks`, `Events`, `Widgets` | 86 | 6.031 | Tổng cộng chỉ 8 offset |
| `voice/` trừ `Record` | 57 | 4.460 | BASS + Opus, API đa nền tảng |
| `gui/widgets/` + `dialogs/` | 28 | ~1.900 | ImGui thuần |

## Phần phải viết lại

| Thành phần | Dòng | Vì sao |
|---|---|---|
| `game/` (root, RW, Animation, Plugins, Entity, Models...) | ~46.000 | 777 offset chỉ đúng với `libGame.so` bản Android |
| `gui/imguiwrapper.cpp` + `imguirenderer.cpp` | ~600 | Vẽ bằng `RwIm2DRenderIndexedPrimitive` mượn từ engine qua `dlsym`; iOS phải viết backend Metal |
| `vendor/GlossHook` + `armhook` | 1.624 | ShadowHook/GlossHook chỉ có binary ARM/ARM64 dựng sẵn, không có mã nguồn, không có bản iOS. Phải thay bằng [Dobby](https://github.com/jmpews/Dobby) |
| `graphics/` | 671 | Sinh mã shader GLSL ES lúc chạy, vô dụng trên Metal |
| `java/` | 316 | 100% cầu JNI, viết lại bằng Objective-C |
| `voice/Record.cpp` | ~250 trong 678 | Engine OpenSL ES. Nhưng BASS đã tự lo thu âm rồi, nên khối này có thể bỏ hẳn |

## Chỗ khó nhất, và vì sao nó khó hơn vẻ ngoài

Client hoạt động bằng cách móc vào engine GTA:SA. Hiện có ~1.000 điểm móc:

- **~805 vị trí dùng offset cứng** kiểu `g_libGTASA + 0x496798`
- **~274 vị trí móc theo tên hàm C++** kiểu `CHook::InlineHook("_ZN5CGame20InitialiseRenderWareEv", ...)`

Nhóm thứ hai trông như dùng lại được, nhưng **không**. Nó chạy được trên Android
chỉ vì `libGame.so` là **thư viện chia sẻ (ELF) còn giữ bảng ký hiệu**, nên
`dlsym` tra được tên hàm. Binary GTA:SA trên iOS là **Mach-O liên kết tĩnh và đã
strip** — không export tên hàm nào cả. Nghĩa là cả 274 chỗ đó cũng phải chuyển
thành dò offset thủ công.

Tổng lại: khoảng **1.000 địa chỉ phải dò lại bằng tay** trên một binary được
biên dịch bởi trình biên dịch khác, phiên bản game khác, cách sắp xếp hàm khác.
Đây là công việc dịch ngược lặp đi lặp lại, cần có binary trong tay lẫn máy thật
để đọc log crash. Không có đường tắt, và không tự động hoá hết được.

Điểm khởi đầu tham khảo: cộng đồng iOSGods đã công bố offset cho GTA:SA iOS bản
1.6 và 2.2. Dùng làm mồi được, nhưng phải tự xác minh lại trên đúng bản IPA của
mình.

## Việc đã làm để chuẩn bị

- `cpp/samp/platform/api.h` — lớp trừu tượng cho phần phụ thuộc hệ điều hành:
  tìm base address của image, đường dẫn dữ liệu, ghi log. Bản Android nằm gọn
  trong `platform/android/`.
- Gỡ `<android/log.h>` khỏi `log.h`, nên nó không còn lan qua `main.h` ra khắp
  cây mã nguồn.
- Gỡ `dlfcn.h` khỏi `util/CUtil.cpp`.
- Gom 5 chỗ hardcode `/storage/emulated/0/GTA/` về `Platform::DataPath()`.
  Đường dẫn này **không thể đúng trên iOS về mặt nguyên tắc** — app iOS chỉ đọc
  được sandbox của chính nó.

## Đường phát hành đã chốt

Bạn có iPhone **không jailbreak** và **không có máy Mac**. Vẫn đi được:

1. **GitHub Actions chạy máy ảo macOS** (`macos-14`, có sẵn Xcode) biên dịch ra
   dylib. Repo public nên miễn phí.
2. Tải dylib về máy Windows.
3. **[Sideloadly](https://sideloadly.io/) chạy trên Windows**, nhúng dylib vào
   file IPA GTA:SA của bạn, ký lại rồi cài lên iPhone.

Không cần jailbreak, không cần mua Mac.

**Hạn chế:** tài khoản Apple miễn phí thì app hết hạn sau 7 ngày, phải cài lại.
Tài khoản nhà phát triển 99 USD/năm thì được 1 năm.

**Bạn vẫn cần file IPA GTA:SA bản iOS của chính mình** — vừa để chơi, vừa để dò
offset. Không có nó thì không xác minh được hook nào.

## Nếu muốn có người chơi iOS ngay

Không cần chờ port. **Server open.mp của bạn đã phục vụ được iOS rồi** — nó nói
giao thức 0.3.7, và client SA-MP cho iOS đang lưu hành kết nối vào được.

Đổi lại: client đó đóng mã nguồn, nên không có nút MENU và bạn không sửa được gì
trong đó. Người chơi iOS vẫn vào chơi, làm nghề, dùng `/menu` bằng cách gõ lệnh
như bình thường.

## Pháp lý

Grand Theft Auto: San Andreas thuộc Rockstar Games / Take-Two Interactive. Dự án
này không liên kết với họ và không phát tán game của họ. Bạn phải tự sở hữu bản
game hợp pháp — trên iOS nghĩa là file IPA từ bản bạn đã mua.
