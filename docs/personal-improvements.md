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
| Shift 组合键 / Shift chords | 按 SDL 左右 Shift 快照修复遗漏的状态转换；独立进程隔离本地 XIM | 113 组队列检查；用户实机确认 @ 和大写正常；@ 取决于远端 ABC/US 布局 |
| PC 风格快捷键 / PC-style editing | 可选将常用 Ctrl 编辑组合转换为 Command；保留 Shift、Ctrl+Alt 和未列出的组合 | 62 组 SDL 队列检查；不区分远端应用，终端真实 Ctrl+C 需关闭该选项 |
| PNG 图片传送 / Manual PNG transfer | 主窗口“传送截图”和 Dock 操作复用文件/终端工具，写入 Mac pasteboard，再恢复桌面 | 实机验证用户复制的 PNG 上传与写入；先传后粘贴，不按 Enter、不发送聊天 |
| 新旧入口区分 / Local launcher | 独立桌面 ID，隐藏旧菜单项；用户命令指向本地构建，显示名称保留原名和“非官方”标记 | 保留正式版程序，避免应用菜单继续命中旧启动入口 |

字体与画布设置是兼容开关，不强制所有机器启用；按键适配只支持已配置的 Mac
和 Debian 包 0.2.45。原始 `/usr/bin` 程序不被二进制修改。
安装和具体使用方法见 [独立会话组件](../contrib/independent-viewer/README.md)。

Fonts/canvas are optional compatibility settings. Input adaptation is scoped to
configured Mac entries and Debian package 0.2.45; it does not binary-patch the stock
viewer. See the companion guide for installation and use.

上游 v0.2.45 的本地 IME 吞键修复（PR #150）作为基线保留，不重复列为个人新增实现。
图片功能是手动的 PNG 辅助流程，原生 UU 协议仍是文本剪贴板；工具关闭时的共享
leave 通知通过暂停/恢复所选独立桌面处理，不宣称无缝双向图片同步。

Upstream's v0.2.45 IME fix (PR #150) is retained as baseline work. The PNG helper
is a manual workflow; native clipboard messages remain text-only. The observed
shared leave notification is handled by pausing/restoring the selected independent
desktop, not by claiming seamless image synchronization.

验证包括：控制端 Rust 测试、GTK 中英/明暗主题界面检查、Python 配置/权限测试、
真实 SDL dummy 队列的重复键与修饰键检查。实机检查不向源码或日志记录输入与剪贴板内容。

Validation covers viewer Rust tests, GTK appearance/settings checks, Python
configuration/permission checks, and real SDL dummy-queue input sequences.
Live checks never record keystrokes or clipboard contents in source or logs.
