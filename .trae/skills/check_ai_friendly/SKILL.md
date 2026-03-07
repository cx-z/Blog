---
name: "check_ai_friendly"
description: "审查代码的 AI 友好度并输出纯 JSON 报告（强制包含 token 估算与热点归因）。用户要求检查 AI 易读性/代码质量或需要辅助理解复杂代码时调用。"
---

# check_ai_friendly

## 作用

本 Skill 用于评估代码是否利于 LLM 理解（可读性、上下文完整性、Token 效率、命名与注释），并以纯 JSON 结构化输出问题与建议。

## 输出硬性约束（必须满足）

本 Skill 的报告必须同时满足以下硬性约束；即使缺少足够信息，也必须用 `null` 或空数组占位，禁止省略字段：

1) **Token 消耗估算必须存在**

- 报告必须包含 `token_usage`（至少包含 estimated input/output/total 的结构；无法获得真实值时 `actual` 必须为 `null`）

2) **热点归因必须存在**

- 报告必须包含 `token_hotspots`（识别输入 token 的 top 热点文件；若无法估算各文件 tokens，仍需输出该字段，且相关数值置为 `null`）

3) **逐 issue 的 token 影响必须存在**

- `issues[]` 中每一条都必须包含 `token_impact`
  - `token_impact.actual_extra_tokens`: 无法获得真实值时必须为 `null`
  - `token_impact.estimated_extra_tokens`: 无法估算时必须为 `null`
  - `token_impact.basis`: 必须给出简短估算依据（无法估算时说明原因）

## 何时调用

- 用户明确提出：用 check_ai_friendly 分析这个文件 / 检查 AI 易读性 / 检查代码质量
- Agent 在阅读或修改代码前：发现代码缺少上下文、命名混乱、重复与冗长导致理解成本高

## 如何使用

默认行为（推荐，按需扫描）：

1) 收集本次任务中 Agent 实际访问过的文件列表（file_paths）
2) 调用 `AIFriendlyChecker.BuildAccessedSnapshot` 生成快照
3) 调用 `AIFriendlyChecker.AnalyzeFolder` 输出一个 JSON 总报告（必须包含 token 估算与热点归因，并对每条 issue 给出 token_impact）
4) 调用 `AIFriendlyChecker.SaveAnalysisReport` 保存为 `codeAudit/{run_id}/{timestamp}_{task}.json`
5) 最终只向用户输出保存路径与关键信息，例如：`{"report_path":".../codeAudit/<run_id>/<timestamp>_<task>.json","score":87}`

### 文件夹分析（架构 + 采样代码质量）

1) 调用 `AIFriendlyChecker.BuildFolderSnapshot` 采集目标文件夹快照（目录树 + 若干文件片段）
2) 调用 `AIFriendlyChecker.AnalyzeFolder` 对快照做架构与整体 AI 友好度分析
3) 调用 `AIFriendlyChecker.SaveAnalysisReport` 把 `AnalyzeFolder` 的 JSON 输出保存到 `codeAudit/{run_id}/{timestamp}_{task}.json`

### LLM 分析函数

- Function: `AIFriendlyChecker.AnalyzeCode`
- 输入参数：
  - `input`: 需要分析的代码内容（推荐直接传完整文件内容或关键片段）
  - `file_path`: 可选，用于报告定位与引用
- 输出：纯 JSON（强制包含 `token_usage`，且 `issues[]` 每条必须包含 `token_impact`）

### 按需扫描快照（Native Function）

- Function: `AIFriendlyChecker.BuildAccessedSnapshot`
- 输入：本次任务中 Agent 实际访问过的文件列表（逗号或换行分隔）
- 行为：只采集这些文件（并可做保守依赖扩展）生成快照 JSON

### 保存报告（Native Function）

- Function: `AIFriendlyChecker.SaveAnalysisReport`
- 输入：`AIFriendlyChecker.AnalyzeCode` 或 `AIFriendlyChecker.AnalyzeFolder` 返回的 JSON 字符串
- 行为：保存到工程根目录 `codeAudit/{run_id}/{timestamp}_{task}.json`，并返回保存路径
- 返回：保存成功的文件路径

## 示例

1) 读取当前文件内容作为 `input`，并传入 `file_path`

2) 调用 `AIFriendlyChecker.AnalyzeCode` 得到 JSON

3) 将 JSON 传入 `AIFriendlyChecker.SaveAnalysisReport` 保存为本地报告
