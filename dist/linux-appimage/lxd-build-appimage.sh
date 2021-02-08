#!/bin/bash

###############################################################
# Uses LXD to create an Ubuntu Xenial container and produce   #
# a reasonably portable AppImage of PrimeWorldEditor.         #
###############################################################

set -e

CMAKE_VERSION=3.15.5
CONTAINER_NAME=pwe-ci

# Set up container, deleting existing if necessary
if lxc info $CONTAINER_NAME >& /dev/null
then
    lxc delete $CONTAINER_NAME --force
fi
lxc init ubuntu:16.04 $CONTAINER_NAME

# Inject build script
lxc file push - $CONTAINER_NAME/root/dobuild.sh <<END
#!/bin/bash
set -e

# PWE build script for Ubuntu 16.04 LTS (Xenial)

# Install build dependencies
apt update
apt -y install build-essential software-properties-common python-software-properties
add-apt-repository -y ppa:ubuntu-toolchain-r/test
add-apt-repository -y ppa:deadsnakes/ppa
add-apt-repository -y ppa:beineri/opt-qt-5.12.3-xenial
apt update
apt -y install g++-8 curl git ninja-build libclang-6.0-dev python3.6 python3-pip zlib1g-dev qt512tools qt512svg libglu1-mesa-dev

# Expose Qt 5.12
export PATH=$PATH:/bin:/opt/qt512/bin

# Doing everything in root is fine
cd /

# Install recent CMake
curl -OL https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-Linux-x86_64.sh
sh cmake-$CMAKE_VERSION-Linux-x86_64.sh --prefix=/usr/local --exclude-subdir

# Get linuxdeployqt
curl -OL https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt-continuous-x86_64.AppImage
/linuxdeployqt-continuous-x86_64.AppImage --appimage-extract

# Cleanup
rm -rf PrimeWorldEditor{-build,-appdir,}

# Clone repository
git clone https://github.com/AxioDL/PrimeWorldEditor
pushd PrimeWorldEditor
git submodule update --recursive --init
popd

# Build
mkdir -p PrimeWorldEditor{-build,-appdir}
pushd PrimeWorldEditor-build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc-8 -DCMAKE_CXX_COMPILER=g++-8 \
              -DPWE_PUBLIC_RELEASE=On -DCMAKE_INSTALL_PREFIX=/PrimeWorldEditor-appdir/usr \
              -DCMAKE_EXECUTE_PROCESS_COMMAND_ECHO=STDERR /PrimeWorldEditor
ninja install
popd

strip -s /PrimeWorldEditor-appdir/usr/bin/PrimeWorldEditor
cp PrimeWorldEditor-appdir/usr/share/icons/hicolor/256x256/apps/PrimeWorldEditor.png PrimeWorldEditor-appdir/
cp PrimeWorldEditor-appdir/usr/share/applications/io.github.arukibree.PrimeWorldEditor.desktop PrimeWorldEditor-appdir/
/squashfs-root/usr/bin/linuxdeployqt /PrimeWorldEditor-appdir/usr/bin/PrimeWorldEditor -appimage
END

# Start container
lxc start $CONTAINER_NAME

# Wait for network
lxc exec $CONTAINER_NAME -- bash -c "while ! systemctl status network.target; do sleep 1; done"

# Run build script
lxc exec $CONTAINER_NAME -t -- bash /root/dobuild.sh

# Retrieve AppImage 
lxc file pull $CONTAINER_NAME/Prime_World_Editor-x86_64.AppImage .

# Cleanup
lxc delete $CONTAINER_NAME --force
