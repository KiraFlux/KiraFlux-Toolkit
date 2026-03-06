import sys
from pathlib import Path

sys.stdout = open("shaphot.txt", "wt", encoding='utf-8')

repo_dir = Path(".")

target_ext = (
    "h", "hpp", "cpp", "md", "ino"
)

target_dirs = (
    "examples", "src", "test"
)

for d in target_dirs:
    _dir = repo_dir / d
    for ext in target_ext:
        for file in _dir.rglob(f"*.{ext}"):
            try:
                print(f"{file.relative_to(repo_dir)}")
                print(f"```{ext}")
                print(file.read_text(encoding='utf-8', errors='ignore').rstrip())
                print("```\n")
            except:
                pass
