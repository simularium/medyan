#!/bin/sh -e

if [ -z $medyan_root_dir ]; then
    echo "Error: variable medyan_root_dir needs to be specified"
    exit 1
fi

# Set directories
medyan_build_dir="$medyan_root_dir/build"
script_dir="$medyan_root_dir/scripts"
build_dir="$script_dir/.build"
vcpkg_dir="$build_dir/vcpkg"

# Set variables
medyan_vcpkg_cmake_toolchain="$medyan_root_dir/scripts/.build/vcpkg/scripts/buildsystems/vcpkg.cmake"

if [ "$MEDYAN_BOOST_INSTALL_MODE" = "manual" ]; then
    medyan_cmake_boost="-DMEDYAN_BOOST_INSTALL_MODE=manual"
    medyan_cmake_boost="$medyan_cmake_boost -DMEDYAN_BOOST_INCLUDE_DIR=$MEDYAN_BOOST_INCLUDE_DIR"
    medyan_cmake_boost="$medyan_cmake_boost -DMEDYAN_BOOST_LIBRARY_DIR=$MEDYAN_BOOST_LIBRARY_DIR"
    medyan_cmake_boost="$medyan_cmake_boost -DMEDYAN_BOOST_LIBRARIES=$MEDYAN_BOOST_LIBRARIES"
    medyan_need_install_boost=false
elif [ "$MEDYAN_BOOST_INSTALL_MODE" = "find" ]; then
    medyan_cmake_boost="-DMEDYAN_BOOST_INSTALL_MODE=find"
    medyan_need_install_boost=false
else
    medyan_need_install_boost=true
fi

if [ -n "$MEDYAN_ADDITIONAL_LINK_DIRS" ]; then
    medyan_cmake_additional_link_dirs="-DMEDYAN_ADDITIONAL_LINK_DIRS=$MEDYAN_ADDITIONAL_LINK_DIRS"
fi

if [ -n "$MEDYAN_RPATH" ]; then
    medyan_cmake_rpath="-DMEDYAN_RPATH=$MEDYAN_RPATH"
fi

if [ "$MEDYAN_BUILD_TOOL" = "Xcode" ]; then
    medyan_cmake_target="Xcode"
else
    medyan_cmake_target="default"
fi


# Download and setup vcpkg
vcpkg_setup() {
    required=$1
    rebuild=$2
    if [ $required = true ]; then
        if [ ! -d "$vcpkg_dir" -o $rebuild = true ]; then
            echo "Configuring vcpkg..."
            (
                cd $build_dir &&
                git clone https://github.com/Microsoft/vcpkg.git &&
                cd $vcpkg_dir &&
                ./bootstrap-vcpkg.sh
            )
        else
            echo "vcpkg is already installed."
        fi
    else
        echo "Skipping vcpkg installation."
    fi
    return 0
}

# Setup dependencies
vcpkg_install() {
    need_boost=$1

    (
        cd $vcpkg_dir && {
            ./vcpkg install catch2 eigen3 spectra
            if [ "$need_boost" = true ]; then
                ./vcpkg install boost
            fi
        }
    )
}

# Generate using CMake
cmake_generate() {
    target=$1

    if [ "$target" = "Xcode" ]; then
        medyan_cmake_generator="-G Xcode"
    fi

    echo "Creating build files..."
    (
        mkdir -p $medyan_build_dir &&
        cd $medyan_build_dir &&
        cmake \
            $medyan_cmake_generator \
            $medyan_cmake_boost \
            $medyan_cmake_additional_link_dirs \
            $medyan_cmake_rpath \
            .. "-DCMAKE_TOOLCHAIN_FILE=$medyan_vcpkg_cmake_toolchain"
    )
}

# Make directories
mkdir -p $build_dir

# Use vcpkg to resolve dependencies
vcpkg_setup true false
vcpkg_install $medyan_need_install_boost

# Use CMake to generate build files
cmake_generate $medyan_cmake_target