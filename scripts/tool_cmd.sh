#!/usr/bin/env bash

. $(dirname $0)/scripts/utils.sh

function do_build_linux {
    increase_version_tweak_number

    # 如果未指定架构，默认使用 x86_64
    if [[ -z $OPTION_ARCH ]]; then
        echo "No architecture specified. Defaulting to x86_64."
        OPTION_ARCH="x86_64"
    fi

    # 根据架构动态设置构建目录、工具链文件和安装目录
    case $OPTION_ARCH in
        "x86_64")
            echo "Building for x86_64 architecture"
            BUILD_DIR="build/linux/x86_64"
            TOOLCHAIN_FILE="../../../mk/linux-x86_64.toolchain.cmake"
            SYSROOT_DIR="../../../sysroot/linux/x86_64"
            ;;
        "arm")
            echo "Building for ARM architecture"
            BUILD_DIR="build/linux/arm"
            TOOLCHAIN_FILE="../../../mk/linux-arm.toolchain.cmake"
            SYSROOT_DIR="../../../sysroot/linux/arm"
            ;;
        *)
            echo "Error: Unsupported architecture: $OPTION_ARCH"
            exit 1
            ;;
    esac

    # 创建构建目录
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR || { echo "Failed to enter build directory: $BUILD_DIR"; exit 1; }

    # 检查工具链文件是否存在
    if [[ ! -f $TOOLCHAIN_FILE ]]; then
        echo "Error: Toolchain file not found: $TOOLCHAIN_FILE"
        exit 1
    fi

    # 检查 CMakeLists.txt 文件是否存在于构建目录的父级
    if [[ ! -f ../../../CMakeLists.txt ]]; then
        echo "Error: CMakeLists.txt not found in the source directory."
        exit 1
    fi

    # 运行 CMake，加载工具链文件
    cmake ../../../ \
        -DPLATFORM=linux \
        -DMODULE_UI=off \
        -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} || { echo "CMake configuration failed."; exit 1; }

    make || { echo "Make failed."; exit 1; }

    if [[ $OPTION_INSTALL == "true" ]]; then
        make install || { echo "Make install failed."; exit 1; }
        exit 0
    fi

    # 检查 attacher.sh 文件是否存在
    if [[ ! -f ../../../scripts/attacher.sh ]]; then
        echo "Error: attacher.sh not found in ../../scripts."
        exit 1
    fi

    # 将 attacher.sh 脚本复制到对应架构的 bin 目录
    mkdir -p ${SYSROOT_DIR}/bin
    cp ../../../scripts/attacher.sh ${SYSROOT_DIR}/bin/attacher.sh
    chmod +x ${SYSROOT_DIR}/bin/attacher.sh
    echo "attacher.sh has been copied to ${SYSROOT_DIR}/bin"
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

        # 如果未指定架构，默认使用 x86_64
        if [[ -z $OPTION_ARCH ]]; then
            echo "No architecture specified. Defaulting to x86_64."
            OPTION_ARCH="x86_64"
        fi

        # 根据架构设置 SYSROOT_DIR
        if [[ $OPTION_ARCH == "arm" ]]; then
            SYSROOT_DIR="sysroot/linux/arm"
        else
            SYSROOT_DIR="sysroot/linux/x86_64"
        fi

        # 检查必要的文件和目录
        if [ ! -d "$SYSROOT_DIR" ]; then
            echo "Error: $SYSROOT_DIR does not exist."
            exit 1
        fi

        if [ ! -f "src/include/libobject/version.h" ]; then
            echo "Error: src/include/libobject/version.h does not exist."
            exit 1
        fi

        # 获取发布包名称
        file_name=$(get_release_package_name xtools_linux_${OPTION_ARCH} src/include/libobject/version.h)
        echo "file_name: $file_name"

        # 如果包已存在，直接退出
        if [ -f packages/$file_name ]; then
            echo "Package $file_name already exists."
            exit 0
        elif [ -z "$file_name" ]; then
            echo "Error: Failed to generate package name."
            exit 1
        fi

        # 创建发布包
        mkdir -p packages/xtools
        cp -rf $SYSROOT_DIR/* packages/xtools/
        cd packages
        tar -zcvf $file_name xtools/*
        rm -rf xtools

        echo "Release package created: $file_name"
        return 0
    else
        OPTION_HELP="true"
        return 0
    fi
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