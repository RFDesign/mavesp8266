#!/usr/bin/env bash

cd -- "$(dirname -- "${BASH_SOURCE[0]}")/data" || exit 1
shopt -s nullglob globstar dotglob

status=0
for archive in **/*.gz; do
    source=${archive%.gz}
    [[ -f "$archive" && -f "$source" ]] || continue

    temp=$(mktemp -- "${archive}.XXXXXX") || exit 1
    if ! gzip -c -- "$source" > "$temp" ||
        ! chmod --reference="$archive" -- "$temp" ||
        ! mv -fT -- "$temp" "$archive"; then
        rm -f -- "$temp"
        status=1
    fi
done
exit "$status"
