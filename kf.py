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


def print_bordered(message: str) -> None:
    print(f"{' ' + message + ' ':=^{terminal_width()}}", flush=True)


def print_table_row(padding: int, name: str, status: str, elapsed: str) -> None:
    print(f"{name:{padding}} {status:10} {elapsed}")


class Job(ABC):
    repo_dir: Final = Path(__file__).parent.resolve()

    platformio_ini_file = "platformio.ini"

    registry: Final = list[Self]()

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        cls.registry.append(cls())

    @abstractmethod
    def register(self, subparsers):
        pass

    @abstractmethod
    def run(self, args):
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


class BuildJob(Job):

    failed_examples_list_file = Job.repo_dir / "failed_examples.txt"

    examples_dir_name = "examples"
    examples_dir = Job.repo_dir / examples_dir_name
    examples_glob = f"{examples_dir_name}/*/*/{Job.platformio_ini_file}"

    @dataclass(kw_only=True)
    class ExampleBuildResult:
        cmd: Job.CmdResult
        example_dir: Path

        def name(self) -> str:
            return str(self.example_dir.relative_to(BuildJob.examples_dir))

    class ExamplesBuildSummary:

        def __init__(self):
            self._results: Final = list[BuildJob.ExampleBuildResult]()

        def add(self, result: BuildJob.ExampleBuildResult) -> None:
            self._results.append(result)
        
        def all(self) -> Sequence[BuildJob.ExampleBuildResult]:
            return self._results

        def max_example_name(self) -> int:
            return max(
                len(r.name())
                for r in self._results
            )

        def total_time(self) -> float:
            return sum(
                r.cmd.duration
                for r in self._results
            )
        
        def failed_examples_paths(self) -> Sequence[Path]:
            return tuple(
                r.example_dir
                for r in self._results
                if not r.cmd.is_success
            )

    def register(self, subparsers):
        p = subparsers.add_parser("build", aliases=["b"])
        p.add_argument("--env", default=Environment.ESP32_DEV)
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def run(self, args):
        print("Build command")

        try:
            example_env = Environment(args.env)
        except ValueError as e:
            print(e)
            return 1

        example_dirs = self._determine_examples(args.all)

        if not example_dirs:
            print("Nothing to build")
            return 0

        total_examples = len(example_dirs)
        print(f"Total examples to build: {total_examples}")
        
        summary = self.ExamplesBuildSummary()

        for i, d in enumerate(example_dirs):
            print(f"Building ({i + 1}/{total_examples}) {d}", flush=True)
            
            result = self._build_example(d, example_env)
            summary.add(result)
            
            if not result.cmd.is_success:
                print(result.cmd.stdout)
                print(Color.RED.apply(result.cmd.stderr))

            print_bordered(f"[{result.cmd.status()}] Took {result.cmd.duration:.2f} seconds")

        print_bordered(Color.GREEN.apply("SUMMARY"))

        padding = summary.max_example_name()

        print_table_row(padding, "Example", "Status", "Duration")
        for r in summary.all():
            print_table_row(padding, r.name(), r.cmd.status(), f"{r.cmd.duration:.3f} s")

        failed_examples = summary.failed_examples_paths()
        
        print_bordered(f"{len(failed_examples)} failed, {total_examples - len(failed_examples)} succeeded in {summary.total_time():.2f} seconds")
        
        self._write_failed_list(failed_examples)
        return bool(failed_examples)

    def _find_all_example_dirs(self) -> Sequence[Path]:
        return sorted(p.parent for p in self.repo_dir.glob(self.examples_glob))

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

    def _determine_examples(self, force_all: bool) -> Sequence[Path]:
        failed_example_paths = self._read_failed_list()

        if force_all or not failed_example_paths: 
            return self._find_all_example_dirs()

        return failed_example_paths
        

    def _build_example(self, example_dir: Path, env: Environment) -> ExampleBuildResult:
        """Build a single example"""
        
        return self.ExampleBuildResult(
            cmd=self.run_cmd((
                "pio", "run", "--silent", "-e", env, "-d", str(example_dir)
            )),
            example_dir = example_dir,
        )

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


class SnapshotJob(Job):

    snapshot_file: Final = Job.repo_dir / "snapshot.txt"

    target_ext: Final = (
        "h", "hpp", "cpp", "ino", 
        "md", "MD",
        "yml", "json", 
        "py", "sh", 
    )

    target_dirs: Final = (
        "examples", "src", "test", ".github",
    )

    bypass_filenames: Final = (
        "platformio.ini",
    )

    ignored_filenames: Final = (
        "compile_commands.json",
        "CHANGELOG.MD"
    )

    def register(self, subparsers):
        p = subparsers.add_parser("snapshot", aliases=["s"])
        p.set_defaults(job=self)

    def run(self, args):
        print(Color.BOLD.apply("Making Snapshot..."))

        snapshot_files = tuple(self._determine_snapshot_file_paths())

        print(Color.CYAN.apply(f"where are {len(snapshot_files)} files to write"))

        fact_writes = 0;

        with self.snapshot_file.open("wt") as f:
            f.write(f"KiraFlux Toolkit repository snapshot ({datetime.now()})\n\n")

            for i, p in enumerate(snapshot_files):
                print(f"* {i:>3} {(p.relative_to(self.repo_dir))}")
                
                fact_writes += self._write_content(f, p)

        is_success = (len(snapshot_files) == fact_writes) 

        print(f"file: {Color.CYAN.apply(str(self.snapshot_file))}")

        s_str = (Color.GREEN.apply("SUCCESS") if is_success else Color.RED.apply("FAILED"))

        print_bordered(s_str)

        return is_success

    def _write_content(self, io: TextIO, path: Path) -> bool:
        next_line = "\n"
        backticks = "```"

        try:
            file_text = path.read_text()
        except Exception as e:
            print(Color.RED.apply(f"failed to read '{path}' error: {e}"))
            return False

        header = str(path.relative_to(self.repo_dir))

        io.write(header + next_line + backticks + path.suffix + next_line)
        io.write(file_text)
        io.write(backticks + next_line * 3)

        return True

    def _patterns_from_ext(self, extensions: Sequence[str]) -> Iterable[str]:
        return (
            f"*.{e}"
            for e in extensions
        )

    def _get_files_by_patterns(self, _dir: Path, patterns: Sequence[str], *, recursive = False) -> Iterable[Path]:
        def _glob_flat(d: Path, p: str) -> Iterable[Path]:
            return d.glob(p)

        def _glob_recursive(d: Path, p: str) -> Iterable[Path]:
            return d.rglob(p)

        glob_method = _glob_recursive if recursive else _glob_flat

        return chain(*(
            glob_method(_dir, p)
            for p in patterns
        ))

    def _determine_snapshot_file_paths(self) -> Iterable[Path]:
        patterns = tuple(self._patterns_from_ext(self.target_ext))

        root_files = self._get_files_by_patterns(self.repo_dir, patterns + self.bypass_filenames)

        dirs_files = (
            self._get_files_by_patterns(self.repo_dir / dir_name, patterns, recursive=True)
            for dir_name in self.target_dirs
        )

        return filter(
            (lambda p: p.name not in self.ignored_filenames),
            chain(
                root_files,
                *dirs_files
            )
        )


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