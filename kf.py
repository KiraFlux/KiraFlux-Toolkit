#!/usr/bin/env python3

# Copyright (c) 2026 KiraFlux
# SPDX-License-Identifier: MIT

from abc import ABC, abstractmethod
import argparse
import enum
import sys
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Final, Self, Sequence, Optional

REPO_DIR: Final = Path(__file__).parent.resolve()


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


def colorize(text: str, color: Color, bold: bool = False) -> str:
    return f"{Color.BOLD if bold else ''}{color}{text}{Color.RESET}"


@dataclass(frozen=True, kw_only=True)
class CmdResult:
    ok: bool
    stdout: str = ""
    stderr: str = ""


def run_cmd(cmd: Sequence[str], capture: bool = False, cwd: Optional[Path] = None) -> CmdResult:
    try:
        res = subprocess.run(cmd, capture_output=capture, text=True, cwd=str(cwd) if cwd else None)
        return CmdResult(ok=res.returncode == 0, stdout=res.stdout, stderr=res.stderr)
    except subprocess.CalledProcessError as e:
        return CmdResult(ok=False, stdout=e.stdout, stderr=e.stderr)


def get_changed_files() -> Sequence[Path]:
    return ()


class Job(ABC):
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


class BuildJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("build", aliases=["b"])
        p.add_argument("example", nargs="?")
        p.add_argument("--env", default=Environment.ESP32_DEV)
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("Build command (stub)", Color.YELLOW))
        return 0


class NewJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("new", aliases=["n"])
        p.add_argument("kind", choices=["example", "e", "test", "t"])
        p.add_argument("group")
        p.add_argument("name")
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("New command (stub)", Color.YELLOW))
        return 0


class FormatJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("format", aliases=["f"])
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("Format command (stub)", Color.YELLOW))
        return 0


class LintJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("lint", aliases=["l"])
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("Lint command (stub)", Color.YELLOW))
        return 0


class TestJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("test", aliases=["t"])
        p.add_argument("--all", action="store_true")
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("Test command (stub)", Color.YELLOW))
        return 0


class MonitorJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("monitor", aliases=["m"])
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("Monitor command (stub)", Color.YELLOW))
        return 0


class SnapshotJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("snapshot", aliases=["s"])
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("Snapshot command (stub)", Color.YELLOW))
        return 0


class DiffJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("diff", aliases=["d"])
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("Diff command (stub)", Color.YELLOW))
        return 0


class CleanJob(Job):
    def register(self, subparsers):
        p = subparsers.add_parser("clean", aliases=["c"])
        p.set_defaults(job=self)

    def run(self, args):
        print(colorize("Clean command (stub)", Color.YELLOW))
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