from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


def _find_repo_root(start: Path) -> Path:
    current = start.resolve()
    for parent in [current, *current.parents]:
        if (parent / ".trae").exists():
            return parent
    return start.resolve()


def _generate_run_id() -> str:
    return datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S_%f")


def _safe_filename_component(value: str, max_len: int = 80) -> str:
    s = "" if value is None else str(value)
    s = s.strip()
    if not s:
        return "task"
    s = s.replace("\n", " ").replace("\r", " ").replace("\t", " ")
    for ch in ["/", "\\", ":", "*", "?", "\"", "<", ">", "|"]:
        s = s.replace(ch, "_")
    while "  " in s:
        s = s.replace("  ", " ")
    s = s.strip(" .")
    if not s:
        return "task"
    if len(s) > max_len:
        s = s[:max_len].rstrip()
    return s


def _extract_task_name(payload: Any) -> str:
    if isinstance(payload, dict):
        meta = payload.get("report_meta")
        if isinstance(meta, dict):
            td = meta.get("task_description")
            if isinstance(td, str) and td.strip():
                return td.strip()
            tp = meta.get("target_path")
            if isinstance(tp, str) and tp.strip():
                return tp.strip()
        subj = payload.get("subject")
        if isinstance(subj, str) and subj.strip():
            return subj.strip()
    return "task"


def _coerce_int(value: Any) -> int | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, float):
        return int(value)
    if isinstance(value, str):
        s = value.strip()
        if s.isdigit():
            return int(s)
    return None


def _get_estimated_input_tokens(payload: Any) -> int | None:
    if not isinstance(payload, dict):
        return None
    token_usage = payload.get("token_usage")
    if isinstance(token_usage, dict):
        estimated = token_usage.get("estimated")
        if isinstance(estimated, dict):
            v = _coerce_int(estimated.get("input_tokens"))
            if isinstance(v, int):
                return v

    summary = payload.get("summary")
    if isinstance(summary, dict):
        v = _coerce_int(summary.get("estimated_input_tokens"))
        if isinstance(v, int):
            return v

    stats = payload.get("stats")
    if isinstance(stats, dict):
        v = _coerce_int(stats.get("estimated_input_tokens"))
        if isinstance(v, int):
            return v

    token_usage_estimate = payload.get("token_usage_estimate")
    if isinstance(token_usage_estimate, dict):
        v = _coerce_int(token_usage_estimate.get("approx_total_tokens"))
        if isinstance(v, int):
            return v
        chars = _coerce_int(token_usage_estimate.get("approx_total_characters"))
        if isinstance(chars, int) and chars > 0:
            return max(1, int((chars + 3) / 4))

    return None


def _issue_weight(issue: Any) -> float:
    if not isinstance(issue, dict):
        return 1.0
    severity = str(issue.get("severity") or "").lower().strip()
    base = 1.5
    if severity in {"high", "critical"}:
        base = 3.0
    elif severity in {"medium", "med"}:
        base = 2.0
    elif severity in {"low"}:
        base = 1.0

    category = str(issue.get("category") or issue.get("type") or "").lower()
    if "security" in category or "auth" in category or "jwt" in category:
        base += 0.8
    if "architecture" in category or "design" in category:
        base += 0.4
    if "readability" in category or "naming" in category or "context" in category:
        base += 0.3

    text = " ".join(
        str(issue.get(k) or "")
        for k in ["title", "message", "symptom", "impact", "recommendation"]
    ).lower()
    if any(w in text for w in ["重复", "样板", "boilerplate", "duplicate", "duplication"]):
        base += 0.7
    if any(w in text for w in ["上下文", "implicit", "magic", "未定义", "undefined"]):
        base += 0.4
    if any(w in text for w in ["命名", "naming", "缩写", "abbrev"]):
        base += 0.2

    return max(0.5, base)


def _ensure_issue_token_impact(payload: Any) -> None:
    if not isinstance(payload, dict):
        return
    issues = payload.get("issues")
    if not isinstance(issues, list) or not issues:
        return

    input_tokens = _get_estimated_input_tokens(payload) or 0
    budget = 220
    if input_tokens > 0:
        budget = max(80, min(2000, int(input_tokens * 0.08)))

    weights = [_issue_weight(it) for it in issues]
    denom = sum(weights) or float(len(weights))

    for idx, issue in enumerate(issues):
        if not isinstance(issue, dict):
            continue
        token_impact = issue.get("token_impact")
        if isinstance(token_impact, dict) and (
            token_impact.get("estimated_extra_tokens") is not None
            or token_impact.get("actual_extra_tokens") is not None
        ):
            continue

        share = weights[idx] / denom if denom else 1.0 / max(1, len(weights))
        estimated_extra = max(1, int(round(budget * share)))
        issue["token_impact"] = {
            "actual_extra_tokens": None,
            "estimated_extra_tokens": estimated_extra,
            "basis": "基于 severity/category 与本次审计输入 token 预算的经验分摊估算"
        }


def SaveAnalysisReport(analysis_json: str, run_id: str = "") -> str:
    repo_root = _find_repo_root(Path(__file__).parent)
    base_dir = repo_root / "codeAudit"
    base_dir.mkdir(parents=True, exist_ok=True)

    payload: Any
    raw = "" if analysis_json is None else str(analysis_json)
    try:
        payload = json.loads(raw) if raw.strip() else {
            "score": 0,
            "issues": [{"line": None, "type": "missing_input", "message": "Empty analysis JSON input"}],
            "suggestions": ["请先运行 AnalyzeCode 得到分析 JSON，再调用 SaveAnalysisReport 保存。"]
        }
    except json.JSONDecodeError as e:
        payload = {
            "score": 0,
            "issues": [{"line": None, "type": "invalid_json", "message": f"Invalid JSON: {e}"}],
            "suggestions": ["请确保传入的是 skprompt.txt 生成的纯 JSON 字符串。"],
            "raw": raw
        }

    resolved_run_id = str(run_id or "").strip()
    if not resolved_run_id:
        meta = payload.get("report_meta") if isinstance(payload, dict) else None
        if isinstance(meta, dict):
            meta_run_id = str(meta.get("run_id") or "").strip()
            if meta_run_id:
                resolved_run_id = meta_run_id
    if not resolved_run_id:
        resolved_run_id = _generate_run_id()

    if isinstance(payload, dict):
        meta = payload.get("report_meta")
        if not isinstance(meta, dict):
            meta = {}
            payload["report_meta"] = meta
        meta["run_id"] = meta.get("run_id") or resolved_run_id
        meta["created_at"] = meta.get("created_at") or datetime.now(timezone.utc).isoformat()
        meta["repo_root"] = meta.get("repo_root") or str(repo_root)

        token_usage = payload.get("token_usage")
        if not isinstance(token_usage, dict):
            token_usage = {}
            payload["token_usage"] = token_usage
        token_usage.setdefault("actual", None)

        estimated = token_usage.get("estimated")
        if not isinstance(estimated, dict):
            estimated = {}
            token_usage["estimated"] = estimated

        input_tokens = estimated.get("input_tokens")
        if input_tokens is None:
            summary = payload.get("summary")
            if isinstance(summary, dict):
                summary_est = summary.get("estimated_input_tokens")
                if isinstance(summary_est, int):
                    input_tokens = summary_est
            stats = payload.get("stats")
            if input_tokens is None and isinstance(stats, dict):
                stats_est = stats.get("estimated_input_tokens")
                if isinstance(stats_est, int):
                    input_tokens = stats_est
            token_usage_estimate = payload.get("token_usage_estimate")
            if input_tokens is None and isinstance(token_usage_estimate, dict):
                approx_tokens = token_usage_estimate.get("approx_total_tokens")
                if isinstance(approx_tokens, int):
                    input_tokens = approx_tokens
                else:
                    approx_chars = token_usage_estimate.get("approx_total_characters")
                    if isinstance(approx_chars, int) and approx_chars > 0:
                        input_tokens = max(1, int((approx_chars + 3) / 4))

        output_tokens = estimated.get("output_tokens")
        if output_tokens is None:
            output_tokens = 0 if not raw else max(1, int((len(raw) + 3) / 4))

        total_tokens = estimated.get("total_tokens")
        if total_tokens is None:
            if isinstance(input_tokens, int):
                total_tokens = int(input_tokens) + int(output_tokens)
            else:
                total_tokens = int(output_tokens)

        estimated["input_tokens"] = input_tokens if isinstance(input_tokens, int) else None
        estimated["output_tokens"] = int(output_tokens)
        estimated["total_tokens"] = int(total_tokens)

    _ensure_issue_token_impact(payload)

    report_dir = base_dir / resolved_run_id
    report_dir.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S_%f")
    task_name = _safe_filename_component(_extract_task_name(payload))
    report_path = report_dir / f"{timestamp}_{task_name}.json"
    if report_path.exists():
        i = 2
        while True:
            candidate = report_dir / f"{timestamp}_{task_name}_{i}.json"
            if not candidate.exists():
                report_path = candidate
                break
            i += 1
    report_path.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return str(report_path)
