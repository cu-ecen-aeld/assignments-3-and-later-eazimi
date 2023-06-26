#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
INTRPRTR="/var/lib/docker/overlay2/e1b53b70d0542d3203c44d2175bb8e367a0c6f304ecfef506b0f183092f72ca8/diff/usr/local/arm-cross-compiler/install/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib/ld-linux-aarch64.so.1"
LIBRESOLV="/var/lib/docker/overlay2/acb8accd7a02a0531c8bb2cf363509d577334b7fc1627315e71ffc3c7b74e8dc/diff/usr/lib/x86_64-linux-gnu/libresolv.so.2"
LIBC="/var/lib/docker/overlay2/acb8accd7a02a0531c8bb2cf363509d577334b7fc1627315e71ffc3c7b74e8dc/diff/usr/lib/x86_64-linux-gnu/libc.so.6"
LIBM="/var/lib/docker/overlay2/acb8accd7a02a0531c8bb2cf363509d577334b7fc1627315e71ffc3c7b74e8dc/diff/usr/lib/x86_64-linux-gnu/libm.so.6"
CUR_DIR=$PWD

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    
    # TODO: Add your kernel build steps here
    echo "start kernel build ..."    
    make mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
cp -r ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}
if [ -e ${OUTDIR}/Image ]; then
    echo "Image files copied to ${OUTDIR}/Image"
else
    echo "couldn't copy image files into ${OUTDIR}/Image"
    exit 1
fi

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
# if [ -d "${OUTDIR}/rootfs" ]
# then
# 	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
#     sudo rm  -rf ${OUTDIR}/rootfs
# fi

# TODO: Create necessary base directories
ROOTFS=${OUTDIR}/rootfs
mkdir -p ${ROOTFS}
cd ${ROOTFS}
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr/bin usr/lib usr/sbin var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]; then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
# make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
# make CONFIG_PREFIX=${ROOTFS} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

PROG_INTERPRETER=$(${CROSS_COMPILE}readelf -a ${ROOTFS}/bin/busybox | grep "program interpreter")
PROG_INTERPRETER=$(echo ${PROG_INTERPRETER} | sed 's/.*\/\(.*\)\]/\1/')
echo -e "program interpreter:\n${PROG_INTERPRETER}"
SHARED_LIB=$(${CROSS_COMPILE}readelf -a ${ROOTFS}/bin/busybox | grep "Shared library")
echo "Library dependencies:"
while IFS= read -r line;do
    echo ${line} | sed 's/.*\[\(.*\)\]/\1/'
done <<< ${SHARED_LIB}

# TODO: Add library dependencies to rootfs
if [ -f ${ROOTFS}/lib/${PROG_INTERPRETER} ]; then
    rm -f ${ROOTFS}/lib/${PROG_INTERPRETER}
fi
sudo cp ${INTRPRTR} ${ROOTFS}/lib
rm -f ${ROOTFS}/lib64/*.*
sudo cp ${LIBRESOLV} ${ROOTFS}/lib64/
sudo cp ${LIBC} ${ROOTFS}/lib64/ 
sudo cp ${LIBM} ${ROOTFS}/lib64/

# TODO: Make device nodes
rm -f ${ROOTFS}/dev/null
rm -f ${ROOTFS}/dev/console
sudo mknod -m 666 ${ROOTFS}/dev/null c 1 3
sudo mknod -m 666 ${ROOTFS}/dev/console c 5 1 

# TODO: Clean and build the writer utility
cd ${CUR_DIR}
make clean
make CROSS_COMPILE=${CROSS_COMPILE} 
sudo cp writer ${ROOTFS}/home

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs

# TODO: Chown the root directory

# TODO: Create initramfs.cpio.gz
