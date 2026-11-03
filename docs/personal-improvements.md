# 个人版改善清单 / Personal development record

基线：上游 `VallierDev/uu-remote-linux` v0.2.45，commit `c3959c0`。
这是个人使用的二次开发仓库，保留上游历史、署名和许可证。
运行时配置、设备信息、登录凭据、实际截图及诊断日志不进入源码提交。

Base: upstream v0.2.45 (`c3959c0`). This personal derivative preserves upstream
history, attribution and licensing. Runtime configuration, device/account data,
real screenshots and diagnostic logs are excluded from source commits.

| 改善 / Improvement | 实现 / Implementation | 验证和边界 / Acceptance and limits |
|---|---|---|
| 界面整理 / UI polish | 独立会话卡片、统一系统图标、分组开关和字体选择；跟随保存的语言及主题 | 中英、浅色/深色界面检查；以虚构设备生成预览 |
| 独立远控窗口 / Independent sessions | 主窗口设备页启动独立用户服务；同一设备重复打开复用会话 | 关闭主窗口后继续推流；注销账号明确停止独立会话 |
| 接管设备 / Takeover | 独立入口默认传入已有的 force_join=true；会话设置可关闭 | 虚构的多个设备分别路由到正确 ID；不改协议、不自动关闭其他本地设备窗口 |
| Dock 图标 / Desktop identity | 每设备生成 `.desktop`，设置 SDL X11/Wayland app id；恢复最小化窗口 | X11/XWayland 复用已有 PID 已验证；原生 Wayland 激活仍待验证 |
| 中文字体 / CJK font | 按设备选择支持 CJK 的字体，通过 `UU_VIEWER_FONT` 传入 | 不更改全局 fontconfig；覆盖范围取决于所选字体 |
| 画面兼容 / Decoder compatibility | 可选 `UU_VIEWER_SIZE=1920x1080` 固定解码画布 | 针对自动尺寸黑屏/停留加载；实机恢复出帧、解码错误为 0 |
| 按键长按连发 / Held-key repeat | 字母、数字、空格、符号、小键盘和 Backspace 的 SDL2 重复事件转为平衡的 release/press | 保留 Shift；快捷键不额外连发，松键/失焦停止；不记录按键 |
| 多键组合 / Modifier combinations | 原生 SDL 路径合入上游 #165 的左右 Shift/Ctrl/Alt/Win 状态恢复；独立进程隔离本地 XIM | 4 项原生回归测试；更新程序和旧入口后用户确认 Mac 锁屏输入恢复；@ 取决于远端 ABC/US 布局 |
| 独立入口更新 / Entry upgrades | 新版主窗口明确传入的 viewer 路径覆盖旧配置；直接打开桌面入口仍使用已保存路径 | 配置测试覆盖替换路径并保留偏好；避免主窗口已更新而独立连接仍调用旧系统程序 |
| PC 风格快捷键 / PC-style editing | 可选将常用 Ctrl 编辑组合转换为 Command；保留 Shift、Ctrl+Alt 和未列出的组合 | 62 组 SDL 队列检查；不区分远端应用，终端真实 Ctrl+C 需关闭该选项 |
| 原生 PNG 粘贴 / Native PNG paste | Ctrl+V 按需读取本地图片，经 TEXT_DATA_CHANNEL 格式协商与分块传输，确认后发送 Command+V | 生成图经官方 Mac 验证 SHA-256 一致且可解码；无终端/文件工具、临时文件或重连；失焦/失败不粘贴 |
| 新旧入口区分 / Local launcher | 独立桌面 ID，隐藏旧菜单项；用户命令指向本地构建，显示名称保留原名和“非官方”标记 | 保留正式版程序，避免应用菜单继续命中旧启动入口 |

字体与画布设置是兼容开关，不强制所有机器启用；按键适配只支持已配置的 Mac
和 Debian 包 0.2.45。原始 `/usr/bin` 程序不被二进制修改。
安装和具体使用方法见 [独立会话组件](../contrib/independent-viewer/README.md)。

Fonts/canvas are optional compatibility settings. Input adaptation is scoped to
configured Mac entries and Debian package 0.2.45; it does not binary-patch the stock
viewer. See the companion guide for installation and use.

上游 v0.2.45 的本地 IME 吞键修复（PR #150）作为基线保留，不重复列为个人新增实现。
图片功能已改为内置的 Linux→Mac PNG 粘贴。旧手动辅助脚本保留兼容调用，
主窗口、设备菜单和设置不再依赖它。范围与协议验证见[原生图片粘贴](native-image-clipboard.md)。

PNG paste is built into the viewer's native clipboard path. The legacy manual helper
remains callable for compatibility but is no longer exposed by the launcher/settings.
See [native image paste](native-image-clipboard.md) for scope and validation.

验证包括：控制端 Rust 测试、GTK 中英/明暗主题界面检查、Python 配置/权限测试、
真实 SDL dummy 队列的重复键与修饰键检查。实机检查不向源码或日志记录输入与剪贴板内容。

Validation covers viewer Rust tests, GTK appearance/settings checks, Python
configuration/permission checks, and real SDL dummy-queue input sequences.
Live checks never record keystrokes or clipboard contents in source or logs.
