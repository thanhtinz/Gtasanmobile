# Server — open.mp + gamemode

Server chạy open.mp, phục vụ giao thức 0.3.7, nên **client mobile và client PC
vào cùng một server và chơi chung**.

## Chạy nhanh

```bash
tools/server-setup.sh      # tải pawncc, include, và bản open.mp server
tools/server-build.sh      # biên dịch gamemode (SQLite, không cần cài gì thêm)
cd server && ./omp-server
```

Mặc định server nghe cổng `7777`. Sửa `server/config.json` để đổi tên server,
cổng, và **mật khẩu RCON** (đang là `CHANGE_ME` — đổi trước khi mở cho người lạ).

Sau đó trỏ client về server này: sửa `SAMP_DEFAULT_HOST` và `SAMP_DEFAULT_PORT`
trong `client/android/app/src/main/cpp/samp/settings.h`.

## Database

Gamemode viết một lần, chạy được trên hai engine. Lựa chọn ở lúc build:

```bash
tools/server-build.sh            # SQLite -> server/gamemodes/gtasan.amx
tools/server-build.sh --mysql    # MySQL  -> server/gamemodes/gtasan-mysql.amx
```

SQLite không cần cài gì, file nằm ở `server/gtasan.db`. Hợp cho server nhỏ và
để test. MySQL hợp khi đông người và khi bạn muốn làm web panel sau này — chép
`server/mysql.ini.example` thành `server/mysql.ini` rồi điền thông tin, và sửa
`main_scripts` trong `config.json` thành `"gtasan-mysql 1"`.

Mọi truy vấn đi qua lớp trừu tượng ở `server/pawn/core/db.inc`. Code gameplay
không bao giờ gọi thẳng native của driver, nên hai bản build sinh ra từ cùng
một mã nguồn.

## Nghề nghiệp

25 nghề, chia 4 nhóm, định nghĩa trong bảng `g_Jobs` ở
`server/pawn/jobs/jobs.inc`:

| Nhóm | Nghề |
|---|---|
| Lái xe / vận chuyển | Taxi, xe bus, xe tải, xe rác, chuyển phát, xe cứu thương, phi công, tàu thuỷ |
| Khai thác / sản xuất | Thợ mỏ, thợ gỗ, ngư dân, nông dân, thợ săn, thợ sửa xe, công nhân dầu |
| Phe phái | Cảnh sát, bác sĩ, cứu hoả, vệ sĩ, cứu hộ giao thông |
| Phi pháp | Trộm xe, cướp cửa hàng, buôn hàng cấm, rửa tiền, cướp ngân hàng |

Vòng lặp chơi: nhận nghề → tới điểm làm việc → vào ca → chạy qua từng checkpoint
→ mỗi chặng trả tiền theo quãng đường thực đi → hết ca có thưởng.

**Bậc nghề tính riêng từng nghề**, lưu trong bảng `job_progress`. Đổi nghề không
mất bậc nghề cũ. Bậc 10 thì lương gấp đôi.

Nghề nhóm phi pháp trả cao hơn nhưng cộng sao truy na mỗi chặng, và cảnh sát
nhận được thông báo vị trí. Cảnh sát dùng `/arrest` để bắt.

### Thêm nghề mới

Thêm một dòng vào `g_Jobs`. Không phải sửa logic nào khác:

```pawn
{"Ten nghe", "Mo ta ngan.", JOBCAT_DRIVING, 5, 800, 1500, 20, 253, 420, 0, x, y, z, POOL_CITY},
//  ten        mo ta        nhom            lvl pay_min pay_max exp skin xe  sao  toa do  pool diem den
```

Pool điểm đến có sẵn: `POOL_CITY`, `POOL_FREIGHT`, `POOL_RURAL`, `POOL_AIR`,
`POOL_SEA`, `POOL_CRIME`.

**Lưu ý:** id nghề chính là chỉ số trong bảng, và bảng `job_progress` lưu theo
id đó. Chèn nghề vào giữa bảng sẽ làm lệch bậc nghề của người chơi cũ — thêm vào
cuối nhóm thì an toàn hơn.

## Panel

`/menu` mở bảng điều khiển, và client mobile có nút MENU trên màn hình gọi đúng
lệnh đó. Panel dựng bằng `ShowPlayerDialog`, mà client Android render bằng view
native có RecyclerView cuộn thật — nên đổi menu chỉ cần sửa server, không cần
phát hành APK mới.

Cây menu ở `server/pawn/core/menu.inc`, phần xử lý bấm ở
`server/pawn/core/dialog_router.inc`.

Lệnh chat vẫn giữ nguyên và vẫn chạy — panel gọi cùng hàm chứ không chép lại
logic.

## Bảo mật mật khẩu — đọc trước khi mở server thật

Mật khẩu lưu bằng SHA-256 với salt ngẫu nhiên riêng từng tài khoản. Không lưu
plaintext, không dùng salt chung.

Nhưng **SHA-256 không phải hàm băm mật khẩu tốt**: nó nhanh, mà nhanh đúng là
thứ bạn không muốn ở đây — lộ database thì crack bằng GPU rất nhanh. open.mp
khuyên dùng bcrypt, và lời khuyên đó đúng.

Tôi không dùng bcrypt vì nó là plugin riêng, server chủ phải tự tìm bản binary
khớp hệ điều hành — mà giữ gamemode không phụ thuộc plugin chính là thứ làm nó
deploy được ở mọi nơi. Nếu server bạn đông và đáng bị tấn công, hãy cài plugin
bcrypt rồi thay hai lời gọi `SHA256_PassHash` trong
`server/pawn/player/account.inc`. Không chỗ nào khác trong gamemode đụng tới
chuỗi hash.

## Chống gian lận

Số dư tiền do server quyết định, không tin client. `Player[][pCash]` là nguồn
sự thật, HUD của client được đồng bộ lại sau mỗi thay đổi. Điều này quan trọng
hơn bình thường ở đây vì client mobile bị chỉnh sửa khá phổ biến.

Độ truy nã cũng do server giữ. Chết chỉ giảm một nửa số sao chứ không xoá sạch,
nếu không thì tự sát sẽ là cách rẻ nhất để thoát cảnh sát.

Mọi biến động tiền được ghi vào bảng `money_log`, khoản lớn ghi thêm ra
`server/logs/economy-*.log`.

## Cấu trúc mã nguồn

```
server/pawn/
├── gamemode.pwn        điểm vào, thứ tự include
├── core/
│   ├── config.inc      hằng số chỉnh được
│   ├── db.inc          lớp trừu tượng SQLite/MySQL
│   ├── schema.inc      định nghĩa bảng
│   ├── cmd.inc         bộ điều phối lệnh (tự viết, không cần zcmd)
│   ├── params.inc      parse tham số (tự viết, không cần sscanf)
│   ├── menu.inc        cây panel
│   └── dialog_router.inc
├── player/             tài khoản, tiền, level, ngân hàng, sự kiện
├── world/              zone, spawn, truy nã và nhà tù
└── jobs/jobs.inc       khung nghề + bảng 25 nghề
```

Không phụ thuộc plugin nào. Bộ điều phối lệnh và parser tham số viết tay thay
cho zcmd/sscanf, để server chủ chỉ cần tải open.mp là chạy được.

**Chữ hiển thị trong game viết không dấu.** Client SA-MP dùng bảng ký tự bitmap
riêng, không có dấu tiếng Việt — trên cả PC lẫn Android. Comment và tài liệu thì
viết bình thường.
