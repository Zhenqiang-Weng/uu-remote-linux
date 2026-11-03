# 独立会话与 Mac 兼容设置 / Independent sessions and Mac compatibility

这是基于上游 v0.2.45 的个人开发组件。原程序、账号登录方式和网络协议保持原样，
辅助脚本按设备保存配置，复用已登录的 `uuremote-viewer`。

This personal-development companion is based on upstream v0.2.45. It reuses the
signed-in viewer, stores per-device settings, and does not modify its network protocol.

## 安装 / Install

Linux 图形用户会话需要 Python 3、systemd 用户服务；设置窗口需要 `python3-gi` 和 GTK3。
原生 PNG 粘贴内置于更新后的 viewer；X11/XWayland 直接读取本地剪贴板。原生 Wayland 的 SDL2 窗口使用按需 `wl-paste`，需要 `wl-clipboard`。不需要 Mac 文件传输或终端权限。
按键适配器从本目录 C 源码编译，需要 `cc`；不包含预编译二进制。

Run as the desktop user, with Python 3 and a systemd user manager. The settings
window needs PyGObject/GTK3. Image transfer needs wl-clipboard, notify-send, and
UU file/terminal access to the Mac. A C compiler builds the optional input adapter.

```sh
./contrib/independent-viewer/install.sh
```

从本个人仓库编译并安装带新入口的主窗口 / Build and install the personal main window:

```sh
cargo build --release --locked --features native-viewer,viewer-sdl --bin uuremote-viewer
./contrib/independent-viewer/install.sh --viewer target/release/uuremote-viewer
```

`--viewer` 把个人版装到 `~/.local/lib/uuremote-personal/`，更新本用户的应用菜单入口，
备份此前的桌面入口；系统包中的程序仍保留。下次从应用菜单打开个人版即可。

`--viewer` installs the personal executable under `~/.local/lib/uuremote-personal/`
and updates the user's launcher, keeping a backup of the previous desktop entry.
It leaves the system package executable in place. Reopen the viewer from the app menu.

主窗口设备页提供 **独立远控**、**会话设置**；图片直接在远控窗口按 Ctrl+V 粘贴，常规设置里也可配置当前所选设备。
配置后，从主窗口或应用菜单打开该设备。独立会话有自己的桌面应用标识，
重复打开会恢复同一窗口，关闭主窗口不会结束独立会话；明确退出账号会关闭这些会话。
Dock 的固定功能仍由桌面环境管理。

The device page adds **Independent remote**, **Session settings**, and **Send screenshot**.
General settings can also configure the selected device. Each device has its own
desktop identity; reopening reuses its session, closing the launcher preserves it,
and explicit sign-out stops independent sessions. Use the desktop's normal pin action.

也可从命令行配置 / Command-line setup:

```sh
~/.local/bin/uuremote-independent configure DEVICE_ID 'Remote Mac' 4
~/.local/bin/uuremote-independent open DEVICE_ID 'Remote Mac' 4
```

平台值：Mac `4`，Windows / Linux `1`。设备 ID 不是密码；登录凭据仍只由原程序读取。
配置位于 `$XDG_CONFIG_HOME/uuremote/independent/`（默认 `~/.config/...`，目录 0700、文件 0600），
桌面入口位于 `$XDG_DATA_HOME/applications/`，不在源码中写入账号或设备信息。

Platform is `4` for Mac, `1` for Windows/Linux. The stock viewer alone reads the
account credentials. Config files are private (0600, in a 0700 directory); generated
desktop entries contain a hashed configuration key, not account credentials.

## 可选设置 / Options

- 接管已连接的设备：默认启用，使用原程序已有的 `force_join=true`，可接管另一台
  电脑或手机的远控连接。关闭后，已占用的设备仍由原控制端保持连接。
- 字体文件：供 SDL 窗口和控制中心显示中文。自动选择可用的文泉驿 / Noto CJK 字体，
  也可指定覆盖所需字符的字体。不会改动系统 fontconfig。
- 固定 1920×1080 解码画布：针对某些自动尺寸解码停留在加载画面的情况；需要时启用。
  不强制更改远端屏幕分辨率。
- 按键长按连发：字母、数字、空格、普通符号、小键盘字符及 Backspace 使用本机重复速度连续输入；
  保留 Shift 大写/符号，松键或失焦即停止。字符快捷键不连发，Enter/Tab/Escape/功能键不额外连发。
- 常用 Ctrl 快捷键对应 Command：A/C/V/X/Z/F/S/O/N/P/W/L/T/R，保留 Shift，
  保留 Ctrl+Alt 和其他未列出的组合。不按远端应用识别终端：启用后 Ctrl+C 是 Command+C，
  要发送真实 Control+C，请在设置中关闭该选项。

Options select a CJK font, an optional fixed decoder canvas, held-key repeats,
PC-style editing shortcuts, and a manual image-transfer action. Mapped Ctrl chords
use Command, including in a terminal; disable the option to send literal Control+C.
Unlisted, Ctrl+Alt, and physical Win/Command chords keep their existing mapping.
Held-key repeat covers letters, digits, space, punctuation, printable keypad keys
and Backspace at the local repeat rate, including Shift. Release/focus loss stops
repeats. Character shortcuts and Enter/Tab/Escape/function keys do not gain repeats.
Existing `backspace_repeat` settings also control typing repeats for compatibility.
Taking over an occupied device is enabled by default using the viewer's existing
`force_join=true` argument. Disable it to keep another computer/phone's session connected.

Mac 入口默认核对每个按键的左右 Shift 状态，补齐输入法或切换焦点时漏掉的按下/松开，
使 `Shift+2`、大写字母等组合按正确顺序传输。`@` 对应 Mac 的 ABC / US 键盘布局；
其他远端布局仍按物理键解释。该恢复不依赖“本机快捷键习惯”或“Backspace 连续删除”开关。
独立窗口还单独设置 `XMODIFIERS=@im=none`，隔离本地 XIM；中文输入继续使用远端输入法。

Mac entries reconcile left/right Shift snapshots before each key, recovering a
missing press or release after an IME/focus change. `Shift+2` yields `@` with the
remote ABC/US layout; other layouts retain their physical-key behavior. Recovery
does not require the optional PC shortcuts or held-Backspace switches. Each independent
window sets `XMODIFIERS=@im=none` to bypass local XIM; use the remote IME for Chinese.

**按键适配仅支持 Debian 包版本 0.2.45，且只对已配置的 Mac 入口加载。**
其他版本保留独立窗口功能，按键适配不加载。原程序二进制和全局键盘设置不被修改。
v0.2.45 已有的本地输入法吞键修复是上游改动（PR #150），本组件不重复实现。

**The SDL2 input adapter is gated to Debian package 0.2.45 and configured
Mac entries.** Other package versions run independent sessions without the adapter.
The stock binary and global keyboard settings are not patched. Upstream PR #150's
IME fix is already included in v0.2.45 and is not reimplemented here.

## 图片流程和限制 / Image flow and limits

复制 PNG 截图后，在 Mac 远控窗口按 Ctrl+V（或 Command/Win+V）。这是 viewer 内置
功能，无须开启“图片传送”。确认所有图片数据块后才发送粘贴键，不按 Enter，不发送聊天。

Copy a PNG screenshot, then press Ctrl+V (or Command/Win+V) in the Mac session.
The viewer transfers the image over the native clipboard channel before sending the
paste chord. No separate image switch, remote terminal, file tools or reconnect is needed.

仅 Linux→Mac PNG，最大 32 MiB；文件、图片列表和反向图片同步不在本次范围内。
X11/XWayland 原生读图支持大图 INCR；SDL2 原生 Wayland 使用按需 `wl-paste`，
尚未完成各桌面的实机验证。窗口失焦、读取失败、超限或确认超时会取消粘贴。
旧 `uuremote-image-clipboard` 手动脚本保留用于兼容，已移出主窗口和设备菜单。
完整说明见 [native-image-clipboard.md](../../docs/native-image-clipboard.md)。

Linux→Mac PNG only, up to 32 MiB. Native Wayland's on-demand SDL2 fallback still
needs compositor-specific acceptance. The legacy manual helper remains callable,
but the launcher and device menu no longer expose it. See the linked protocol notes.

## 验证 / Validation

```sh
python3 -m unittest discover -s contrib/independent-viewer -p 'test_*.py' -v
cc -Wall -Wextra -Werror -shared -fPIC contrib/independent-viewer/sdl-input-compat.c -ldl -o /tmp/uu-input-test.so
SDL_VIDEODRIVER=dummy UU_VIEWER_BACKSPACE_REPEAT=1 UU_VIEWER_PC_SHORTCUTS=1 UU_VIEWER_SHIFT_SYNC=1 \
  LD_PRELOAD=/tmp/uu-input-test.so python3 contrib/independent-viewer/check-key-repeat.py
```

测试使用虚构设备和 SDL dummy 窗口，不连接账号、不读取真实剪贴板、不发送真实按键。
当前实机验证范围：GNOME Wayland/XWayland 控制官方 Mac，独立窗口关闭/重开、
中文字体、固定解码画布、图片写入及重连；用户确认修复后 `@` 和大写字母输入正常。
按键状态转换另有 SDL 队列检查。

Tests use fixture devices and SDL's dummy video backend, never a real clipboard
or keyboard. Live acceptance covered GNOME Wayland/XWayland to an official Mac,
independent window lifecycle, CJK font/canvas compatibility, and image pasteboard
write/reconnect. The user confirmed that `@` and uppercase letters work after the
fix was installed. Shortcut/modifier sequences are covered by real SDL queue tests.
Shift recovery covers both sides, `Shift+2`, A–Z, missing press/release, and focus
changes (113 scenarios). Takeover/target routing is checked with fixture devices;
an end-to-end two-controller takeover test remains pending.
