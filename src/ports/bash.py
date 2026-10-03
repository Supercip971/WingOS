
import sys
import os
from cutekit import builder,  const, model, cli, vt100, jexpr, rules
from typing_extensions import Self

bash_PATH = os.path.join(const.SRC_DIR, "ports", "wingos-bash")

def shell_run(cmd, cwd=None):
    vt100.title(f"Running command: {' '.join(cmd)} in {cwd if cwd else os.getcwd()}")

    #result = os.system(" ".join(cmd) if isinstance(cmd, list) else cmd)
    # result = os.system(" ".join(cmd) if isinstance(cmd, list) else cmd)
    result = os.system("(cd {} && {})".format(cwd if cwd else os.getcwd(), " ".join(cmd) if isinstance(cmd, list) else cmd))

    return result
def port():

    if not os.path.exists(bash_PATH):
        vt100.error(f"bash port source not found at {bash_PATH}")
        return None

    cross_compiler = os.path.join("meta", "build", "cross",  "bin", "x86_64-pc-wingos-gcc")

    sysroot = os.path.abspath(os.path.join("meta", "build", "sysroot"))

    if not os.path.exists(os.path.join(bash_PATH, "makefile")):
        vt100.title("Running bash config")
        shell_run([f"CC={os.path.abspath(cross_compiler)} LSCRIPT={os.path.abspath(os.path.join('meta', 'targets', 'wingos-x86_64.ld'))}", os.path.join("./configure"), f"--host=x86_64-wingos", f"--prefix={sysroot}"], cwd=bash_PATH)

    # run: CC=/home/cyp/project/wingos/meta/build/cross/bin/x86_64-pc-wingos-gcc ./configure --host=x86_64-wingos --prefix=/home/cyp/project/wingos/meta/build/sysroot/
    shell_run([f"CC={os.path.abspath(cross_compiler)} LSCRIPT={os.path.abspath(os.path.join('meta', 'targets', 'wingos-x86_64.ld'))} CFLAGS='-std=c89'", "make"], cwd=bash_PATH)

    shell_run([f"CC={os.path.abspath(cross_compiler)} LSCRIPT={os.path.abspath(os.path.join('meta', 'targets', 'wingos-x86_64.ld'))} CFLAGS='-std=c89'", "make", f"all"], cwd=bash_PATH)



    return "hello"
