from pathlib import Path
from typing import Callable


proj_path = Path(__file__).parent
inc_path = proj_path.joinpath("include/thesauros")


def recurse(path: Path, fun: Callable[[Path], bool]):
    for p in sorted(path.iterdir()):
        if fun(p):
            recurse(p, fun)


def header_guard_name(path: Path) -> str:
    name = str(path.relative_to(proj_path))
    for c in ("-", ".", "/"):
        name = name.replace(c, "_")
    return name.upper()


def fix_headers(base: Path):
    def inner(path: Path) -> bool:
        if path.is_dir():
            header = path.with_name(f"{path.name}.hpp")
            children = [
                str(p.relative_to(path.parent))
                for p in sorted(path.iterdir())
                if p.suffix == ".hpp"
            ]
            guard = header_guard_name(header)

            includes = [f'#include "{s}"' for s in children]
            lines = [
                f"#ifndef {guard}",
                f"#define {guard}",
                "",
                *includes,
                "",
                f"#endif // {guard}",
            ]

            with open(header, "w") as f:
                print("\n".join(lines), file=f)

        if path.is_file() and path.suffix == ".hpp":
            with open(path, "r") as f:
                contents = f.read().splitlines()

            assert contents[0].startswith("#ifndef")
            assert contents[1].startswith("#define")
            assert contents[-1].startswith("#endif")

            guard = header_guard_name(path)
            contents[0] = f"#ifndef {guard}"
            contents[1] = f"#define {guard}"
            contents[-1] = f"#endif // {guard}\n"

            with open(path, "w") as f:
                f.write("\n".join(contents))

        return path.is_dir()

    recurse(base, inner)


fix_headers(inc_path)
