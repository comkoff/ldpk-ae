#!/usr/bin/env python3
import argparse
import re
from pathlib import Path

# Matches the configuration condition line inside a PropertyGroup block.
CONFIG_RE = re.compile(r"Release-Nuke(\d+)\.(\d+)\|x64")
TOOLSET_RE = re.compile(r"(<PlatformToolset>)(v\d+)(</PlatformToolset>)")
PROPERTY_BLOCK_RE = re.compile(
    r"(<PropertyGroup\b[^>]*Condition=\"[^\"]*Release-Nuke\d+\.\d+\|x64[^\"]*\"[^>]*Label=\"Configuration\"[^>]*>)(.*?)(</PropertyGroup>)",
    re.DOTALL,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Retarget vcxproj PlatformToolset by Nuke release version."
    )
    parser.add_argument(
        "root",
        nargs="?",
        default=".",
        help="Root directory to scan for .vcxproj files (default: current directory)",
    )
    parser.add_argument(
        "--legacy-max",
        default="15.2",
        help="Highest Nuke version that should use v141 (default: 15.2)",
    )
    parser.add_argument(
        "--legacy-toolset",
        default="v141",
        help="Toolset for versions <= --legacy-max (default: v141)",
    )
    parser.add_argument(
        "--modern-toolset",
        default="v143",
        help="Toolset for versions > --legacy-max (default: v143)",
    )
    parser.add_argument(
        "--write",
        action="store_true",
        help="Write changes to files (without this flag, runs as dry-run)",
    )
    parser.add_argument(
        "--include-v141-dirs",
        action="store_true",
        help="Also process project files inside directories named *_v141 (default: excluded)",
    )
    return parser.parse_args()


def parse_version(version: str) -> tuple[int, int]:
    parts = version.split(".")
    if len(parts) != 2:
        raise ValueError(f"Invalid version '{version}'. Expected format MAJOR.MINOR, e.g. 15.1")
    return int(parts[0]), int(parts[1])


def process_file(
    file_path: Path,
    legacy_max: tuple[int, int],
    legacy_toolset: str,
    modern_toolset: str,
) -> tuple[int, int, str]:
    text = file_path.read_text(encoding="utf-8")
    changes = 0
    touched_blocks = 0

    def repl(match: re.Match[str]) -> str:
        nonlocal changes, touched_blocks
        block_open = match.group(1)
        block_body = match.group(2)
        block_close = match.group(3)

        cfg_match = CONFIG_RE.search(block_open)
        if not cfg_match:
            return match.group(0)

        ver = (int(cfg_match.group(1)), int(cfg_match.group(2)))
        target_toolset = legacy_toolset if ver <= legacy_max else modern_toolset

        new_body, n = TOOLSET_RE.subn(rf"\1{target_toolset}\3", block_body, count=1)
        if n > 0 and new_body != block_body:
            changes += 1
            touched_blocks += 1
        return f"{block_open}{new_body}{block_close}"

    new_text = PROPERTY_BLOCK_RE.sub(repl, text)
    return changes, touched_blocks, new_text


def main() -> int:
    args = parse_args()
    root = Path(args.root).resolve()
    legacy_max = parse_version(args.legacy_max)

    vcxprojs = sorted(root.rglob("*.vcxproj"))
    if not args.include_v141_dirs:
        vcxprojs = [p for p in vcxprojs if not any("_v141" in part for part in p.parts)]
    if not vcxprojs:
        print(f"No .vcxproj files found under {root}")
        return 1

    total_changes = 0
    changed_files = 0

    for path in vcxprojs:
        changes, touched_blocks, new_text = process_file(
            path, legacy_max, args.legacy_toolset, args.modern_toolset
        )
        if changes > 0:
            changed_files += 1
            total_changes += touched_blocks
            print(f"{path}: {touched_blocks} block(s) updated")
            if args.write:
                path.write_text(new_text, encoding="utf-8")

    mode = "APPLY" if args.write else "DRY-RUN"
    print(
        f"[{mode}] files changed: {changed_files}, configuration blocks updated: {total_changes}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
