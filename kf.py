#!/usr/bin/env python3

# Copyright (c) 2026 KiraFlux
# SPDX-License-Identifier: MIT

from abc import ABC, abstractmethod
import argparse
from datetime import datetime
import enum
import os
import sys
import subprocess
from dataclasses import dataclass
from pathlib import Path
import time
from itertools import chain
from typing import Final, Iterable, Self, Sequence, Optional, TextIO, final


class Environment(enum.StrEnum):
    NATIVE = "native"
    ESP32_DEV = "esp32dev"


class Color(enum.StrEnum):
    RED = "\033[91m"
    GREEN = "\033[92m"
    YELLOW = "\033[93m"
    BLUE = "\033[94m"
    CYAN = "\033[96m"
    BOLD = "\033[1m"
    RESET = "\033[0m"

    def apply(self, s: str) -> str:
        return self + s + self.RESET


def terminal_width() -> int:
    try:
        return os.get_terminal_size().columns
    except OSError:
        return 80


def make_bordered(message: str, *, thick=False) -> str:
    char = "=" if thick else "-"
    return f"{' ' + message + ' ':{char}^{terminal_width()}}"


def print_footer(is_success: bool, *, success_message: str = "SUCCESS") -> None:
    message, color = ((success_message, Color.GREEN) if is_success else ("FAILED", Color.RED))
    print(color.apply(make_bordered(message, thick=True)))


def print_table_row(padding: int, name: str, status: str, elapsed: str) -> None:
    print(f"{name:{padding}} {status:10} {elapsed}")


class Job(ABC):

    examples_dir_name = "examples"

    platformio_ini_file = "platformio.ini"

    source_extensions: Final = (
        "h", "hpp", "cpp",
    )

    docs_extensions: Final = (
        "md",
    )

    config_extensions: Final = (
        "yml", "json",
    )

    script_extensions: Final = (
        "py", "sh", "mak",
    )

    source_dirs: Final = (
        examples_dir_name, "src", "test",
    )

    misc_dirs: Final = (
        ".github",
    )

    repo_dir: Final = Path(__file__).parent.resolve()

    registry: Final = list[Self]()

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        cls.registry.append(cls())

    @abstractmethod
    def register(self, subparsers):
        pass

    @abstractmethod
    def run(self, args) -> int:
        raise NotImplementedError

    @dataclass(frozen=True, kw_only=True)
    class CmdResult:
        is_success: bool
        duration: float
        stdout: str = ""
        stderr: str = ""

        def status(self) -> str:
            return (Color.GREEN.apply("SUCCESS") if self.is_success else Color.RED.apply("FAILED"))

    @final
    def run_cmd(self, args: Sequence[str]) -> CmdResult:
        start = time.perf_counter()

        proc = subprocess.run(
            args,
            capture_output=True,
            text=True
        )

        duration = time.perf_counter() - start

        return self.CmdResult(
            is_success=(proc.returncode == 0),
            duration=duration,
            stdout=proc.stdout,
            stderr=proc.stderr,
        )

    @final
    def patterns_from_ext(self, extensions: Sequence[str]) -> Iterable[str]:
        return (
            f"*.{e}"
            for e in extensions
        )

    @final
    def get_files_by_patterns(self, _dir: Path, patterns: Sequence[str], *, recursive=False) -> Iterable[Path]:
        def _glob_flat(d: Path, p: str) -> Iterable[Path]:
            return d.glob(p)

        def _glob_recursive(d: Path, p: str) -> Iterable[Path]:
            return d.rglob(p)

        glob_method = _glob_recursive if recursive else _glob_flat

        return chain(*(
            glob_method(_dir, p)
            for p in patterns
        ))


class BulkPathsJob(Job, ABC):

    def __init__(self):
        super().__init__()
        self.paths: Sequence[Path] = ()

    @abstractmethod
    def on_path(self, path: Path, args) -> None:
        """Action on each path"""

    @abstractmethod
    def on_begin(self, args) -> bool:
        """Action on beginning of processing"""

    @abstractmethod
    def on_end(self, args) -> bool:
        """Action on end of processing"""

    @abstractmethod
    def determine_paths(self, args) -> Iterable[Path]:
        "Get paths processed by Job"

    @final
    def run(self, args) -> int:
        self.paths = sorted(self.determine_paths(args))
        path_count = len(self.paths)

        print(Color.BOLD.apply(f"Running '{self.__class__.__name__}'"))

        if path_count == 0:
            print("Nothing to process.")
            return 0

        print(Color.CYAN.apply(f"Find {path_count} paths to process"))

        if not self.on_begin(args):
            return 1

        for i, path in enumerate(self.paths):
            print(f"[{i + 1}/{path_count}]  \t{Color.CYAN.apply(str(path.relative_to(self.repo_dir)))}", flush=True)

            self.on_path(path, args)

        return self.on_end(args)


class BuildJob(BulkPathsJob):

    failed_examples_list_file = Job.repo_dir / "failed_examples.txt"

    examples_dir = Job.repo_dir / Job.examples_dir_name
    examples_glob = f"{Job.examples_dir_name}/*/*/{Job.platformio_ini_file}"

    @dataclass(frozen=True, kw_only=True)
    class ExampleBuildResult(Job.CmdResult):
        example_dir: Path

        def name(self) -> str:
            return str(self.example_dir.relative_to(BuildJob.examples_dir))

    def __init__(self):
        super().__init__()
        self._results: Final = list[self.ExampleBuildResult]()

        self._example_env: Optional[Environment] = None

    def on_path(self, path: Path, args) -> None:
        cmd_result = self.run_cmd((
            "pio", "run", "--silent", "-e", self._example_env, "-d", str(path)
        ))

        build_result = self.ExampleBuildResult(
            is_success=cmd_result.is_success,
            duration=cmd_result.duration,
            stdout=cmd_result.stdout,
            stderr=cmd_result.stderr,
            example_dir=path,
        )

        self._results.append(build_result)

        if not build_result.is_success:
            print(build_result.stdout)
            print(Color.RED.apply(build_result.stderr))

        print(make_bordered(f"[{build_result.status()}] Took {build_result.duration:.2f} seconds"))

    def on_begin(self, args) -> bool:
        try:
            self._example_env = Environment(args.env)
            return True

        except ValueError as e:
            print(Color.RED.apply(e))
            return False

    def on_end(self, args) -> bool:
        print_footer(True, success_message="SUMMARY")

        padding = self._max_example_name()

        print_table_row(padding, "Example", "Status", "Duration")

        for r in self._results:
            print_table_row(padding, r.name(), r.status(), f"{r.duration:.3f} s")

        failed_examples = tuple(self._failed_examples_paths())

        print(make_bordered(f"{len(failed_examples)} failed, {len(self._results) - len(failed_examples)} succeeded in {self._total_time():.2f} seconds"))

        self._write_failed_list(failed_examples)
        return bool(failed_examples)

    def determine_paths(self, args) -> Iterable[Path]:
        failed_example_paths = self._read_failed_list()

        if args.all or not failed_example_paths:
            return self._find_all_example_dirs()

        return failed_example_paths

    def register(self, subparsers):
        p = subparsers.add_parser("build", aliases=["b"])
        p.add_argument("--env", default=Environment.ESP32_DEV)
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def _max_example_name(self) -> int:
        return max(
            len(r.name())
            for r in self._results
        )

    def _total_time(self) -> float:
        return sum(
            r.duration
            for r in self._results
        )

    def _failed_examples_paths(self) -> Iterable[Path]:
        return (
            r.example_dir
            for r in self._results
            if not r.is_success
        )

    def _find_all_example_dirs(self) -> Iterable[Path]:
        return (
            p.parent
            for p in self.repo_dir.glob(self.examples_glob)
        )

    def _read_failed_list(self) -> Sequence[Path]:
        """Read list of relative paths from failed file."""

        if not self.failed_examples_list_file.exists() or (self.failed_examples_list_file.stat().st_size == 0):
            return ()

        with open(self.failed_examples_list_file, "r") as f:
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
                    (lambda l: self.repo_dir / l),
                    lines
                )
            )

            return tuple(paths)

    def _write_failed_list(self, failed_paths: Sequence[Path]) -> None:
        """Write relative paths of failed examples."""

        self.failed_examples_list_file.write_text("\n".join(
            str(p.relative_to(self.repo_dir))
            for p in failed_paths
        ))


class NewJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("new", aliases=["n"])
        p.add_argument("kind", choices=["example", "e", "test", "t"])
        p.add_argument("group")
        p.add_argument("name")
        p.set_defaults(job=self)

    def run(self, args):
        print("New command (stub)")
        return 0


class FormatJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("format", aliases=["f"])
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def run(self, args):
        print("Format command (stub)")
        return 0


class LintJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("lint", aliases=["l"])
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def run(self, args):
        print("Lint command (stub)")
        return 0


class TestJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("test", aliases=["t"])
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def run(self, args):
        print("Test command (stub)")
        return 0


class MonitorJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("monitor", aliases=["m"])
        p.set_defaults(job=self)

    def run(self, args):
        print("Monitor command (stub)")
        return 0


class SnapshotJob(BulkPathsJob):

    snapshot_file: Final = Job.repo_dir / "snapshot.txt"

    bypass_filenames: Final = (
        Job.platformio_ini_file,
    )

    ignored_filenames: Final = (
        "compile_commands.json",
        "CHANGELOG.md"
    )

    def __init__(self):
        super().__init__()
        self._fact_writes = 0
        self._io: Optional[TextIO] = None

    def on_path(self, path: Path, args) -> None:
        next_line = "\n"
        backticks = "```"

        try:
            file_text = path.read_text()
        except Exception as e:
            print(Color.RED.apply(f"failed to read '{path}' error: {e}"))
            return

        header = str(path.relative_to(self.repo_dir))

        self._io.write(header + next_line + backticks + path.suffix + next_line)
        self._io.write(file_text)
        self._io.write(backticks + next_line * 3)

        self._fact_writes += 1

    def on_begin(self, args) -> bool:
        self._io = self.snapshot_file.open("wt")
        self._io.write(f"KiraFlux Toolkit repository snapshot ({datetime.now()})\n\n")
        return True

    def on_end(self, args) -> bool:
        self._io.close()

        is_success = (len(self.paths) == self._fact_writes)

        print(f"\n{Color.BOLD}Snapshot file:{Color.RESET}\n\t{Color.BLUE.apply(str(self.snapshot_file))}")
        print_footer(is_success)

        return is_success

    def determine_paths(self, args) -> Iterable[Path]:
        patterns = tuple(self.patterns_from_ext(chain(
            self.source_extensions,
            self.script_extensions,
            self.config_extensions,
            self.docs_extensions,
        )))

        root_files = self.get_files_by_patterns(self.repo_dir, chain(patterns, self.bypass_filenames))

        dirs_files = (
            self.get_files_by_patterns(self.repo_dir / dir_name, patterns, recursive=True)
            for dir_name in chain(self.source_dirs, self.misc_dirs)
        )

        return filter(
            (lambda p: p.name not in self.ignored_filenames),
            chain(
                root_files,
                *dirs_files
            )
        )

    def register(self, subparsers):
        p = subparsers.add_parser("snapshot", aliases=["s"])
        p.set_defaults(job=self)


class DiffJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("diff", aliases=["d"])
        p.set_defaults(job=self)

    def run(self, args):
        print("Diff command (stub)")
        return 0


class CleanJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("clean", aliases=["c"])
        p.set_defaults(job=self)

    def run(self, args):
        print("Clean command (stub)")
        return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="KiraFlux Toolkit CLI")
    sub = parser.add_subparsers(dest="command", required=True)

    for job in Job.registry:
        job.register(sub)

    args = parser.parse_args()
    return args.job.run(args)


if __name__ == "__main__":
    sys.exit(main())
