from __future__ import annotations

import json
import re
from collections import deque
from fnmatch import fnmatch
from pathlib import Path
from typing import Any, Iterable


def _find_repo_root(start: Path) -> Path:
    current = start.resolve()
    for parent in [current, *current.parents]:
        if (parent / ".trae").exists():
            return parent
    return start.resolve()


def _split_paths(value: str) -> list[str]:
    if value is None:
        return []
    raw = str(value)
    parts = []
    for line in raw.replace(",", "\n").splitlines():
        p = line.strip()
        if p:
            parts.append(p)
    return parts


def _posix_rel(path: Path, root: Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except Exception:
        return path.as_posix()


def _estimate_tokens(text: str) -> int:
    if not text:
        return 0
    return max(1, int((len(text) + 3) / 4))


def _file_metrics(text: str) -> dict[str, Any]:
    if not text:
        return {"lines": 0, "max_line_length": 0, "avg_line_length": 0, "indent_max": 0, "estimated_tokens": 0}
    lines = text.splitlines()
    if not lines:
        return {"lines": 0, "max_line_length": 0, "avg_line_length": 0, "indent_max": 0, "estimated_tokens": 0}
    lengths = [len(l) for l in lines]
    indent_max = 0
    for l in lines:
        if not l.strip():
            continue
        indent = len(l) - len(l.lstrip(" \t"))
        indent_max = max(indent_max, indent)
    return {
        "lines": len(lines),
        "max_line_length": max(lengths),
        "avg_line_length": int(sum(lengths) / len(lengths)),
        "indent_max": indent_max,
        "estimated_tokens": _estimate_tokens(text),
    }


def _is_probably_binary(sample: bytes) -> bool:
    if b"\x00" in sample:
        return True
    non_printable = 0
    for b in sample:
        if b in (9, 10, 13):
            continue
        if 32 <= b <= 126:
            continue
        non_printable += 1
    return len(sample) > 0 and (non_printable / len(sample)) > 0.25


def _read_excerpt(file_path: Path, max_file_bytes: int) -> str:
    data = file_path.read_bytes()
    head = data[: min(len(data), 4096)]
    if _is_probably_binary(head):
        return ""
    text = data.decode("utf-8", errors="replace")
    if len(text.encode("utf-8", errors="replace")) <= max_file_bytes:
        return text

    lines = text.splitlines(keepends=True)
    head_lines = lines[:120]
    tail_lines = lines[-80:] if len(lines) > 200 else []
    excerpt = "".join(head_lines)
    if tail_lines:
        excerpt += "\n...\n"
        excerpt += "".join(tail_lines)

    encoded = excerpt.encode("utf-8", errors="replace")
    if len(encoded) <= max_file_bytes:
        return excerpt
    return encoded[:max_file_bytes].decode("utf-8", errors="ignore")


_CPP_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.MULTILINE)
_JS_IMPORT_RE = re.compile(r'^\s*import\s+(?:.+?\s+from\s+)?[\'"]([^\'"]+)[\'"]\s*;?', re.MULTILINE)
_JS_REQUIRE_RE = re.compile(r'require\(\s*[\'"]([^\'"]+)[\'"]\s*\)')
_HTML_REF_RE = re.compile(r'(?i)\b(?:src|href)\s*=\s*[\'"]([^\'"]+)[\'"]')


def _discover_deps(file_path: Path, excerpt: str) -> list[Path]:
    deps: list[Path] = []
    suffix = file_path.suffix.lower()

    def add_rel(p: str) -> None:
        if not p or p.startswith(("http://", "https://", "//")):
            return
        if p.startswith(("/", "\\")):
            return
        candidate = (file_path.parent / p).resolve()
        deps.append(candidate)

    if suffix in {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp"}:
        for m in _CPP_INCLUDE_RE.finditer(excerpt):
            add_rel(m.group(1))
    elif suffix in {".js", ".jsx", ".ts", ".tsx"}:
        for m in _JS_IMPORT_RE.finditer(excerpt):
            add_rel(m.group(1))
        for m in _JS_REQUIRE_RE.finditer(excerpt):
            add_rel(m.group(1))
    elif suffix in {".html", ".htm"}:
        for m in _HTML_REF_RE.finditer(excerpt):
            add_rel(m.group(1))

    normalized: list[Path] = []
    for p in deps:
        if p.suffix == "" and (suffix in {".js", ".jsx", ".ts", ".tsx"}):
            for ext in [".ts", ".tsx", ".js", ".jsx"]:
                candidate = p.with_suffix(ext)
                normalized.append(candidate)
            normalized.append(p / "index.ts")
            normalized.append(p / "index.tsx")
            normalized.append(p / "index.js")
            normalized.append(p / "index.jsx")
        else:
            normalized.append(p)

    uniq: list[Path] = []
    seen: set[str] = set()
    for p in normalized:
        key = str(p)
        if key in seen:
            continue
        seen.add(key)
        uniq.append(p)
    return uniq


def _matches_any(path_posix: str, patterns: Iterable[str]) -> bool:
    for pat in patterns:
        if fnmatch(path_posix, pat):
            return True
    return False


def BuildAccessedSnapshot(
    file_paths: str,
    task_description: str = "",
    max_hops: int = 1,
    max_files: int = 60,
    max_file_bytes: int = 12000,
    max_total_bytes: int = 180000,
) -> str:
    repo_root = _find_repo_root(Path(__file__).parent)
    exclude = [
        "**/.git/**",
        "**/.svn/**",
        "**/.hg/**",
        "**/.DS_Store",
        "**/node_modules/**",
        "**/dist/**",
        "**/build/**",
        "**/cmake-build-*/**",
        "**/.idea/**",
        "**/.vscode/**",
        "**/.trae/**",
        "**/codeAudit/**",
        "**/server/third_party/**",
    ]

    seeds = _split_paths(file_paths)
    seed_paths: list[Path] = []
    missing: list[str] = []
    for p in seeds:
        path = Path(p)
        if not path.is_absolute():
            path = (repo_root / path).resolve()
        if not path.exists() or not path.is_file():
            missing.append(p)
            continue
        seed_paths.append(path)

    visited: set[Path] = set()
    queue: deque[tuple[Path, int]] = deque()
    for s in seed_paths:
        queue.append((s, 0))

    file_items: list[dict[str, Any]] = []
    total_bytes = 0

    while queue and len(file_items) < int(max_files) and total_bytes < int(max_total_bytes):
        current, hop = queue.popleft()
        if current in visited:
            continue
        visited.add(current)

        rel_posix = _posix_rel(current, repo_root)
        if _matches_any(rel_posix, exclude):
            continue

        try:
            stat = current.stat()
            size = int(stat.st_size)
        except OSError:
            size = None

        try:
            excerpt = _read_excerpt(current, int(max_file_bytes))
        except OSError:
            excerpt = ""

        excerpt_bytes = len(excerpt.encode("utf-8", errors="replace"))
        if excerpt and (total_bytes + excerpt_bytes) > int(max_total_bytes):
            remaining = int(max_total_bytes) - total_bytes
            if remaining > 0:
                excerpt = excerpt.encode("utf-8", errors="replace")[:remaining].decode("utf-8", errors="ignore")
                excerpt_bytes = len(excerpt.encode("utf-8", errors="replace"))
            else:
                excerpt = ""
                excerpt_bytes = 0

        metrics = _file_metrics(excerpt)
        total_bytes += excerpt_bytes
        file_items.append(
            {
                "path": rel_posix,
                "size": size,
                "excerpt": excerpt,
                "metrics": metrics,
            }
        )

        if int(max_hops) > 0 and hop < int(max_hops) and excerpt:
            for dep in _discover_deps(current, excerpt):
                if dep.exists() and dep.is_file():
                    queue.append((dep, hop + 1))

    estimated_input_tokens = sum(int(f.get("metrics", {}).get("estimated_tokens", 0)) for f in file_items)
    snapshot = {
        "repo_root": str(repo_root),
        "scope": "accessed_files",
        "task_description": str(task_description or ""),
        "stats": {
            "seed_count": len(seeds),
            "missing_seeds": missing,
            "snapshot_files": len(file_items),
            "snapshot_total_bytes": total_bytes,
            "estimated_input_tokens": estimated_input_tokens,
            "max_hops": int(max_hops),
            "max_files": int(max_files),
            "max_file_bytes": int(max_file_bytes),
            "max_total_bytes": int(max_total_bytes),
        },
        "tree": sorted(f["path"] for f in file_items),
        "files": file_items,
    }
    return json.dumps(snapshot, ensure_ascii=False, indent=2)
