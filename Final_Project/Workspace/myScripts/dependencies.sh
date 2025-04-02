##============================================================================
## gem5 Dependencies

## make sure to maintain following versions:
# git : gem5 uses git for version control.
# gcc: gcc is used to compiled gem5. Version >=10 must be used. We support up to gcc Version 13.
# Clang: Clang can also be used. At present, we support Clang 7 to Clang 16 (inclusive).
# SCons : gem5 uses SCons as its build environment. SCons 3.0 or greater must be used.
# Python 3.6+ : gem5 relies on Python development libraries. gem5 can be compiled and run in environments using Python 3.6+.
# protobuf 2.1+ (Optional): The protobuf library is used for trace generation and playback.
# Boost (Optional): The Boost library is a set of general purpose C++ libraries. It is a necessary dependency if you wish to use the SystemC implementation.
##============================================================================

## Setup on Ubuntu 24.04 (gem5 >= v24.0)
sudo apt install build-essential scons python3-dev git pre-commit zlib1g zlib1g-dev \
    libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
    libboost-all-dev  libhdf5-serial-dev python3-pydot python3-venv python3-tk mypy \
    m4 libcapstone-dev libpng-dev libelf-dev pkg-config wget cmake doxygen rsync gnuplot

##================================================================================
##  Other Operating Systems versions
##================================================================================
## Setup on Ubuntu 22.04 (gem5 >= v21.1)
# sudo apt install build-essential git m4 scons zlib1g zlib1g-dev \
#     libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
#     python3-dev libboost-all-dev pkg-config python3-tk

## Setup on Ubuntu 20.04 (gem5 >= v21.0)
# sudo apt install build-essential git m4 scons zlib1g zlib1g-dev \
#     libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
#     python3-dev python-is-python3 libboost-all-dev pkg-config gcc-10 g++-10 \
#     python3-tk

##================================================================================
##  Dependencies for GhostMinion implementation in x86 processors
##================================================================================
sudo apt-get install gfortran

sudo apt-get install gnuplot
sudo apt-get install swig
sudo apt-get install zlib1g-dev
sudo apt-get install device-tree-compiler



