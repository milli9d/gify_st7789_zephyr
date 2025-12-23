#!/usr/bin/env bash

# High Intensity
IBlack='\033[0;90m'  # Black
IRed='\033[0;91m'    # Red
IGreen='\033[0;92m'  # Green
IYellow='\033[0;93m' # Yellow
IBlue='\033[0;94m'   # Blue
IPurple='\033[0;95m' # Purple
ICyan='\033[0;96m'   # Cyan
IWhite='\033[0;97m'  # White
IReset='\033[0m'     # Text Reset

# parse in CLI arguments
proj_dir=$(pwd)
command=$1
board=$2
pristine=$3

if [[ -e $4 ]]; then
    output_dir=$4
else
    output_dir=${proj_dir}/build
fi

# add full zephyr setup
if [[ ${command} == "help" || -z ${command} ]]; then
    (
        echo -e "${IGreen}Zephyr Build System Script${IReset}"
        echo "=========================================================="
        echo -e "${IGreen}Usage:${IReset}"
        echo "      ./script <command> <board> <pristine>"
        echo -e "${IGreen}Parameters:${IReset}"
        echo "      command     :   help / setup / build / flash / menuconfig / clean / monitor"
        echo "      board       :   Select a board [pixy_esp32/esp32/procpu or appcpu]"
        echo "      pristine    :   Perform a pristine build [always/auto]"

    )
    exit 0
fi

if [[ $(uname) == "Darwin" ]]; then
    zephyrproject_dir=~/zephyrproject
    zephyr_root_dir=~/zephyrproject/zephyr
elif [[ $(OS) == "Windows_NT" ]]; then
    echo "Not supported on Windows"
    exit 1
fi

# check if zephyr exists
if [[ ! -d ${zephyr_root_dir} ]]; then
    (
        cd ${proj_dir}/../../
        ./scripts/setup.sh
    )
fi

echo "Zephyr Project Directory : ${proj_dir}"
echo "Zephyr Root Directory : ${zephyr_root_dir}"

# add full zephyr setup
if [[ ${command} == "setup" ]]; then
    (
        source ~/zephyrproject/.venv/bin/activate
        cd ~/zephyrproject/zephyr
        west build
    )
fi

# build the project
if [[ ${command} == "build" ]]; then

    if [[ -z "${board}" ]]; then
        echo -e "${IRed}Please select a board${IReset}"
        exit 1
    fi

    if [[ -z "${pristine}" ]]; then
        echo -e "${IYellow}Performing Pristine build${IReset}"
        pristine=always
    fi

    source ~/zephyrproject/.venv/bin/activate
    cd ~/zephyrproject/zephyr
    west build -b=${board} ${proj_dir} -p=${pristine}

    if [[ ${4} == "flash" ]]; then
        west flash    
    fi

    if [[ ${5} == "monitor" ]]; then
        west espressif monitor    
    fi
fi

if [[ ${command} == "flash" ]]; then
    (
        source ~/zephyrproject/.venv/bin/activate
        cd ~/zephyrproject/zephyr
        west flash
    )
fi

if [[ ${command} == "monitor" ]]; then
    (
        source ~/zephyrproject/.venv/bin/activate
        cd ~/zephyrproject/zephyr
        west espressif monitor
    )
fi

if [[ ${command} == "clean" ]]; then
    (
        source ~/zephyrproject/.venv/bin/activate
        cd ~/zephyrproject/zephyr
        west build -t clean
    )
fi
