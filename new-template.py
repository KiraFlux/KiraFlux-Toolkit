import sys
from pathlib import Path
from typing import Sequence


REPO_DIR = Path(".").resolve()

PLATFORMIO_INI = """
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_extra_dirs = ../../..

; remove gnu++11, add gnu++17 (with GCC ext)
build_unflags = -std=gnu++11
build_flags = -std=gnu++17
monitor_speed = 115200
"""

TEST_CPP = """
#include <unity.h>

void test() {
    // TODO: implement
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test);
    return UNITY_END();
}
"""

MAIN_CPP = """
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    // TODO: implement
}

void loop() {
    // TODO: implement
}
"""

def _create_new_test(group_name: str, test_name: str) -> int:
    print(f"New test: {group_name=} {test_name=}")

    test_path = REPO_DIR / "test" / "unit" / group_name / f"test_{test_name}"

    if test_path.exists():
        print(f"{test_path=} exists")
        return 1
    
    test_path.mkdir(parents=True)

    (test_path / f"{test_name}.cpp").write_text(TEST_CPP)

    print(f"Created new test '{group_name}/{test_name}' ({test_path})")
    return 0

def _create_new_example(group_name: str, example_name: str) -> int:
    print(f"New example: {group_name=} {example_name=}")

    example_path = REPO_DIR / "examples" / group_name / example_name

    if example_path.exists():
        print(f"{example_path=} exists")
        return 1

    example_path.mkdir(parents=True)
    (example_path / "platformio.ini").write_text(PLATFORMIO_INI)
    
    src_path = example_path / "src"
    src_path.mkdir()
    (src_path / "main.cpp").write_text(f"// KiraFlux-Toolkit Demo '{example_name}'" + MAIN_CPP )

    print(f"Created new example '{example_name}' ({example_path})")
    return 0

def _start(args: Sequence[str]) -> int:
    args_got = len(args) 
    
    if args_got <= 1:
        print("invalid args")
        return 1

    target_name = args[1]

    ret = {
        "example": (2, _create_new_example),
        "test": (2, _create_new_test),
    }.get(target_name)

    if ret is None:
        print(f"unknown {target_name=}")
        return 1

    need_args, target = ret

    if args_got != (2 + need_args):
        print(f"invalid args: {need_args=} (got: {args_got})")
        return 1

    return target(*args[2:])

exit(_start(sys.argv))
