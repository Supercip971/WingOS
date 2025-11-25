import os
import shutil

from cutekit import builder,  const, shell, model, cli, vt100, jexpr, rules
from typing_extensions import Self
import importlib.util as importlib
import sys 
def load_module(source, module_name=None):
    """
    reads file source and loads it as a module

    :param source: file to load
    :param module_name: name of module to register in sys.modules
    :return: loaded module
    """


    spec = importlib.util.spec_from_file_location(module_name, source)
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)

    return module


def shell_run(cmd, cwd=None):
    vt100.title(f"Running command: {' '.join(cmd)} in {cwd if cwd else os.getcwd()}")
    result = os.system(" ".join(cmd) if isinstance(cmd, list) else cmd)
    
    
    return result
@cli.command("p", "Build ports")
def portFunc(args: model.TargetArgs):


    args.target = "wingos-x86_64"
    scope = builder.TargetScope.use(args)
    
    # registry = model.Registry.use(args)
    
    vt100.title("Building libc...")



    vt100.title("Building iol...")
    manifest2 = scope.registry.lookup("iol-wingos", type=model.Component, includeProvides=True)
    iol_product = builder.build(scope, manifest2)[0]
    
    crt0_path = scope.buildpath(f"iol-wingos/__obj__/crt0.cpp.o")
    vt100.title(f"Built crt0 at: {crt0_path}")

    manifest_libc = scope.registry.lookup("libc-custom", type=model.Component, includeProvides=True)
    libc_product = builder.build(scope, manifest_libc)[0]
    outfile_libc = builder.outfile(libc_product)

    vt100.title(f"Built libc: {outfile_libc}")
 
    manifest_iol = scope.registry.lookup("iol-wingos", type=model.Component, includeProvides=True)
    iol_product = builder.build(scope, manifest_iol)[0]
    outfile_iol = builder.outfile(iol_product)

    manifest_liballoc = scope.registry.lookup("liballoc-blanham", type=model.Component, includeProvides=True)
    liballoc_product = builder.build(scope, manifest_liballoc)[0]
    outfile_liballoc = builder.outfile(liballoc_product)

    manifest_libcore = scope.registry.lookup("libcore", type=model.Component, includeProvides=True)
    libcore_product = builder.build(scope, manifest_libcore)[0]
    outfile_libcore = builder.outfile(libcore_product)

    manifest_libarch = scope.registry.lookup("userspace-arch", type=model.Component, includeProvides=True)
    libarch_product = builder.build(scope, manifest_libarch)[0]
    outfile_libarch = builder.outfile(libarch_product)

    
    SYSROOT_LIB_PATH = os.path.join("meta", "build", "sysroot", "lib")
    shell.cp(outfile_libc, 
             os.path.join(SYSROOT_LIB_PATH, "libc_libc.a"))
    shell.cp(outfile_iol, 
             os.path.join(SYSROOT_LIB_PATH, "libc_iol.a"))
    shell.cp(outfile_liballoc, 
             os.path.join(SYSROOT_LIB_PATH, "libc_liballoc.a"))
    shell.cp(outfile_libcore, 
             os.path.join(SYSROOT_LIB_PATH, "libc_libcore.a"))
    shell.cp(outfile_libarch, 
             os.path.join(SYSROOT_LIB_PATH, "libc_libarch.a"))
    
    shell.rmrf(os.path.join(SYSROOT_LIB_PATH, "libc.a"))
    
    # copy every file from src/libc to sysroot/lib
    LIBC_SRC_PATH = os.path.join(const.SRC_DIR, "libc")
    
    shell_run(["cp", "-r", LIBC_SRC_PATH + "/.", SYSROOT_LIB_PATH])
   
    shell_run([os.path.join("meta/build/cross/bin/x86_64-pc-wingos-ar"), "crsT", 
                 os.path.join(SYSROOT_LIB_PATH, "libc.a"), 
                 os.path.join(SYSROOT_LIB_PATH, "libc_libc.a"), 
                 os.path.join(SYSROOT_LIB_PATH, "libc_iol.a"),
                 os.path.join(SYSROOT_LIB_PATH, "libc_liballoc.a"), 
                 os.path.join(SYSROOT_LIB_PATH, "libc_libcore.a"), 
                 os.path.join(SYSROOT_LIB_PATH, "libc_libarch.a")])

    shell_run([os.path.join("meta/build/cross/bin/x86_64-pc-wingos-ranlib"), 
                 os.path.join(SYSROOT_LIB_PATH, "libc.a")])

    shell.cp(crt0_path, 
             os.path.join(SYSROOT_LIB_PATH, "crt0.o"))
    
    # list all python files in src/ports and run them and export the component 
    portsDir = os.path.join(const.SRC_DIR, "ports")
    for file in os.listdir(portsDir):
        if file.endswith(".py"):
            path = os.path.join(portsDir, file)
            vt100.title(f"Running port script: {path}")
            # get the returned component:


            spec = importlib.spec_from_file_location("plugin", path)

            if not spec or not spec.loader:
                vt100.warning(f"Cannot load port from {path}")
                return

            module = importlib.module_from_spec(spec)
            sys.modules["plugin"] = module

            try:
                spec.loader.exec_module(module)
            except Exception as e:
                vt100.warning(f"Plugin {path} loading skipped due to: {e}")

            
            if hasattr(module, "port"):
                comp = module.port()
                vt100.title(f"Built port component: {comp}")
            else:
                vt100.title(f"No 'port' function found in {path}")

