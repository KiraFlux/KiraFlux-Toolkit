import sys
from pathlib import Path
from typing import Sequence

platformio_ini = """
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_extra_dirs = ../..

; remove gnu++11, add gnu++17 (with GCC ext)
build_unflags = -std=gnu++11
build_flags = -std=gnu++17
monitor_speed = 115200
"""

def _make_main_cpp(example_name: str) -> str:
    return f"// KiraFlux-Toolkit Demo '{example_name}'" + """
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    // TODO: implement
}

void loop() {
    // TODO: implement
}
"""

repo_dir = Path(".")

def _start(args: Sequence[str]) -> int:
    if len(args) != 2:
        print("invalid args")
        return 1

    example_name = args[1]

    example_path = (repo_dir / "examples" / example_name).resolve()

    if example_path.exists():
        print(f"{example_path=} exists")
        return 1

    print(f"{example_path=}")

    example_path.mkdir()
    (example_path / "platformio.ini").write_text(platformio_ini)
    
    src_path = example_path / "src"
    src_path.mkdir()
    (src_path / "main.cpp").write_text(_make_main_cpp(example_name))

    return 0

exit(_start(sys.argv))
