#!/usr/bin/env bash

. $(dirname $0)/scripts/utils.sh

function do_build_linux {
    increase_version_tweak_number
    rm -rf /usr/local/include/libobject
    mkdir -p build/linux
    cd build/linux
    cmake ../.. -DPLATFORM=linux -DMODULE_UI=off
    make
    if [[ $OPTION_INSTALL == "true" ]]; then
        make install
        exit 0
    fi
}

function do_build_mac {
    mkdir -p build/mac
    cd build/mac
    cmake ../.. -DPLATFORM=mac &&make
    cd ../..
    printf '\x07' | dd of=sysroot/mac/bin/xtools bs=1 seek=160 count=1 conv=notrunc
}

function do_build_ios {
    mkdir -p build/ios
    cd build/ios
    cmake ../.. -DPLATFORM=ios -DIOS_OPTION_PLATFORM=SIMULATOR64 -DCMAKE_TOOLCHAIN_FILE=../../mk/ios.toolchain.cmake
    makes
    #make install
    #cd ../..
    #cp -rf src/include/libobject /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include/
}

function do_build_android {
    mkdir -p build/android
    cd build/android
    cmake ../.. -DOPTION_PLATFORM=android -DANDROID_ABI=armeabi-v7a -DCMAKE_ANDROID_NDK=/Users/alanlin/Library/Android/sdk/ndk-bundle&&make&&
    cd ../..

    #cp -rf lib/android/ ~/workspace/goya-github/android/goya/app/src/main/jni/lib
    #cp -rf lib/android/ ~/workspace/goya-github/android/goya-alone/app/src/main/jni/lib
    #cp lib/android/armeabi-v7a/*.so ${NDK_ROOT}/platforms/android-21/arch-arm/usr/lib
    #cp lib/android/armeabi-v7a/*.so ${NDK_ROOT}/platforms/android-21/arch-arm/usr/lib
    #cp -rf lib/android/armeabi-v7a/*.so ~/workspace/goya-github/android/goya-alone/app/src/main/jni/lib/armeabi-v7a

    #cp -rf src/include/libobject ${NDK_ROOT}/sysroot/usr/include/
    #cp -rf src/include/libobject ~/workspace/goya-github/android/goya-alone/app/src/main/jni/include/libobject
}

function do_build_windows {
    increase_version_tweak_number
    # rm -rf /usr/local/include/libobject
    mkdir -p build/windows
    cd build/windows
    cmake ../.. -G "Unix Makefiles" -DPLATFORM=windows -DMODULE_UI=off
    make
    if [[ $OPTION_INSTALL == "true" ]]; then
        mingw32-make install
        echo "mingw32-make install"
        exit 0
    fi
}

function do_build {
    if [[ $OPTION_PLATFORM == "linux" ]]; then
        echo "build linux"
        do_build_linux
        exit 0
    elif [[ $OPTION_PLATFORM == "mac" ]]; then
        echo "build mac"
        echo "PlatForm: ${OPTION_PLATFORM}"
        do_build_mac
    elif [[ $OPTION_PLATFORM == "ios" ]]; then
        echo "build ios"
        echo "PlatForm: ${OPTION_PLATFORM}"
        do_build_ios
    elif [[ $OPTION_PLATFORM == "android" ]]; then
        echo "build android"
        echo "PlatForm: ${OPTION_PLATFORM}"
        do_build_android
    elif [[ $OPTION_PLATFORM == "windows" ]]; then
        echo "-- build windows"
        echo "-- PlatForm: ${OPTION_PLATFORM}"
        do_build_windows
    else
        OPTION_HELP="true"
    fi
}

function do_clean {
    echo "clean build"
    rm -rf build && rm -rf sysroot
}

function do_release {
    if [[ $OPTION_PLATFORM == "linux" ]]; then
        echo "do release package"
    else
        OPTION_HELP="true"
        return 0;
    fi

    if [ ! -d "sysroot/linux" ];then
        exit 1
    fi

    if [ ! -f "src/include/libobject/version.h" ];then
        exit 1
    fi
    
    file_name=$(get_release_package_name xtools_linux src/include/libobject/version.h)
    echo "file_name:$file_name"
    if [ -f packages/$file_name ]; then
        exit 0
    elif [ ! -n $file_name ]; then
        exit 1
    fi

    mkdir -p packages/xtools
    cp -rf sysroot/linux/* packages/xtools/
    cd packages
    tar -zcvf $file_name xtools/*
    rm -rf xtools

    return 0
}

function do_deploy {
    if [[ $OPTION_PLATFORM == "linux" ]]; then
        echo "1.kill running xtools progess ..."
        ret=$($expect_command_prefix "ssh" "root@$OPTION_IP" "ps -aux|grep "xtools" | grep -v grep | awk '{print \$2}'")
        ret=$(echo "$ret" | tr -d '\r')
        ret=$(echo "$ret" | awk 'NR >=3 {print $1}')
        echo "ret:" $ret

        array=(`echo $ret` )  
        for element in "${array[@]}"
        do
            if ! $expect_command_prefix "ssh" "root@$OPTION_IP" "kill -9" "$element"; then
                echo "kill xtools process failed!"
            else
                echo "kill running xtools progess id: $element success"
            fi
        done

        echo "2.mkdir packages: $OPTION_TO_PATH"
        if ! $expect_command_prefix "ssh" "root@$OPTION_IP" "mkdir -p $OPTION_TO_PATH"; then
            echo "uploading images failed!"
            exit 1
        fi

        echo "3.copy package to $OPTION_IP:$OPTION_TO_PATH ..."
        if ! $expect_command_prefix "scp" $OPTION_PACKAGE_PATH "root@$OPTION_IP:$OPTION_TO_PATH"; then
            echo "uploading images failed!"
            exit 1
        fi

        echo "4.mkdir ~/.xtools/sysroot/"
        if ! $expect_command_prefix "ssh" "root@$OPTION_IP" "rm -rf ~/.xtools/sysroot/ && mkdir -p ~/.xtools/sysroot/"; then
            echo "uploading images failed!"
            exit 1
        fi

        echo "5.expand $(basename $OPTION_PACKAGE_PATH) to working dir ..."
        if ! $expect_command_prefix "ssh" "root@$OPTION_IP" "tar -zxvf $OPTION_TO_PATH/$(basename $OPTION_PACKAGE_PATH) -C ~/.xtools/sysroot --strip-components 1"; then
            echo "uploading images failed!"
            exit 1
        fi

        echo "6.run xtools program"
        if ! $expect_command_prefix "ssh" "root@$OPTION_IP" "{ export LD_LIBRARY_PATH=~/.xtools/sysroot/lib:~/.xtools/sysroot/lib64:\$LD_LIBRARY_PATH&&nohup stdbuf -oL -eL ~/.xtools/sysroot/bin/xtools --log-level=0x20016 node --host=0.0.0.0 --service=12345 --deamon=t; } >~/.xtools/logs 2>&1 &"; then
            echo "run xtools program failed!"
            exit 1
        fi
        echo "7.deploy xtools success!"
    else
        OPTION_HELP="true"
        return 0;
    fi

    return 0
}

function do_install_docker {
    if [[ -z $OPTION_PLATFORM ]]; then
        echo -e "\nplease set platform when install docker\n"
        exit 1
    fi
    echo "do_install_docker"
    distro="$(lsb_release --short --id )"
    distro_version="$(lsb_release --short --release )"
    echo "$distro $distro_version"
    if [[ $OPTION_PLATFORM == "linux" ]]; then
        echo "install docker at linux"
        if ! which curl > /dev/null; then
            echo "install curl, please wait..."
            sudo apt-get install curl
        fi
        if ! which docker > /dev/null; then
            echo "install docker, please wait..."
            curl -fsSL https://get.docker.com | bash -s docker --mirror Aliyun
        fi
        local user_name="$( getent passwd `who` | head -n 1 | cut -d : -f 1 )"
        echo "user_name:$user_name"
        sudo usermod -aG docker "$user_name"
        exit 0
    elif [[ $OPTION_PLATFORM == "mac" ]]; then
        exit 0
    elif [[ $OPTION_PLATFORM == "ios" ]]; then
        exit 0
    elif [[ $OPTION_PLATFORM == "android" ]]; then
        exit 0
    fi
}

function do_build_docker_image_paddlefish {
    local version
    local tar_file_name

    echo "do deploy arg1:$OPTION_DB_IP"
    version=$(get_release_package_version src/include/version.h)
    tar_file_name=$(get_release_package_name paddlefish_linux src/include/version.h)
    echo "tar_file_name:$tar_file_name, version:$version"
    docker build -f docker/dockerfile                                                 \
        -t agile/paddlefish:$version                                                  \
        -t agile/paddlefish                                                           \
        --build-arg TAR_PACKAGE_NAME=${tar_file_name}                                 \
        --build-arg DB_IP=$OPTION_DB_IP                                               \
        .
}

function do_remove_docker_container {
    local container_id
    
    if [[ -z $OPTION_NAME ]]; then
        OPTION_HELP="true"
        return 1
    fi
    container_id=$(docker ps -a| grep $OPTION_NAME | awk '{print $1}')
    echo "remover docker container, container id:$container_id"
    docker stop $container_id
    docker rm $container_id
}

function do_run_docker_container {
    local image_name
    local container_id
    
    if [[ -z $OPTION_NAME ]]; then
        OPTION_HELP="true"
        return 1
    fi
    image_name=$(docker images | grep $OPTION_NAME | awk '{print $1}')
    echo "run docker container, container image_name:$image_name OPTION_ARGS:$OPTION_ARGS"
    docker run -itd $OPTION_ARGS agile/paddlefish
    container_id=$(docker ps | grep $OPTION_NAME | awk '{print $1}')
    docker exec -it $container_id /bin/bash
}

function do_build_docker_image {
    if [[ -z $OPTION_DB_IP ]]; then
        OPTION_HELP="true"
        return 1
    fi
    if [[ -z $OPTION_NAME ]]; then
        OPTION_HELP="true"
        return 1
    elif [[ $OPTION_NAME == "paddlefish" ]]; then
        do_build_docker_image_paddlefish
    fi
}

function do_test {
    echo "do_test"
}


function do_help { # pring usage message to STDOUT
cat << EOF
    Usage:
    devops.sh build|clean|release|deploy|docker|test|help 
            [-p|--platform=<OPTION_PLATFORM>] [-i|--install] [--install-docker] 
            [--db-ip=IP_ADRESS] [--package-path]
            [--build=PACKAGE_NAME] [--run=PACKAGE_NAME] [--remove=PACKAGE_NAME]
    
    commands:
    build                  build this repo.
        -p, --platform:    Optional, OPTION_PLATFORM can be: linux, mac, windows, ios, android.
        --install:         Optional. install lib, bin, and include files.

    release                release package
        -p, --platform:    Optional, OPTION_PLATFORM can be: linux, mac, windows, ios, android.

    clean                  clean build files.  

    docker                 do docker relevant operations.
        --db-ip            Optional. db ip address.
        --package-root     Optional. deploy package root path.
        --install          Optional. install docker
        --remove           Optional. remove container
        --run              Optional. run container
        --args             Optional. designate args.
        --build            build docker.

    test                   do some test at this command

    help                   Print this help message.

    demos:
    ./devops.sh build --platform=linux
    ./devops.sh build --platform=windows
    ./devops.sh docker --install --platform=linux    #install docker at linux platform
    ./devops.sh release -p=linux
    ./devops.sh deploy -p=linux --host=139.159.231.27 --package-path=./packages/xtools_v2.13.0.442.tar.gz
    
EOF
}