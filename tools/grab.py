import sys
from pathlib import Path

sys.stdout = open("out.txt", "wt", encoding='utf-8')

DIR = Path("../")



for file in DIR.rglob("*.hpp"):
    try:
        print(f"{file.relative_to(DIR)}")
        print("```cpp")
        print(file.read_text(encoding='utf-8', errors='ignore').rstrip())
        print("```\n")
    except:
        pass

for file in DIR.rglob("*.cpp"):
    try:
        print(f"{file.relative_to(DIR)}")
        print("```cpp")
        print(file.read_text(encoding='utf-8', errors='ignore').rstrip())
        print("```\n")
    except:
        pass

for file in DIR.rglob("*.md"):
    try:
        print(f"{file.relative_to(DIR)}")
        print("```cpp")
        print(file.read_text(encoding='utf-8', errors='ignore').rstrip())
        print("```\n")
    except:
        pass