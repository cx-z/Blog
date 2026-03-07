from __future__ import annotations

import json
import os
from fnmatch import fnmatch
from pathlib import Path
from typing import Any, Iterable


def _find_repo_root(start: Path) -> Path:
    current = start.resolve()
    for parent in [current, *current.parents]:
        if (parent / ".trae").exists():
            return parent
    return start.resolve()


def _split_patterns(value: str) -> list[str]:
    if value is None:
        return []
    items = [p.strip() for p in str(value).split(",")]
    return [p for p in items if p]


def _path_to_posix_relative(path: Path, root: Path) -> str:
    try:
        rel = path.resolve().relative_to(root.resolve())
        return rel.as_posix()
    except Exception:
        return path.as_posix()


def _matches_any(path_posix: str, patterns: Iterable[str]) -> bool:
    for pat in patterns:
        if fnmatch(path_posix, pat):
            return True
    return False


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


def BuildFolderSnapshot(
    folder_path: str,
    include_patterns: str = "",
    exclude_patterns: str = "",
    max_files: int = 200,
    max_file_bytes: int = 12000,
    max_total_bytes: int = 220000,
) -> str:
    repo_root = _find_repo_root(Path(__file__).parent)
    folder = Path(folder_path) if folder_path else repo_root
    if not folder.is_absolute():
        folder = (repo_root / folder).resolve()

    include = _split_patterns(include_patterns)
    if not include:
        include = [
            "**/*.c",
            "**/*.cc",
            "**/*.cpp",
            "**/*.cxx",
            "**/*.h",
            "**/*.hpp",
            "**/*.hh",
            "**/*.go",
            "**/*.java",
            "**/*.kt",
            "**/*.py",
            "**/*.rs",
            "**/*.js",
            "**/*.jsx",
            "**/*.ts",
            "**/*.tsx",
            "**/*.json",
            "**/*.yml",
            "**/*.yaml",
            "**/*.toml",
            "**/*.ini",
            "**/*.sql",
            "**/*.md",
            "**/CMakeLists.txt",
            "**/Makefile",
            "**/*.sh"
        ]

    exclude = _split_patterns(exclude_patterns)
    if not exclude:
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
            "**/server/third_party/**"
        ]

    tree_paths: list[str] = []
    file_items: list[dict[str, Any]] = []

    total_bytes = 0
    scanned_files = 0
    included_files = 0

    if not folder.exists() or not folder.is_dir():
        return json.dumps(
            {
                "error": "folder_not_found",
                "folder_path": folder_path,
                "resolved_folder_path": str(folder),
            },
            ensure_ascii=False,
            indent=2,
        )

    for root, dirs, files in os.walk(folder):
        root_path = Path(root)
        rel_root = _path_to_posix_relative(root_path, folder)
        if rel_root != ".":
            tree_paths.append(rel_root + "/")

        pruned_dirs: list[str] = []
        for d in dirs:
            d_path = root_path / d
            d_posix = _path_to_posix_relative(d_path, folder)
            if _matches_any(d_posix + "/", exclude):
                continue
            pruned_dirs.append(d)
        dirs[:] = pruned_dirs

        for f in files:
            scanned_files += 1
            f_path = root_path / f
            f_posix = _path_to_posix_relative(f_path, folder)
            if _matches_any(f_posix, exclude):
                continue
            if include and not _matches_any(f_posix, include):
                continue

            included_files += 1
            tree_paths.append(f_posix)

            if len(file_items) >= int(max_files):
                continue
            if total_bytes >= int(max_total_bytes):
                continue

            try:
                stat = f_path.stat()
                size = int(stat.st_size)
            except OSError:
                size = None

            excerpt = ""
            try:
                excerpt = _read_excerpt(f_path, int(max_file_bytes))
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

            total_bytes += excerpt_bytes
            file_items.append(
                {
                    "path": f_posix,
                    "size": size,
                    "excerpt": excerpt,
                }
            )

    snapshot = {
        "repo_root": str(repo_root),
        "folder_path": folder_path,
        "resolved_folder_path": str(folder),
        "stats": {
            "scanned_files": scanned_files,
            "included_files": included_files,
            "snapshot_files": len(file_items),
            "snapshot_total_bytes": total_bytes,
            "max_files": int(max_files),
            "max_file_bytes": int(max_file_bytes),
            "max_total_bytes": int(max_total_bytes),
            "include_patterns": include,
            "exclude_patterns": exclude,
        },
        "tree": sorted(set(tree_paths)),
        "files": file_items,
    }
    return json.dumps(snapshot, ensure_ascii=False, indent=2)
