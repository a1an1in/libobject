#!/usr/bin/env bash

. $(dirname $0)/scripts/environment_variables.sh
. $(dirname $0)/scripts/tool_cmd.sh

function get_release_package_name {
    local version
    version=$(grep "PROJECT_VERSION" $2 | awk '{print $3}')
    version=$(echo $version|sed 's/\"//g')
    echo "$1_$version.tar.gz"
}

function get_release_package_version {
    local version
    version=$(grep "PROJECT_VERSION" $1 | awk '{print $3}')
    version=$(echo $version|sed 's/\"//g')
    echo "$version"
}

function parse_args { # Read commande line arguments and update global vairbles
    local error_msg=""
    local i
    for i in "$@"; do # Parse arguments in arg=value format
        case $i in
            build|clean|release|deploy|docker|test|help)
                CMD_NAME=$i
                OPTION_HELP=""
                shift # past argument=value
                ;;
            -p=*|--platform=*)
                OPTION_PLATFORM="${i#*=}"
                OPTION_HELP=""
                shift # past argument=value
                ;;
            -i|--install)
                OPTION_INSTALL=true
                OPTION_HELP=""
                shift # past argument
                ;;
            --build=*)
                OPTION_BUILD=true
                OPTION_NAME="${i#*=}"
                OPTION_HELP=""
                shift # past argument
                ;;
            --remove=*)
                OPTION_NAME="${i#*=}"
                OPTION_REMOVE=true
                OPTION_HELP=""
                shift # past argument
                ;;
            --run=*)
                OPTION_NAME="${i#*=}"
                OPTION_RUN=true
                OPTION_HELP=""
                shift # past argument
                ;;
            --args=*)
                OPTION_ARGS="${i#*=}"
                OPTION_HELP=""
                shift # past argument
                ;;
            --host=*)
                OPTION_IP="${i#*=}"
                OPTION_HELP=""
                shift # past argument=value
                ;;
            --to=*)
                OPTION_TO_PATH="${i#*=}"
                OPTION_HELP=""
                shift # past argument
                ;;  
            --package-path=*)
                OPTION_PACKAGE_PATH="${i#*=}"
                OPTION_HELP=""
                shift # past argument
                ;; 
            *)
                error_msg="Unknown option $i"
                OPTION_HELP="true"
                shift # past argument
                ;;
        esac
    done
}

function process_args {
    if command -v expect > /dev/null && command -v ./scripts/expect.sh > /dev/null ; then
        echo "find auto exec command"
        expect_command_prefix="./scripts/expect.sh Alan@123"
    fi

    if [[ $CMD_NAME == "build" ]]; then
        do_build
    elif [[ $CMD_NAME == "release" ]]; then
        do_release
    elif [[ $CMD_NAME == "clean" ]]; then
        do_clean
        exit 0
    elif [[ $CMD_NAME == "docker" ]]; then
        echo "docker"
        if [[ $OPTION_INSTALL == "true" ]]; then
            echo "install docker"
            do_install_docker
            exit 0
        elif [[ $OPTION_BUILD == "true" ]]; then
            echo "build"
            do_build_docker_image
        elif [[ $OPTION_REMOVE == "true" ]]; then
            do_remove_docker_container
        elif [[ $OPTION_RUN == "true" ]]; then
            do_run_docker_container
        else 
            OPTION_HELP="true"
        fi
    elif [[ $CMD_NAME == "deploy" ]]; then
        do_deploy
        exit 0
    elif [[ $CMD_NAME == "test" ]]; then
        do_test
    elif [[ $CMD_NAME == "help" ]]; then
        do_help
        exit 0
    else
        OPTION_HELP="true"
    fi

    if [[ -n $error_msg ]]; then
        do_help
        echo -e "\nErrors:\n${error_msg}\n"
        exit 1
    fi

    if [[ $OPTION_HELP == "true" ]]; then
        do_help
        exit 0
    fi
}

parse_args "$@"
process_args
