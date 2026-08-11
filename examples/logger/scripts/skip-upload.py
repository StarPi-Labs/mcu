Import("env")
from sys import stderr

def skip_upload(source, target, env):
    print("Skipping upload step for qemu environment.", file=stderr)
    env.Exit(0)

env.AddPreAction("upload", skip_upload)