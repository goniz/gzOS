#!/bin/bash -e

dir=$(readlink -f $(dirname $0))
build_dir="${dir}/../build"
rootfs_dir="${build_dir}/rootfs"
userland_dir="${dir}/../userland"
apps_dir="${userland_dir}/apps"
apps_build_dir="${userland_dir}/build/apps"

apps=$(find $apps_dir/ -type d -exec basename {} \; | grep -vw apps)
echo $apps

mkdir -p "${rootfs_dir}/bin"

for app in $apps; do
	cp "${apps_build_dir}/${app}/${app}.stripped" "${rootfs_dir}/bin/${app}"
done

tree -h "${rootfs_dir}"
