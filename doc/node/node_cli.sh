if [[ -n "$1" ]]; then
    export HOST="$1"
fi
if [[ -n "$2" ]]; then
    export SERVICE="$2"
fi

node_cli() {
    local HOST="${HOST:-127.0.0.1}"
    local SERVICE="${SERVICE:-12345}"

    if [[ "$(uname -s)" == "Linux" ]]; then
        ND_CLI="./sysroot/linux/x86_64/bin/xtools node_cli"
    elif [[ "$(uname -s)" == "MINGW"* || "$(uname -s)" == "CYGWIN"* || "$(uname -s)" == "MSYS"* ]]; then
        ND_CLI="./sysroot/windows/x86_64/bin/xtools.exe node_cli"
    else
        echo "Unsupported OS: $(uname -s)"
        return 1
    fi

    $ND_CLI --host="$HOST" --service="$SERVICE" "$@"
}