#!/usr/bin/env bash

cd -- "$(dirname -- "${BASH_SOURCE[0]}")/data" || exit 1
shopt -s nullglob globstar dotglob

status=0
for archive in **/*.gz; do
    [[ -f "$archive" ]] || continue
    target=${archive%.gz}
    if [[ -e "$target" || -L "$target" ]]; then
        read -r -p "Overwrite $target? [y/N] " answer || continue
        [[ "$answer" == [yY] || "$answer" == [yY][eE][sS] ]] || continue
    fi

    temp=$(mktemp -- "${target}.XXXXXX") || exit 1
    if ! gzip -dc -- "$archive" > "$temp" ||
        ! chmod --reference="$archive" -- "$temp" ||
        ! mv -fT -- "$temp" "$target"; then
        rm -f -- "$temp"
        status=1
    fi
done
exit "$status"
