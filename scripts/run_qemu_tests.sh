#!/bin/bash -xe

dir=$(readlink -f $(dirname $0))

${dir}/run_qemu_gzos.sh ${dir}/../kernel/build/tests.elf
