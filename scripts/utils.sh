function get_version_number {
    local version
    version=$(grep "project (libobject VERSION" CMakeLists.txt | awk '{print $4}')
    version=$(echo $version|sed 's/)//g')
    echo "$version"
}

function increase_version_tweak_number {
    local version
    local new_version

    version=$(get_version_number)
    echo "-- increase_version_tweak_number:$version"
    new_verison=$(echo $version | awk -F. '{print $1"."$2"."$3"."++$4}')
    echo "-- sed -i 's/$version/$new_verison/g' CMakeLists.txt"
    sed -i "s/$version/$new_verison/g" CMakeLists.txt
}
