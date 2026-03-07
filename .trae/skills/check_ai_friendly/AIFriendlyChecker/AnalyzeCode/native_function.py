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


def SaveAnalysisReport(analysis_json: str) -> str:
    repo_root = _find_repo_root(Path(__file__).parent)
    reports_dir = repo_root / "codeAudit"
    reports_dir.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S_%f")
    report_path = reports_dir / f"report_{timestamp}.json"

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

    report_path.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return str(report_path)
