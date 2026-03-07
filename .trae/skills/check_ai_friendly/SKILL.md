---
name: "check_ai_friendly"
description: "审查代码的 AI 友好度并输出结构化报告。用户要求检查 AI 易读性/代码质量或需要辅助理解复杂代码时调用。"
---

# check_ai_friendly

## 作用

本 Skill 用于评估代码是否利于 LLM 理解（可读性、上下文完整性、Token 效率、命名与注释），并以纯 JSON 结构化输出问题与建议。

## 何时调用

- 用户明确提出：用 check_ai_friendly 分析这个文件 / 检查 AI 易读性 / 检查代码质量
- Agent 在阅读或修改代码前：发现代码缺少上下文、命名混乱、重复与冗长导致理解成本高

## 如何使用

默认行为（推荐）：

1) 调用 `AIFriendlyChecker.AnalyzeCode` 获取分析 JSON（不要直接把这段长 JSON 原样输出给用户）
2) 立刻调用 `AIFriendlyChecker.SaveAnalysisReport` 把分析 JSON 保存到根目录 `codeAudit/`
3) 最终只向用户输出保存路径（可附带 score 与 issues 数量），例如：`{"report_path":"...","score":87}`

### 文件夹分析（架构 + 采样代码质量）

1) 调用 `AIFriendlyChecker.BuildFolderSnapshot` 采集目标文件夹快照（目录树 + 若干文件片段）
2) 调用 `AIFriendlyChecker.AnalyzeFolder` 对快照做架构与整体 AI 友好度分析
3) 调用 `AIFriendlyChecker.SaveAnalysisReport` 把 `AnalyzeFolder` 的 JSON 输出保存到根目录 `codeAudit/`

### LLM 分析函数

- Function: `AIFriendlyChecker.AnalyzeCode`
- 输入参数：
  - `input`: 需要分析的代码内容（推荐直接传完整文件内容或关键片段）
  - `file_path`: 可选，用于报告定位与引用
- 输出：纯 JSON（字段：`score`, `issues`, `suggestions`）

### 保存报告（Native Function）

- Function: `AIFriendlyChecker.SaveAnalysisReport`
- 输入：`AIFriendlyChecker.AnalyzeCode` 返回的 JSON 字符串
- 行为：保存到工程根目录 `codeAudit/report_{timestamp}.json`，并返回保存路径
- 返回：保存成功的文件路径

## 示例

1) 读取当前文件内容作为 `input`，并传入 `file_path`

2) 调用 `AIFriendlyChecker.AnalyzeCode` 得到 JSON

3) 将 JSON 传入 `AIFriendlyChecker.SaveAnalysisReport` 保存为本地报告
