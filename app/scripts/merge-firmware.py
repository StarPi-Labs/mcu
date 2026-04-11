from pathlib import Path
import subprocess

Import("env")

def post_build_action(source, target, env):
    esptool = Path(env["PROJECT_CORE_DIR"]) / "penv" / "bin" / "esptool"
    build_dir = Path(env.subst("$BUILD_DIR"))
    output_image = build_dir / "qemu-image.bin"

    print("Merging firmware binary using esptool.py...")

    subprocess.run(
        [str(esptool), "--chip", "esp32s3", "merge-bin",
            "-o", str(output_image),
            "--pad-to-size", "8MB",
            "--flash-mode", "dio",
            "0x0", str(build_dir / "bootloader.bin"),
            "0x8000", str(build_dir / "partitions.bin"),
            "0x10000", str(build_dir / "firmware.bin")
        ],
        check=True
    )

    print(f"Done merging firmware binary. Output file: {output_image}")

env.AddPostAction("buildprog", post_build_action)