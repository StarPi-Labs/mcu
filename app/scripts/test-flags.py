Import("env")

from sys import stderr

# Get the name of the test currently being built (e.g., "core/test_logging")
test_name = env.get("PIOTEST_RUNNING_NAME", "")

# Check if the currently running test matches our target test
if test_name == "core/test_logging":
    print(f"Disabling log timestamps for test: {test_name}", file=stderr)
    env.Append(CPPDEFINES=[("MCU_LOG_TIMESTAMP_ENABLE", "0")])