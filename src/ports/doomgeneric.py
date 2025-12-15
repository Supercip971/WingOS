
import sys 
import os 
from cutekit import builder,  const, model, cli, vt100, jexpr, rules
from typing_extensions import Self


MODULE_PATH = os.path.join(const.SRC_DIR, "ports", "wingos-doomgeneric")


def shell_run(cmd, cwd=None):
    vt100.title(f"Running command: {' '.join(cmd)} in {cwd if cwd else os.getcwd()}")

    #result = os.system(" ".join(cmd) if isinstance(cmd, list) else cmd)
    # result = os.system(" ".join(cmd) if isinstance(cmd, list) else cmd)
    result = os.system("(cd {} && {})".format(cwd if cwd else os.getcwd(), " ".join(cmd) if isinstance(cmd, list) else cmd))
    
    return result
def port():
    
    if not os.path.exists(MODULE_PATH):
        vt100.error(f"doom generic port source not found at {MODULE_PATH}")
        return None
    
    cross_compiler = os.path.join("meta", "build", "cross",  "bin", "x86_64-pc-wingos-gcc")

    sysroot = os.path.abspath(os.path.join("meta", "build", "sysroot"))

    
    # run: CC=/home/cyp/project/wingos/meta/build/cross/bin/x86_64-pc-wingos-gcc ./configure --host=x86_64-wingos --prefix=/home/cyp/project/wingos/meta/build/sysroot/
    shell_run([f"CC={os.path.abspath(cross_compiler)}",
               "CXX={}".format(os.path.abspath(cross_compiler).replace("gcc", "g++")),
               "make", "clean", "-j"], cwd=os.path.join(MODULE_PATH, "doomgeneric/"))


    shell_run([f"CC={os.path.abspath(cross_compiler)}",
               "CXX={}".format(os.path.abspath(cross_compiler).replace("gcc", "g++")),
               "make", "all", "-j"], cwd=os.path.join(MODULE_PATH, "doomgeneric/"))


    shell_run(["cp doomgeneric {}".format(os.path.abspath( os.path.join(const.PROJECT_CK_DIR, "wingos-disk/bin/doomgeneric"))),], cwd=os.path.join(MODULE_PATH, "doomgeneric/"
                                                                                                        ))
    return "hello"