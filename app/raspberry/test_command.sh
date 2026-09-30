#!/bin/bash
# Test script to send commands to the StarPi command socket

SOCKET="/tmp/starpi_cmd.sock"

if [ ! -S "$SOCKET" ]; then
    echo "Socket $SOCKET not found. Is radio_app running?"
    exit 1
fi

# Send a command - CMD_EJECT_A (1) with data=0
echo '{"command": 1, "data": 0}' | socat - UNIX-CONNECT:$SOCKET

# Send a command - CMD_EJECT_C (2) with data=12345
echo '{"command": 2, "data": 12345}' | socat - UNIX-CONNECT:$SOCKET

# Send a command - CMD_CUT_MAIN (3) with data=999
echo '{"command": 3, "data": 999}' | socat - UNIX-CONNECT:$SOCKET

echo "Commands sent"