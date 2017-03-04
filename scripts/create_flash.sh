#!/bin/bash -x

input_dir="$1"
output_file="$2"
build_dir="${3-build}"

mkdir -p "$build_dir"

# create the yamon reserved partition space
part_yamon="${build_dir}/part_yamon"
dd if=/dev/zero of="$part_yamon" bs=1M count=1

# create the userdata partition
part_userdata="${build_dir}/part_userdata"
userdata_mnt="${build_dir}/mnt"
dd if=/dev/zero of="$part_userdata" bs=1M count=3
#mkfs.ext2 -L userdata -I 128 -r 0 "$part_userdata"
sudo mkfs.vfat -n userdata "$part_userdata"

mkdir -p "${userdata_mnt}"
sudo mount -o loop "$part_userdata" "$userdata_mnt"
sudo rsync -a "${input_dir}/" "${userdata_mnt}/"
sudo umount "${userdata_mnt}"

# pack the parts into the final image
cat "$part_yamon" "$part_userdata" > "$output_file"
