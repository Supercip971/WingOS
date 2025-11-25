
import sys 
import os 
from cutekit import builder,  const, model, cli, vt100, jexpr, rules
from typing_extensions import Self


DASH_PATH = os.path.join(const.SRC_DIR, "ports", "dash-wingos")


def shell_run(cmd, cwd=None):
    vt100.title(f"Running command: {' '.join(cmd)} in {cwd if cwd else os.getcwd()}")

    #result = os.system(" ".join(cmd) if isinstance(cmd, list) else cmd)
    # result = os.system(" ".join(cmd) if isinstance(cmd, list) else cmd)
    result = os.system("(cd {} && {})".format(cwd if cwd else os.getcwd(), " ".join(cmd) if isinstance(cmd, list) else cmd))
    
    return result
def port():
    
    if not os.path.exists(DASH_PATH):
        vt100.error(f"Dash port source not found at {DASH_PATH}")
        return None
    
    cross_compiler = os.path.join("meta", "build", "cross",  "bin", "x86_64-pc-wingos-gcc")

    sysroot = os.path.abspath(os.path.join("meta", "build", "sysroot"))

    if not os.path.exists(os.path.join(DASH_PATH, "configure")):
        vt100.title("Running dash config")
        shell_run(["./autogen.sh"], cwd=DASH_PATH)
        shell_run([f"CC={os.path.abspath(cross_compiler)}", os.path.join("./configure"), f"--host=x86_64-wingos", f"--prefix={sysroot}"], cwd=DASH_PATH)
    
    
    # run: CC=/home/cyp/project/wingos/meta/build/cross/bin/x86_64-pc-wingos-gcc ./configure --host=x86_64-wingos --prefix=/home/cyp/project/wingos/meta/build/sysroot/
   
    shell_run([f"CC={os.path.abspath(cross_compiler)}", "make", f"all"], cwd=DASH_PATH)


    
    return "hello"