#!/usr/bin/env python3
import os
import sys
import subprocess
import time

from dataclasses import dataclass
from pathlib import Path
from typing import Final, Sequence

repo_dir = Path(__file__).parent.resolve()
examples_dir = repo_dir / "examples"
examples_glob = "examples/*/*/platformio.ini"
failed_examples_list_file = repo_dir / "failed_examples.txt"
example_env = "esp32dev"

GREEN = '\033[92m'
RED = '\033[91m'
RESET = '\033[0m'

try:
    terminal_width = os.get_terminal_size().columns
except OSError:
    terminal_width = 80


def red(s: str) -> str:
    return RED + s + RESET


def green(s: str) -> str:
    return GREEN + s + RESET


@dataclass(kw_only=True)
class ExampleBuildResult:
    path: Path
    is_success: bool
    duration: float
    stdout: str
    stderr: str

    def name(self) -> str:
        return str(self.path.relative_to(examples_dir))

    def status(self) -> str:
        return (green("SUCCESS") if self.is_success else red("FAILED"))


class ExamplesBuildSummary:

    def __init__(self):
        self._results: Final = list[ExampleBuildResult]()

    def add(self, result: ExampleBuildResult) -> None:
        self._results.append(result)
    
    def all(self) -> Sequence[ExampleBuildResult]:
        return self._results

    def max_example_name(self) -> int:
        return max(
            len(r.name())
            for r in self._results
        )

    def total_time(self) -> float:
        return sum(
            r.duration
            for r in self._results
        )
    
    def failed_examples_paths(self) -> Sequence[Path]:
        return tuple(
            r.path
            for r in self._results
            if not r.is_success
        )


def find_all_example_dirs() -> Sequence[Path]:
    """Return list of Path objects for directories containing platformio.ini."""
    return sorted(p.parent for p in repo_dir.glob(examples_glob))


def read_failed_list() -> Sequence[Path]:
    """Read list of relative paths from failed file."""

    if not failed_examples_list_file.exists() or (failed_examples_list_file.stat().st_size == 0):
        return ()
    
    with open(failed_examples_list_file, "r") as f:
        lines = filter(
            (lambda l: bool(l)),
            map(
                (lambda l: l.strip()),
                f
            )
        )

        paths = filter(
            (lambda l: l.exists()),
            map(
                (lambda l: repo_dir / l), 
                lines
            )
        )

        return tuple(paths)


def write_failed_list(failed_paths: Sequence[Path]) -> None:
    """Write relative paths of failed examples."""

    failed_examples_list_file.write_text("\n".join(
        str(p.relative_to(repo_dir))
        for p in failed_paths
    ))


def build_example(example_dir: Path) -> ExampleBuildResult:
    """Build a single example for esp32dev, return (success, elapsed_seconds)."""
    
    start = time.perf_counter()
    
    proc = subprocess.run(
        ["pio", "run", "--silent", "-e", example_env, "-d", str(example_dir)],
        capture_output=True,
        text=True
    )
    
    duration = time.perf_counter() - start
    
    return ExampleBuildResult(
        path = example_dir,
        is_success=(proc.returncode == 0),
        duration=duration,
        stdout=proc.stdout,
        stderr=proc.stderr,
    )


def determine_examples() -> Sequence[Path]:
    failed_example_paths = read_failed_list()

    if failed_example_paths:
        return failed_example_paths
    
    return find_all_example_dirs()


def print_bordered(message: str) -> None:
    print(f"{' ' + message + ' ':=^{terminal_width}}", flush=True)


def print_table_row(padding: int, name: str, status: str, elapsed: str) -> None:
    print(f"{name:{padding}} {status:10} {elapsed}")


def main():
    example_dirs = determine_examples()

    if not example_dirs:
        print("Nothing to build")
        return 0

    total_examples = len(example_dirs)
    print(f"Total examples to build: {total_examples}")
    
    summary = ExamplesBuildSummary()

    for i, d in enumerate(example_dirs):
        print(f"Building ({i + 1}/{total_examples}) {d}", flush=True)
        
        result = build_example(d)
        summary.add(result)
        
        if not result.is_success:
            print(result.stdout)
            print(red(result.stderr))

        print_bordered(f"[{result.status()}] Took {result.duration:.2f} seconds")


    print_bordered(green("SUMMARY"))

    padding = summary.max_example_name()

    print_table_row(padding, "Example", "Status", "Duration")
    for r in summary.all():
        print_table_row(padding, r.name(), r.status(), f"{r.duration:.3f} s")

    failed_examples = summary.failed_examples_paths()
    
    print_bordered(f"{len(failed_examples)} failed, {total_examples - len(failed_examples)} succeeded in {summary.total_time():.2f} seconds")
    
    write_failed_list(failed_examples)
    return bool(failed_examples)

if __name__ == "__main__":
    sys.exit(main())