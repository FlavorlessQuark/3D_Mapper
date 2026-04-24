#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 input_file"
    exit 1
fi

file="$1"

echo "float data[][3] = {"

first=true

while IFS=',' read -r x y z; do
    # remove Windows carriage return (\r)
    x=${x//$'\r'/}
    y=${y//$'\r'/}
    z=${z//$'\r'/}

    # trim whitespace
    x=$(echo "$x" | xargs)

    # skip empty or comment lines
    if [[ -z "$x" || "$x" == \#* ]]; then
        continue
    fi

    y=$(echo "$y" | xargs)
    z=$(echo "$z" | xargs)

    if [ "$first" = true ]; then
        first=false
        printf "    {%s,%s,%s}\n" "$x" "$y" "$z"
    else
        printf "    ,{%s,%s,%s}\n" "$x" "$y" "$z"
    fi

done < "$file"

echo "};"