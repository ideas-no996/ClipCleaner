# ClipCleaner

ClipCleaner 是一个 Windows 剪切板文本清洗小工具。当前主版本是 C++ Win32 原生实现：运行后常驻系统托盘，按默认快捷键 `Ctrl+Alt+V` 后，会读取当前剪切板 Unicode 文本，按规则清洗，再写回剪切板。

程序只在本机处理剪切板内容，不联网，不上传任何内容，不写注册表。

## 功能

- 合并英文 PDF 复制产生的错误换行
- 删除中文之间多余空格
- 将多个空格合并为一个
- 删除行尾多余空格
- 可选：去除 Markdown 链接，只保留标题文本
- Win32 API 原生实现，无 GUI 主窗口
- `RegisterHotKey` 全局快捷键
- `OpenClipboard` / `GetClipboardData` / `SetClipboardData` 读写剪切板
- `Shell_NotifyIcon` 系统托盘
- MSVC 静态运行时，单个 exe，无外部运行时依赖

## 运行

需要 Windows 10/11。运行 `ClipCleaner.exe` 后托盘会出现 ClipCleaner 图标。复制文本后按 `Ctrl+Alt+V`，剪切板里的文本会被清洗。也可以右键托盘图标，点击 `Clean clipboard now`。

### 规则说明

| 规则 | 说明 |
| --- | --- |
| `merge_english_pdf_line_breaks` | 将英文段落中被 PDF 复制打断的换行合并为空格，也会处理英文断词换行 |
| `remove_spaces_between_chinese` | 删除中文字符之间的空格，例如 `你 好` 变成 `你好` |
| `collapse_multiple_spaces` | 将连续多个半角空格或 Tab 合并成一个空格 |
| `trim_trailing_spaces` | 删除每一行末尾的空格或 Tab |
| `strip_markdown_links` | 将 `[标题](https://example.com)` 变成 `标题`；当前可通过托盘菜单临时开关 |

## 使用示例

输入：

```text
This is a para-
graph copied from a
PDF file.

中 文 之 间 有 空 格
hello     world
[OpenAI](https://openai.com)
```

如果启用全部规则，输出：

```text
This is a paragraph copied from a PDF file.

中文之间有空格
hello world
OpenAI
```

## 构建 C++ 原生版本

需要 CMake + MSVC。请先在当前终端中确保 `cmake` 和 MSVC 编译环境可用。

项目提供了 `build-native.ps1`，会输出 `Release\ClipCleaner.exe`。

```powershell
cd <repo>
.\build-native.ps1
```

构建结果：`Release\ClipCleaner.exe`。

## 隐私

ClipCleaner 只读取并写回 Windows 本地剪切板，不包含任何网络请求逻辑，也不会上传剪切板内容。
