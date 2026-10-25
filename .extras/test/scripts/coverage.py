#!/usr/bin/env python3
"""Collect LLVM source coverage from the HTTP examples only."""

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys


def run(command, **kwargs):
    return subprocess.run(command, check=True, **kwargs)


def collect(args):
    build_dir = Path(args.build_dir).resolve()
    source_dir = Path(os.path.abspath(args.source_dir))
    manifest = Path(args.manifest)
    executables = list(dict.fromkeys(manifest.read_text().splitlines()))
    if not executables or any(
        not name or not Path(name).is_absolute() or not Path(name).is_file()
        or not os.access(name, os.X_OK)
        for name in executables
    ):
        raise ValueError("The HTTP executable manifest must contain existing absolute paths")
    sources = sorted(
        str(path) for path in source_dir.rglob("*")
        if path.is_file() and path.suffix in (".cpp", ".h")
    )
    if not sources:
        raise ValueError("No library source files found")

    # Other test runs and earlier coverage reports must not affect this run.
    report_dir = build_dir / "coverage-http"
    if report_dir.is_symlink():
        raise ValueError("Refusing to replace a symlink at the coverage output path")
    if report_dir.exists():
        shutil.rmtree(report_dir)
    raw_dir = report_dir / "raw"
    raw_dir.mkdir(parents=True)

    environment = os.environ.copy()
    environment["LLVM_PROFILE_FILE"] = str(raw_dir / "%m-%p.profraw")
    command = [args.ctest, "-L", "^http$", "--output-on-failure"]
    if args.config:
        command.extend(["-C", args.config])
    run(command, cwd=build_dir, env=environment)
    profiles = sorted(str(path) for path in raw_dir.glob("*.profraw"))
    if not profiles:
        raise ValueError("The HTTP tests produced no coverage profiles")

    profile = report_dir / "coverage.profdata"
    run([args.llvm_profdata, "merge", "-sparse", *profiles, "-o", str(profile)])
    coverage_args = [executables[0], "-instr-profile=" + str(profile)]
    for executable in executables[1:]:
        coverage_args.extend(["-object", executable])
    source_args = ["--sources", *sources]

    json_path = report_dir / "coverage.json"
    with json_path.open("w") as output:
        run([args.llvm_cov, "export", *coverage_args, *source_args], stdout=output)
    coverage = json.loads(json_path.read_text())
    datasets = coverage.get("data", [])
    if not datasets or not any(dataset.get("files") for dataset in datasets):
        raise ValueError("The HTTP profiles contain no coverage for the library sources")
    source_paths = {Path(path).resolve() for path in sources}
    if any(
        Path(file["filename"]).resolve() not in source_paths
        for dataset in datasets for file in dataset["files"]
    ):
        raise ValueError("LLVM source filtering included files outside the library sources")
    totals = {}
    for metric in ("lines", "functions", "branches"):
        totals[metric] = {
            key: sum(dataset["totals"][metric][key] for dataset in datasets)
            for key in ("count", "covered")
        }
    if not totals["lines"]["count"]:
        raise ValueError("The HTTP profiles contain no instrumented library source lines")

    summary = run(
        [args.llvm_cov, "report", *coverage_args, *source_args],
        capture_output=True, text=True,
    )
    if summary.stderr:
        print(summary.stderr, file=sys.stderr, end="")
    (report_dir / "summary.txt").write_text(summary.stdout)
    print(summary.stdout, end="", flush=True)
    run([
        args.llvm_cov, "show", *coverage_args, "-format=html",
        "-output-dir=" + str(report_dir / "html"),
        "-show-line-counts-or-regions", "-show-branches=count", *source_args,
    ])
    with (report_dir / "coverage.lcov").open("w") as output:
        run([
            args.llvm_cov, "export", *coverage_args, "-format=lcov", *source_args,
        ], stdout=output)

    markdown = [
        "## HTTP test coverage", "",
        "Only tests with the `http` label contribute to this report. "
        "Coverage measures instrumented native code in `src/`, including "
        "instantiated templates. Uninstantiated templates and inactive "
        "conditional code are outside the measured totals.", "",
        "| Metric | Covered | Total | Coverage |",
        "| --- | ---: | ---: | ---: |",
    ]
    for metric, counts in totals.items():
        covered, count = counts["covered"], counts["count"]
        percent = f"{100 * covered / count:.2f}%" if count else "n/a"
        markdown.append(f"| {metric.title()} | {covered} | {count} | {percent} |")
    markdown.extend([
        "", "The coverage artifact includes `html/index.html`, `summary.txt`, "
        "`coverage.json`, and `coverage.lcov`.", "",
    ])
    (report_dir / "summary.md").write_text("\n".join(markdown))
    print(f"HTTP coverage report: {report_dir / 'html' / 'index.html'}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("build-dir", "source-dir", "manifest", "llvm-cov", "llvm-profdata", "ctest"):
        parser.add_argument("--" + name, required=True)
    parser.add_argument("--config", default="")
    args = parser.parse_args()
    try:
        collect(args)
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        print(f"HTTP coverage failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
