# clang-tidy "-checks=-*,misc-include-cleaner,clang-diagnostic-dangling" src/**.cpp src/**.h src/**.hpp
#
#

import importlib.util as importlib
import json
import os
import shutil
import sys
from typing import Literal, Union

from cutekit import builder, cli, const, jexpr, model, rules, shell, vt100
from typing_extensions import Self


def shell_run(cmd, cwd=None):
    vt100.title(f"Running command: {' '.join(cmd)} in {cwd if cwd else os.getcwd()}")
    result = os.system(" ".join(cmd) if isinstance(cmd, list) else cmd)

    return result


def build_object_db(
    args: model.TargetArgs, component, target, genCompileCommands=False
):
    vt100.title(f"checking: {component} with target {target}")

    if target != "":
        args.target = target

    scope = builder.TargetScope.use(args)

    model.Registry.use(args)
    # components = list(registry.iter(model.Component))
    #    targets = builder.TargetScope(args)
    fcomponent = scope.registry.lookup(
        component, type=(model.Component, model.Target), includeProvides=True
    )

    if fcomponent is None:
        raise ValueError(f"Component '{component}' not found")

    db_product = builder.build(scope, fcomponent, generateCompilationDb=True)[0]

    # filter src/ to not include src/ports and src/external
    # open json file and extract source files
    with open("compile_commands.json", "r") as f:
        db = json.load(f)
        source_files = [entry["file"] for entry in db]
        source_files = [f for f in source_files if not f.startswith("src/ports")]
        source_files = [f for f in source_files if not f.startswith("src/libmath")]
        source_files = [f for f in source_files if not f.startswith("src/external")]

    vt100.title(f"Found {len(source_files)} source files")
    shell_run(
        [
            "run-clang-tidy",
            "-j",
            "8",
            "--use-color",
            "-p",
            ".",
            "-checks=-*,misc-include-cleaner,clang-diagnostic-dangling",
        ]
        + source_files,
        const.SRC_DIR,
    )


def checkSystem(args: model.TargetArgs, genCompileCommands=False):
    builder.TargetScope.use(args)
    registry = model.Registry.use(args)
    components = list(registry.iter(model.Component))

    build_object_db(args, "kernel-loader-limine", "kernel-x86_64", genCompileCommands)

    list(registry.iter(model.Target))

    vt100.title("Components")
    vt100.p(f"{components}")
    for pkg in filter(lambda m: const.EXTERN_DIR not in m.dirname(), components):
        if (
            pkg.type == model.Kind.EXE
            and pkg.id != "kernel-loader-limine"
            and pkg.props.get("exported", "") == "module"
        ):
            build_object_db(args, pkg.id, "wingos-x86_64", genCompileCommands)
        if (
            pkg.type == model.Kind.EXE
            and pkg.id != "kernel-loader-limine"
            and pkg.props.get("exported", "") == "disk"
        ):
            build_object_db(args, pkg.id, "wingos-x86_64", genCompileCommands)


@cli.command("gen-db", "")
def dbCreatioun(args: model.TargetArgs, genCompileCommands=True):
    checkSystem(args)


def find_all_source() -> list[str]:
    result: list[str] = []

    for root, _, files in os.walk(const.SRC_DIR):
        for f in files:
            vt100.title(f"{root}")
            if (
                not root.startswith("src/ports")
                and not root.startswith("src/external")
                and not root.startswith("src/libmath")
                and f.endswith((".cpp", ".h", ".hpp", ".c"))
            ):
                result.append(os.path.join(root, f))

    # sort for reproducibility
    return sorted(result)


@cli.command("check", "check-code-quality")
def qualityCheck(args: model.TargetArgs):

    args.target = "wingos-x86_64"
    # registry = model.Registry.use(args)
    vt100.title("checking code quality")

    # filter src/ to not include src/ports and src/external
    source_files = find_all_source()
    vt100.title(f"Found {len(source_files)} source files")
    shell_run(
        [
            "clang-tidy",
            "-checks=-*,misc-include-cleaner,clang-diagnostic-dangling",
            "--extra-arg=-Isrc/",
        ]
        + source_files,
        const.SRC_DIR,
    )
