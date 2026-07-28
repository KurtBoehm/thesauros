#!/usr/bin/env python3
"""Enforces the library's structural invariants.

Umbrella includes
  An "umbrella" is a header `P.hpp` next to a directory `P/`: it exists only to re-export
  the headers below it. Including one from ordinary library code drags in a whole
  sub-library, inflates compile times, and hides the real dependency edges that the
  module layering is meant to keep visible. An umbrella `P.hpp` may only include headers
  at or below `P/`; any other header may not include an umbrella at all.

{fmt} confinement
  `{fmt}` is reachable only from the `format` and `test` modules, so that everything below
  them stays free of it and can be built without the dependency. Code outside those modules
  builds messages with `thes::cat` from `charconv/concat.hpp` instead.
"""

import sys
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent.parent / "include" / "thesauros"
INCLUDE = re.compile(r'^\s*#\s*include\s+"thesauros/([^"]+)"', re.MULTILINE)

# The test harness deliberately wants every formatter in scope: it probes
# `fmt::formattable` to decide how to render values in test names and failure
# messages, so a narrower include would silently degrade its output rather than
# fail to compile. `test` is the topmost module and depends on nothing below it,
# so the umbrella creates no cycle.
EXEMPT = {"test/assert.hpp", "test/equality.hpp", "test/test.hpp"}

# Modules allowed to reach {fmt}. `format` wraps it; `test` renders values with it.
FMT_MODULES = ("format/", "test/")
FMT_USE = re.compile(r"fmt::|#\s*include\s+[\"<](?:fmt/|thesauros/format/fmtlib\.hpp)")


def is_umbrella(rel: str) -> bool:
    return (ROOT / rel[: -len(".hpp")]).is_dir()


def check_umbrellas(headers: list[tuple[Path, str, str]]) -> list[str]:
    problems: list[str] = []
    for _path, rel, text in headers:
        if rel in EXEMPT:
            continue
        own_dir = rel[: -len(".hpp")] + "/" if is_umbrella(rel) else None
        for target in INCLUDE.findall(text):
            if not is_umbrella(target):
                continue
            if own_dir is not None and target.startswith(own_dir):
                continue
            problems.append(f"{rel}: umbrella include of thesauros/{target}")
    return problems


def check_fmt(headers: list[tuple[Path, str, str]]) -> list[str]:
    problems: list[str] = []
    for _path, rel, text in headers:
        if rel.startswith(FMT_MODULES):
            continue
        for number, line in enumerate(text.splitlines(), start=1):
            if FMT_USE.search(line):
                problems.append(
                    f"{rel}:{number}: {{fmt}} used outside format/ and test/"
                )
    return problems


def main() -> int:
    headers = [
        (p, p.relative_to(ROOT).as_posix(), p.read_text())
        for p in sorted(ROOT.rglob("*.hpp"))
    ]
    problems = check_umbrellas(headers) + check_fmt(headers)

    for problem in problems:
        print(problem, file=sys.stderr)
    if problems:
        print(f"\n{len(problems)} structural violation(s).", file=sys.stderr)
        return 1
    print(
        f"{len(headers)} headers: no umbrella includes, {{fmt}} confined to format/ and test/."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
