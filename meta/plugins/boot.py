
import os
import shutil

from cutekit import builder,  const, shell, model, cli, vt100
from typing_extensions import Self


def kvmAvailable() -> bool:
    return os.path.exists("/dev/kvm") and os.access("/dev/kvm", os.R_OK)

class LimineCfg:
    kernel: builder.ProductScope
    pkgs: list[builder.ProductScope]
    cfg: str

    def __init__(self, kernel: str):
        print("LimineCfg")
        print(kernel)
        self.kernel = kernel
        self.pkgs = []
        self.cfg = "TIMEOUT=0\n:Mu\nPROTOCOL=limine\nKERNEL_PATH=boot:///boot/kernel.elf\n"
        self.imageDir = None

    def append_file(self, pkg: builder.ProductScope):
        if pkg != '.keep':
            self.cfg += f"MODULE_PATH=boot://{pkg}\n"

    def append_component(self, pkg: builder.ProductScope, is_bin=True):
        if is_bin:
            self.cfg += f"MODULE_PATH=boot:///bin/{pkg.component.id}\n"
            self.pkgs.append(pkg)
        else:
            self.cfg += f"MODULE_PATH=boot://{pkg.component.id}\n"

    def createImage(self) -> Self:
        self.imageDir = shell.mkdir(
            os.path.join(const.PROJECT_CK_DIR, "wingos"))
        efiBootDir = shell.mkdir(os.path.join(self.imageDir, "EFI", "BOOT"))
        bootDir = shell.mkdir(os.path.join(self.imageDir, "boot"))
        binDir = shell.mkdir(os.path.join(self.imageDir, "bin"))

        limine = shell.wget(
            "https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/BOOTX64.EFI"
        )

        shell.cp(limine, os.path.join(efiBootDir, "BOOTX64.EFI"))
        shell.cp(builder.outfile(self.kernel), os.path.join(bootDir, "kernel.elf"))

        for root, _, files in os.walk(os.path.join(const.META_DIR, "sysroot")): 
            root = root.replace(os.path.join(const.META_DIR, "sysroot"), "")
            print(f"\n\n root: {root}\n\n")
            list(map(lambda f: self.append_file(os.path.join(root, f)), files))

        shell.cpTree(os.path.join(const.META_DIR, "sysroot"), self.imageDir)

        list(map(lambda pkg: shell.cp(builder.outfile(pkg),
             os.path.join(binDir, pkg.component.id)), self.pkgs))

        with open(os.path.join(bootDir, "limine.cfg"), 'w') as f:
            f.write(self.cfg)

        return self

    def run(self):
        ovmf = shell.wget(
            "https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd")

        qemuCmd: list[str] = [
            "qemu-system-x86_64",
            "-no-reboot",
            "-no-shutdown",
            "-serial", "stdio",
            "-bios", ovmf,
            "-m", "4G",
            "-smp", "4",
            "-drive",

            f"file=fat:rw:{self.imageDir},media=disk,format=raw",
          #  "-d", "cpu_reset",
           # "-d", "guest_errors",
          #  "-d", "int"
        ]

        if kvmAvailable():
            qemuCmd += ["-enable-kvm", "-cpu", "host"]

        shell.exec(*qemuCmd)

"""

def _(args: TargetArgs):

    vt100.title("Components")
    if len(components) == 0:
        print(vt100.p("(No components available)"))
    else:
        print(vt100.p(", ".join(map(lambda m: m.id, components))))
    print()

    vt100.title("Targets")

    if len(targets) == 0:
        print(vt100.p("(No targets available)"))
    else:
        print(vt100.p(", ".join(map(lambda m: m.id, targets))))
    print()
"""


def build_object(args: model.TargetArgs, component, target):

    vt100.title(f"building: {component} with target {target}")

    if target != "": 
        args.target = target

    scope = builder.TargetScope.use(args)
    registry = model.Registry.use(args)
   # components = list(registry.iter(model.Component))
#    targets = builder.TargetScope(args)
    fcomponent = scope.registry.lookup(
        component, type=(model.Component, model.Target), 
        includeProvides=True
    )


    if fcomponent is None:
        raise ValueError(f"Component '{component}' not found")
    
    product = builder.build(scope, fcomponent)[0]
    
    return product


@cli.command("s", "boot", "Boot Wingos")
def bootFunc(args: model.TargetArgs):
    
    scope = builder.TargetScope.use(args)
    registry = model.Registry.use(args)
    components = list(registry.iter(model.Component))
    
    limine = LimineCfg(build_object(args, "kernel-loader-limine", "kernel-x86_64"))

    targets = list(registry.iter(model.Target))

    vt100.title("Components")
    vt100.p(f'{components}')
    for pkg in filter(lambda m: const.EXTERN_DIR not in m.dirname(), components):
        if pkg.type == model.Kind.EXE and pkg.id != "kernel-loader-limine" and pkg.props.get("exported", False):
            limine.append_component(build_object(args, pkg.id, "wingos-x86_64"))

    limine.createImage().run()


#cmds.append(
#    cmds.Cmd("s", "boot", "Boot Wingos", bootFunc)
#)
