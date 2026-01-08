import os
import shutil

from cutekit import builder, cli, const, jexpr, model, rules, shell, vt100
from typing_extensions import Self


def kvmAvailable() -> bool:
    return os.path.exists("/dev/kvm") and os.access("/dev/kvm", os.R_OK)


class LimineCfg:
    kernel: builder.ProductScope
    pkgs: list[builder.ProductScope]
    diskPkgs: list[builder.ProductScope]
    cfg: str

    def __init__(self, kernel: str):
        print("LimineCfg")
        print(kernel)
        self.kernel = kernel
        self.pkgsBoot = []
        self.pkgsDisk = []

        self.cfg = (
            "TIMEOUT=0\n:Mu\nPROTOCOL=limine\nKERNEL_PATH=boot:///boot/kernel.elf\n"
        )
        self.imageDirBoot = None
        self.imageDirDisk = None

    def append_file_module(self, pkg: builder.ProductScope):
        if pkg != ".keep":
            self.cfg += f"MODULE_PATH=boot://{pkg}\n"

    def append_component_module(self, pkg: builder.ProductScope, is_bin=True):
        if is_bin:
            self.cfg += f"MODULE_PATH=boot:///bin/{pkg.component.id}\n"
            self.pkgsBoot.append(pkg)
        else:
            self.cfg += f"MODULE_PATH=boot://{pkg.component.id}\n"

    def append_component(self, pkg: builder.ProductScope, is_bin=True):
        self.pkgsDisk.append(pkg)

    def createImage(self) -> Self:
        self.imageDirDisk = shell.mkdir(
            os.path.join(const.PROJECT_CK_DIR, "wingos-disk")
        )

        self.imageDirBoot = shell.mkdir(
            os.path.join(const.PROJECT_CK_DIR, "wingos-boot")
        )

        efiBootDir = shell.mkdir(os.path.join(self.imageDirBoot, "EFI", "BOOT"))
        bootDir = shell.mkdir(os.path.join(self.imageDirBoot, "boot"))
        binDirBoot = shell.mkdir(os.path.join(self.imageDirBoot, "bin"))
        binDirDisk = shell.mkdir(os.path.join(self.imageDirDisk, "bin"))

        limine = shell.wget(
            "https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/BOOTX64.EFI"
        )

        shell.cp(limine, os.path.join(efiBootDir, "BOOTX64.EFI"))
        shell.cp(builder.outfile(self.kernel), os.path.join(bootDir, "kernel.elf"))

        for root, _, files in os.walk(os.path.join(const.META_DIR, "sysroot/boot")):
            root = root.replace(os.path.join(const.META_DIR, "sysroot/boot"), "")
            print(f"\n\n root: {root}\n\n")
            list(map(lambda f: self.append_file_module(os.path.join(root, f)), files))

        shell.cpTree(os.path.join(const.META_DIR, "sysroot/boot"), self.imageDirBoot)
        shell.cpTree(os.path.join(const.META_DIR, "sysroot"), self.imageDirDisk)

        list(
            map(
                lambda pkg: shell.cp(
                    builder.outfile(pkg), os.path.join(binDirBoot, pkg.component.id)
                ),
                self.pkgsBoot,
            )
        )

        list(
            map(
                lambda pkg: shell.cp(
                    builder.outfile(pkg), os.path.join(binDirDisk, pkg.component.id)
                ),
                self.pkgsDisk,
            )
        )

        with open(os.path.join(bootDir, "limine.cfg"), "w") as f:
            f.write(self.cfg)

        return self

    def createDiskImage(self) -> Self:
        if self.imageDirBoot is None:
            raise ValueError("Image directory not set. Call createImage() first.")

        self.diskImagePath = os.path.join(const.PROJECT_CK_DIR, "disk.hdd")

        print(f"Creating disk image at {self.diskImagePath}")

        shell.exec("bash", const.META_DIR + "/plugins/disk_gen.sh")

        return self

    def run(self):
        ovmf = "/usr/share/edk2/x64/OVMF.4m.fd"
        if not os.path.exists(ovmf):
            ovmf = shell.wget(
                "https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd"
            )

        self.createDiskImage()
        qemuCmd: list[str] = [
            "qemu-system-x86_64",
            "-no-reboot",
            "-no-shutdown",
            "-serial",
            "stdio",
            "-bios",
            ovmf,
            "-m",
            "4G",
            "-smp",
            "4",
            "-device",
            "nvme,drive=nvm,serial=deadbeef",
            "-drive",
            f"file={self.diskImagePath},if=none,id=nvm",
            "-boot",
            "c",
            "-s",
            "-S",
            #  f"file=fat:rw:{self.imageDir},media=disk,format=raw",
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

rules.rules["cc"].args.remove("-fcolor-diagnostics")
rules.rules["cxx"].args.remove("-fcolor-diagnostics")
rules.rules["cxx"].args.remove("-fmodules-reduced-bmi")


@jexpr.exposed("wingos.target")
def wingos_target() -> str:
    # out the current building target
    # query the ENV

    target = os.getenv("CK_TARGET", "wingos-x86_64")
    vt100.title(f"Wingos target: {target}")

    # print all env

    return target


def build_object(args: model.TargetArgs, component, target):
    vt100.title(f"building: {component} with target {target}")

    if target != "":
        args.target = target

    scope = builder.TargetScope.use(args)

    registry = model.Registry.use(args)
    # components = list(registry.iter(model.Component))
    #    targets = builder.TargetScope(args)
    fcomponent = scope.registry.lookup(
        component, type=(model.Component, model.Target), includeProvides=True
    )

    if fcomponent is None:
        raise ValueError(f"Component '{component}' not found")

    product = builder.build(scope, fcomponent)[0]

    return product


@cli.command("s", "Boot Wingos")
def bootFunc(args: model.TargetArgs):
    scope = builder.TargetScope.use(args)
    registry = model.Registry.use(args)
    components = list(registry.iter(model.Component))

    limine = LimineCfg(build_object(args, "kernel-loader-limine", "kernel-x86_64"))

    targets = list(registry.iter(model.Target))

    vt100.title("Components")
    vt100.p(f"{components}")
    for pkg in filter(lambda m: const.EXTERN_DIR not in m.dirname(), components):
        if (
            pkg.type == model.Kind.EXE
            and pkg.id != "kernel-loader-limine"
            and pkg.props.get("exported", "") == "module"
        ):
            obj = build_object(args, pkg.id, "wingos-x86_64")
            limine.append_component_module(obj)
            limine.append_component(obj)
        if (
            pkg.type == model.Kind.EXE
            and pkg.id != "kernel-loader-limine"
            and pkg.props.get("exported", "") == "disk"
        ):
            obj = build_object(args, pkg.id, "wingos-x86_64")
            limine.append_component(obj)
    limine.createImage().run()


# cmds.append(
#    cmds.Cmd("s", "boot", "Boot Wingos", bootFunc)
# )
