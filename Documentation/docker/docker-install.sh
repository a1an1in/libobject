#!/bin/sh -eu
script_path="$(cd "$(dirname "$0")"; pwd -P)"
docker_gpg_key_path="$script_path/docker-gpg-key"

distro="$(lsb_release --short --id )"
distro_version="$(lsb_release --short --release )"

if ! which apt-get > /dev/null; then
    echo "apt-get not found. Please install Docker manually instead." >&2
    exit 1
fi

# We only use the Docker.com repositories for Ubuntu 17.10 or older.
# In all other cases, i.e. 18.04 and newer (and for people being courageous
# enough to use something different), we use the Docker packages provided by the
# distributions's repositories. Package name differs, so set it here as well.
if [ "$distro" = "Ubuntu" ] && dpkg --compare-versions "$distro_version" lt '18.04'; then
    add_docker_repos=1
    docker_pkg_name="docker-ce"
    echo "Installing Docker from third-party APT repo provided by Docker.com"
else
    add_docker_repos=0
    docker_pkg_name="docker.io"
    echo "Installing Docker from official distribution ($distro) repository"
fi

if [ $add_docker_repos -eq 1 ]; then
    echo "Set up repositories"
    sudo apt-get update
    sudo apt-get install apt-transport-https
    sudo apt-key add "$docker_gpg_key_path"
    sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
    # Might need edge if running on a very new Ubuntu release
    # sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable edge"
fi

echo "Install docker (via package $docker_pkg_name)"
sudo apt-get update

sudo apt-get install "$docker_pkg_name"

echo "Allow current (non-root) user to run docker"
echo "  (must log out for this to take effect!)"
sudo usermod -aG docker "$USER"

# Test installation
echo ""
echo "===================================================="
echo ""
echo "To test the installation, run the following command:"
echo "  docker run --rm hello-world"
echo "(Use sudo if you are too impatient to log out first...)"
echo "Expected output is 'Hello from Docker!' followed by some more text"
echo ""
echo "To clean up Docker caches (old images, etc):"
echo "  docker system prune"

# To test docker gpg key:
# sudo apt-key fingerprint 0EBFCD88
# Key fingerprint should be = 9DC8 5822 9FC7 DD38 854A  E2D8 8D81 803C 0EBF CD88
#!/bin/sh -eu
script_path="$(cd "$(dirname "$0")"; pwd -P)"
docker_gpg_key_path="$script_path/docker-gpg-key"

distro="$(lsb_release --short --id )"
distro_version="$(lsb_release --short --release )"

if ! which apt-get > /dev/null; then
    echo "apt-get not found. Please install Docker manually instead." >&2
    exit 1
fi

# We only use the Docker.com repositories for Ubuntu 17.10 or older.
# In all other cases, i.e. 18.04 and newer (and for people being courageous
# enough to use something different), we use the Docker packages provided by the
# distributions's repositories. Package name differs, so set it here as well.
if [ "$distro" = "Ubuntu" ] && dpkg --compare-versions "$distro_version" lt '18.04'; then
    add_docker_repos=1
    docker_pkg_name="docker-ce"
    echo "Installing Docker from third-party APT repo provided by Docker.com"
else
    add_docker_repos=0
    docker_pkg_name="docker.io"
    echo "Installing Docker from official distribution ($distro) repository"
fi

if [ $add_docker_repos -eq 1 ]; then
    echo "Set up repositories"
    sudo apt-get update
    sudo apt-get install apt-transport-https
    sudo apt-key add "$docker_gpg_key_path"
    sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
    # Might need edge if running on a very new Ubuntu release
    # sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable edge"
fi

echo "Install docker (via package $docker_pkg_name)"
sudo apt-get update

sudo apt-get install "$docker_pkg_name"

echo "Allow current (non-root) user to run docker"
echo "  (must log out for this to take effect!)"
sudo usermod -aG docker "$USER"

# Test installation
echo ""
echo "===================================================="
echo ""
echo "To test the installation, run the following command:"
echo "  docker run --rm hello-world"
echo "(Use sudo if you are too impatient to log out first...)"
echo "Expected output is 'Hello from Docker!' followed by some more text"
echo ""
echo "To clean up Docker caches (old images, etc):"
echo "  docker system prune"

# To test docker gpg key:
# sudo apt-key fingerprint 0EBFCD88
# Key fingerprint should be = 9DC8 5822 9FC7 DD38 854A  E2D8 8D81 803C 0EBF CD88
