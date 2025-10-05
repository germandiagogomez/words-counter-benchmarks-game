#!/usr/bin/env bash
set -euxo pipefail

export DEBIAN_FRONTEND=noninteractive

# function install_base_packages() {
#     if [[ $OSTYPE == 'linux'* ]]; then
#         apt update
#         apt install -y --no-install-recommends \
#             python3 python3-pip ninja-build wget \
#             git pkg-config cmake autoconf libtool
#     else
#         /usr/bin/env bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
#         brew install wget pip ninja cmake pkg-config python@3.10
#         brew postinstall python@3.10
#         brew link python@3.10
#     fi
# }

function install_conan() {
#   python3 -m pip install --user --upgrade pip setuptools
   python3 -m pip install --user conan==2.21.0
   #   ln -s /Users/germandiago/Library/Python/3.12/bin/conan /usr/local/bin/conan
#   conan --version
}

function install_meson() {
    python3 -m pip install meson==1.9.1
}

install_meson
install_conan
conan profile detect
