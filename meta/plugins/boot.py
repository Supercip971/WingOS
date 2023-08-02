
import os
import shutil
from cutekit import args, cmds, builder, context, const, shell, model
from typing import Self


def kvmAvailable() -> bool:
    return os.path.exists("/dev/kvm") and os.access("/dev/kvm", os.R_OK)

class LimineCfg:
    def __init__(self, kernel: context.ComponentInstance):
        self.kernel = kernel
        self.pkgs: list[context.ComponentInstance] = []
        self.cfg = "TIMEOUT=0\n:Mu\nPROTOCOL=limine\nKERNEL_PATH=boot:///boot/kernel.elf\n"
        self.imageDir = None

    def append(self, pkg: context.ComponentInstance, is_bin=True):
        if is_bin:
            self.cfg += f"MODULE_PATH=boot:///bin/{pkg.id()}\n"
            self.pkgs.append(pkg)
        else:
            self.cfg += f"MODULE_PATH=boot://{pkg}\n"

    def createImage(self) -> Self:
        self.imageDir = shell.mkdir(
            os.path.join(const.PROJECT_CK_DIR, "munix"))
        efiBootDir = shell.mkdir(os.path.join(self.imageDir, "EFI", "BOOT"))
        bootDir = shell.mkdir(os.path.join(self.imageDir, "boot"))
        binDir = shell.mkdir(os.path.join(self.imageDir, "bin"))

        limine = shell.wget(
            "https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/BOOTX64.EFI"
        )

        shell.cp(limine, os.path.join(efiBootDir, "BOOTX64.EFI"))
        shell.cp(self.kernel.outfile(), os.path.join(bootDir, "kernel.elf"))

        for root, _, files in os.walk(os.path.join(const.META_DIR, "sysroot")): 
            root = root.replace(os.path.join(const.META_DIR, "sysroot"), "")
            list(map(lambda f: self.append(os.path.join(root, f), False), files))

        shell.cpTree(os.path.join(const.META_DIR, "sysroot"), self.imageDir)

        list(map(lambda pkg: shell.cp(pkg.outfile(),
             os.path.join(binDir, pkg.id())), self.pkgs))

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
        ]

        if kvmAvailable():
            qemuCmd += ["-enable-kvm", "-cpu", "host"]

        shell.exec(*qemuCmd)


def bootFunc(args: args.Args):
    limine = LimineCfg(builder.build("kernel-loader-limine", "kernel-x86_64"))

    for pkg in filter(lambda m: const.EXTERN_DIR not in m.dirname(), context.loadAllComponents()):
        if pkg.type == model.Type.EXE and pkg.id != "kernel-loader-limine":
            limine.append(builder.build(pkg.id, "wingos-x86_64"))

    limine.createImage().run()


cmds.append(
    cmds.Cmd("s", "boot", "Boot Wingos", bootFunc)
)