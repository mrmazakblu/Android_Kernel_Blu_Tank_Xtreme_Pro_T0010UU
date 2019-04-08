#!/bin/bash
################################################################################
#
#  build_kernel.sh
#
#  Hacked From Amazon Original Script
#
################################################################################

################################################################################
#
#  V A R I A B L E S
#
################################################################################

# Retrieve the directory where the script is currently held
SCRIPT_BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Configuration file for the build.
CONFIG_FILE="${SCRIPT_BASE_DIR}/build_kernel_config.sh"

# Prebuilts include folder
PREBUILTS_FOLDER="${SCRIPT_BASE_DIR}/prebuilt"
mkdir -p "${PREBUILTS_FOLDER}"

# Workspace directory & relevant work folders.
WORKSPACE_DIR="${SCRIPT_BASE_DIR}/workspace"
mkdir -p "${WORKSPACE_DIR}"
TOOLCHAIN_DIR="${WORKSPACE_DIR}/toolchain"
PLATFORM_EXTRACT_DIR="${SCRIPT_BASE_DIR}"
WORKSPACE_OUT_DIR="${WORKSPACE_DIR}/out"
TARGET_DIR="${WORKSPACE_DIR}/out-cleaned"
ERROR_DIR="${WORKSPACE_DIR}/error"
for d in "${TOOLCHAIN_DIR}" "${WORKSPACE_OUT_DIR}" "${TARGET_DIR}" "${ERROR_DIR}"
do
    mkdir -p "${d}"
done

PARALLEL_EXECUTION="-j3"

function pause(){
   read -p "$*"
}

function usage {
    echo "Usage: ${BASH_SOURCE[0]} path_to_platform_tar output_folder" 1>&2
    exit 1
}

function validate_input_params {

    if [[ ! -f "${CONFIG_FILE}" ]]
    then
        echo "ERROR: Could not find config file ${CONFIG_FILE}. Please check" \
             "that you have extracted the build script properly and try again."
        usage
    fi

    if [[ ! -d "${PREBUILTS_FOLDER}" ]]
    then
        echo "ERROR: Missing prebuilts folder ${PREBUILTS_FOLDER}"
        usage
    fi
}

function display_config {
    echo "-------------------------------------------------------------------------"
    echo "SOURCE TARBALL: ${PLATFORM_TARBALL}"
    echo "TARGET DIRECTORY: ${TARGET_DIR}"
    echo "KERNEL SUBPATH: ${KERNEL_SUBPATH}"
    echo "DEFINITION CONFIG: ${DEFCONFIG_NAME}"
    echo "TARGET ARCHITECTURE: ${TARGET_ARCH}"
    echo "TOOLCHAIN REPO: ${TOOLCHAIN_REPO}"
    echo "TOOLCHAIN PREFIX: ${TOOLCHAIN_PREFIX}"
    echo "-------------------------------------------------------------------------"
    echo "Sleeping 6 seconds before continuing."
    sleep 6
}

function setup_output_dir {
    if [[ -d "${WORKSPACE_OUT_DIR}" ]]
    then
        FILECOUNT=$(find "${WORKSPACE_OUT_DIR}" -type f | wc -l)
        if [[ ${FILECOUNT} -gt 0 ]]
        then
    	    echo "-------------------------------------------------------------------------"
            echo "ERROR: Destination folder is not empty. Not Recomend to build" \
                 "to a non-clean target"
    	    echo "Sleeping 10 seconds before continuing."
    	    echo "ctrl-c now to quit or continueing will clean the output folder"
            sleep 10
            echo "Make clean"
            #make clean O=${WORKSPACE_OUT_DIR} 
            # comment out next 2 lines to skip folder delete
            rm -rf ${WORKSPACE_OUT_DIR}
            mkdir -p "${WORKSPACE_OUT_DIR}"
    	    echo "-------------------------------------------------------------------------"
        fi
    else
        echo "Making target directory ${WORKSPACE_OUT_DIR}"
        mkdir -p "${WORKSPACE_OUT_DIR}"

        if [[ $? -ne 0 ]]
        then
            echo "ERROR: Could not make target directory ${WORKSPACE_OUT_DIR}"
            exit 1
        fi
    fi
}

function download_toolchain {
    if [[ -d "${TOOLCHAIN_DIR}" ]]
    then
        FILECOUNT=$(find "${TOOLCHAIN_DIR}" -type f | wc -l)
        if [[ ${FILECOUNT} -gt 0 ]]
        then
            echo "TOOLCHAIN DIRECTORY NOT EMPTY< NO NEED TO DOWNLOAD AGAIN"
        else
            echo "Cloning toolchain ${TOOLCHAIN_REPO} to ${TOOLCHAIN_DIR}"
            git clone "${TOOLCHAIN_REPO}" "${TOOLCHAIN_DIR}"
        fi
    fi
}

function exec_build_kernel {
    CCOMPILE="${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}"

    if [[ -n "${KERNEL_SUBPATH}" ]]
    then
        MAKE_ARGS="-C ${KERNEL_SUBPATH}"
    fi

    MAKE_ARGS="${MAKE_ARGS} O=${WORKSPACE_OUT_DIR} ARCH=${TARGET_ARCH} CROSS_COMPILE=${CCOMPILE}"
    echo "Base make args: ${MAKE_ARGS}"

    # Move into the build base folder.
    pushd "${PLATFORM_EXTRACT_DIR}"
    echo "Retreiving Subversion Number"
    subversion="$(sed '3q;d' Makefile )"

    # Step 1: defconfig
    echo "Make defconfig: make ${MAKE_ARGS} ${DEFCONFIG_NAME}"
    make ${MAKE_ARGS} ${DEFCONFIG_NAME}

    # Step 2: check trapz:
    local KERNEL_FOLDER="${PLATFORM_EXTRACT_DIR}/${KERNEL_SUBPATH}"
    local TRAPZ_CFG="${KERNEL_FOLDER}/arch/${TARGET_ARCH}/configs/trapz.config"
    local OUTPUT_CFG="${WORKSPACE_OUT_DIR}/.config"

    if [[ -f $TRAPZ_CFG ]]
    then
        echo "Adding trapz config to config"
        cat "${TRAPZ_CFG}" >> "${OUTPUT_CFG}"
    else
        echo "No trapz config found."
    fi

    # Step 3: run oldconfig
    echo "Make oldconfig: make ${MAKE_ARGS} oldconfig"
    make ${MAKE_ARGS} oldconfig

    # Step 3A: output config, for reference
    echo ".config contents"
    echo "---------------------------------------------------------------------"
    cat "${OUTPUT_CFG}"
    echo "---------------------------------------------------------------------"

    # Step 4: Make headers install
    echo "Make headers install: make ${MAKE_ARGS} headers_install"
    make ${MAKE_ARGS} headers_install

    # Step 5: install prebuilt headers
    if [[ -d "${PREBUILTS_FOLDER}" ]]
    then
        echo "Copying prebuilts to ${WORKSPACE_OUT_DIR}"
        cp -av ${PREBUILTS_FOLDER}/* "${WORKSPACE_OUT_DIR}"
    fi

    # Step 6: dtbs
    if [[ -n "${MAKE_DTBS}" ]]
    then
        echo "Make dtbs: make ${MAKE_ARGS} dtbs"
        make ${MAKE_ARGS} dtbs
    fi

    # Step 7: full make
    echo "Running full make"
    if [[ -n "${ZIMAGE_TARGET}" ]]
    then
        make ${MAKE_ARGS} ${PARALLEL_EXECUTION} USE_TRAPZ=true zImage 2> "${ERROR_DIR}/${subversion}_error.txt"
    else
        make ${MAKE_ARGS} ${PARALLEL_EXECUTION} USE_TRAPZ=true 2> "${ERROR_DIR}/${subversion}_error.txt"
    fi

    popd
}

function copy_to_output {
    echo "Copying files to output"
    pushd "${WORKSPACE_OUT_DIR}"
    find "./arch/"${TARGET_ARCH}"/boot" -type f | sed 's/^\.\///' | while read CPFILE
    do
        local BASEDIR="$(dirname "${CPFILE}")"
        if [[ ! -d "${TARGET_DIR}/${BASEDIR}" ]]
        then
            mkdir -p "${TARGET_DIR}/${BASEDIR}"
        fi
        cp -v "${CPFILE}" "${TARGET_DIR}/${CPFILE}_${subversion}"
    done
    popd
}

################################################################################
#
#  M A I N
#
################################################################################

# Phase 1: Set up execution
validate_input_params
source "${CONFIG_FILE}"
setup_output_dir
TARGET_DIR="$(cd "${TARGET_DIR}" && pwd)"
display_config

# Phase 2: Set up environment
download_toolchain

# Phase 3: build
exec_build_kernel

# Phase 4: move to output
copy_to_output
