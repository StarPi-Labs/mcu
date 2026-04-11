Import("env")
import platform
import shutil
import tarfile
from pathlib import Path
from urllib.error import URLError
from urllib.request import urlopen

base_url = "https://github.com/espressif/qemu/releases/download/"
version = "esp-develop-9.2.2-20250817"
bin_prefix = "qemu-xtensa-softmmu-" + version.replace("-", "_") + "-"
# https://github.com/espressif/qemu/releases/download/esp-develop-9.2.2-20250228/qemu-xtensa-softmmu-esp_develop_9.2.2_20250228-x86_64-linux-gnu.tar.xz

install_dir = Path(env.subst("$PROJECT_DIR")) / "qemu"
install_dir.mkdir(exist_ok=True)

def normalized_os_name() -> str:
    system_name = platform.system().lower()
    if system_name == "linux":
        return "linux-gnu"
    if system_name == "darwin":
        return "apple-darwin"
    if system_name == "windows":
        return "w64-mingw32"
    raise RuntimeError(f"Unsupported host OS: {platform.system()}")


def normalized_arch_name() -> str:
    machine_name = platform.machine().lower()
    if machine_name in {"x86_64", "amd64"}:
        return "x86_64"
    if machine_name in {"aarch64", "arm64"}:
        return "aarch64"
    raise RuntimeError(f"Unsupported host architecture: {platform.machine()}")

def expected_archive_name() -> str:
    return f"{bin_prefix}{normalized_arch_name()}-{normalized_os_name()}.tar.xz"

def expected_binary() -> Path | None:
    for candidate in install_dir.rglob("qemu-system-xtensa*"):
        if candidate.is_file():
            return candidate
    return None

def safe_extract(archive: tarfile.TarFile, destination: Path) -> None:
    destination_root = destination.resolve()
    for member in archive.getmembers():
        member_path = destination / member.name
        resolved_member_path = member_path.resolve()
        if destination_root not in resolved_member_path.parents and resolved_member_path != destination_root:
            raise RuntimeError(f"Unsafe path detected in archive member: {member.name}")
    archive.extractall(destination)

binary = expected_binary()
if binary is not None:
    print(f"QEMU already installed at {binary}")
else:
    archive_name = expected_archive_name()
    download_url = f"{base_url}{version}/{archive_name}"
    archive_path = install_dir / archive_name

    print(f"Downloading QEMU from {download_url}")

    try:
        with urlopen(download_url) as response, open(archive_path, "wb") as archive_file:
            shutil.copyfileobj(response, archive_file)
    except URLError as error:
        raise RuntimeError(f"Failed to download QEMU archive: {download_url}") from error

    print(f"Extracting {archive_name} into {install_dir}")
    with tarfile.open(archive_path, mode="r:xz") as archive:
        safe_extract(archive, install_dir)

    archive_path.unlink(missing_ok=True)
