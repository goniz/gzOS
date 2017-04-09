#!/bin/bash -x

input_dir="$(realpath $1)"
output_file="$(readlink -f $2)"

cd $input_dir
find . -depth -print | cpio -H crc -o > "$output_file"
