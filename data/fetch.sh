#!/usr/bin/bash

cd "$(cd "$(dirname "${BASH_SOURCE[0]}" )" > /dev/null 2>&1 && pwd)"

function fetch_file() {
    local frame_local="$1"
    local frame_bbb=$(($1+5580))
    local fname="$(printf "img_%04d.png" "$frame_local")"

    if [ ! -e "$fname" ]; then
        wget --quiet \
             "https://media.xiph.org/BBB/BBB-360-png/big_buck_bunny_0${frame_bbb}.png" \
             -O "$fname"
        printf .
    fi
}

export -f fetch_file
seq 0 100 | tr '\n' '\0' | xargs -0 -P16 -I{} bash -c 'fetch_file "$@"' _ {}
echo
