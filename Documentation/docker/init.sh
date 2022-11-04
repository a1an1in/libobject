# Build the PetaLinux SDK Docker image and
# provide wrapper function for running it

# Check arguments
if [ "$1" ]
then
  PLSDK_DOCKER_CONTEXT_PATH=$(cd "$1" && pwd)
fi
if [ -z "$PLSDK_DOCKER_CONTEXT_PATH" ]
then
  echo "No PetaLinux SDK Docker image context path specified!"
  return 1
fi
if [ ! -f "$PLSDK_DOCKER_CONTEXT_PATH/Dockerfile" ]
then
  echo "Invalid PetaLinux SDK Docker image context path specified!"
  return 1
fi

if ! which docker > /dev/null; then
  echo "Docker not found. Install Docker using"
  echo "    $(dirname "$0")/docker-install.sh"
  return 1
fi

# Replace hard URLs with environment variables
. $(dirname "${BASH_SOURCE[0]}")/environment-variables.sh

# Create directory for isolated Docker container $HOME
mkdir -p $DOCKERHOME/.plsdk

PLSDK_ROOT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"

# Internal helper function to create isolated ssh-config dedicated to the docker-container
function create-ssh-config-if-needed {
  local user_ssh_known_hosts_path="$HOME/.ssh/known_hosts"
  local ssh_known_hosts_path="$DOCKERHOME/.plsdk/.ssh/known_hosts"
  local user_ssh_config_path="$HOME/.ssh/config"
  local ssh_config_path="$DOCKERHOME/.plsdk/.ssh/config"
  local ssh_config_dir="$(dirname "$ssh_config_path")"
  if [ -f "$ssh_config_path" ]; then
    return 0
  fi

  # Empty line to keep some distance to previous output
  echo

  mkdir -p "$ssh_config_dir"

  if [ ! -f "$ssh_known_hosts_path"  ]; then
    cp -v "$user_ssh_known_hosts_path" "$ssh_known_hosts_path" || echo "Failed copying SSH known_hosts"
  fi

  if grep -Fsq "${GERRIT_HOSTNAME}" "$user_ssh_config_path"; then
    cp -v "$user_ssh_config_path" "$ssh_config_path" && echo "SSH-config copied."
    echo
    echo "Make sure your username for ${GERRIT_HOSTNAME} is configured in $ssh_config_path."
    echo
  else
    cat << EOF > "$ssh_config_path"
Host ${GERRIT_HOSTNAME}
    Port 29418
    User $(whoami)
EOF
    echo "A dedicated SSH config has been created. This is needed for cloning repositories in Docker"
    echo "during build. The config is located at $ssh_config_path and"
    echo "you might need to change the username to the one you use to login to"
    echo "${GERRIT_HOSTNAME} (usually firstname.lastname)."
    echo
    echo ">>>BEGIN CREATED CONFIG $ssh_config_path"
    cat "$ssh_config_path"
    echo "<<<END CREATED CONFIG $ssh_config_path"
    echo
  fi
  # Warning for ssh incompatibility
  echo "========================================================================"
  echo -e "\e[1;34mATTENTION:\e[0m One might not be able to access ssh correctly when"
  echo "compiling the product, which might be caused by config-options incompatiblity"
  echo "with the old SSH-version in Docker image v2014.4 PetaLinux SDK."
  echo "Please test the config using: "
  echo -e "\e[1;34mplsdk ssh ${GERRIT_HOSTNAME}\e[0m"
  echo "You will receive a welcome message when you connect successfully via SSH."
  echo "If it fails and there are errors about incompatible options, you need to remove"
  echo "them, which are located at $ssh_config_path "
  echo "========================================================================"
}

# Internal helper function
function get-plsdk-image-tag {
  echo $(basename "$PLSDK_DOCKER_CONTEXT_PATH")-$(echo "$PLSDK_DOCKER_CONTEXT_PATH" | sha256sum | cut -d ' ' -f 1)-u$(id -u)-g$(id -g)
}

# For external use
function plsdk {
  local plsdk_tty_arg
  local plsdk_env_arg
# check if "i" is in options list (if run in interactive mode)
  case $- in *i*) plsdk_tty_arg=-t;; esac
# check if DISPLAY variable set (if X-server is used)
  if [ "${DISPLAY+x}" ]; then
    plsdk_env_arg=(--network host -e DISPLAY="$DISPLAY")
  fi

  # If no SSH Agent is available, we start our own.
  # We also add a key to SSH-Agent if none are registered.
  # Using subshell to keep the started ssh-agent isolated to this invocation.
  # Not using a subshell might lead to undesirable results when another function
  # overrides the EXIT-trap or in the following scenario
  # 1. Run plsdk without agent running (plsdk starts agent)
  # 2. User starts their own ssh-agent
  # 3. User exits shell
  # 4. UNEXPECTED: The exit-trap we registered will kill the manually started
  #    agent and keep the one started by us.
  (
    if [ -z "$SSH_AUTH_SOCK" ]; then
      eval $(ssh-agent | grep -v 'echo Agent pid .*;')
      trap 'eval $(ssh-agent -k | (grep -v "echo Agent pid .* killed;" || true))' EXIT
    fi
    local num_keys_registered="$(ssh-add -l | (grep -v "The agent has no identities." || true) | wc -l)"
    if [ $num_keys_registered -eq 0 ]; then
      # Filtering output because OpenSSH <7.6 does not have the -q parameter
      # The line filtered out (Identity added) is the only line in the output,
      # which will make grep -v return 1 which will cause issues when Jenkins
      # for example runs the script with set -e. Therefore we need to ignore
      # the return code.
      # Also redirect any output that is left to stderr should there be any
      # other stderr-messages.
      (ssh-add 2>&1 || echo "Failed to add SSH-keys to agent, cloning repos might not work!" >&2) | (grep -v '^Identity added' || true) 1>&2
    fi

    ${plsdk_cmd_prefix} docker run \
      --rm \
      -i \
      $plsdk_tty_arg \
      ${plsdk_env_arg[@]} \
      -v "$PLSDK_ROOT_PATH:$PLSDK_ROOT_PATH" \
      -w "$(pwd)" \
      -v "$DOCKERHOME/.plsdk:/home" \
      -v "$SSH_AUTH_SOCK:/var/run/ssh-agent" \
      -e SSH_AUTH_SOCK=/var/run/ssh-agent \
      -u "$(id -u):$(id -g)" \
      petalinux-sdk:"$(get-plsdk-image-tag)" \
      "$@"
  )
}

function tplsdk {
    plsdk_cmd_prefix="time"
    plsdk $@
    unset plsdk_cmd_prefix
}

# For PetaLinux SDK v2017.3:
# - Full build takes ~25 minutes and requires ~30Gb disk space
# - Final image is ~10Gb
docker build --network host \
  --build-arg HOST_USER_UID=$(id -u) \
  --build-arg HOST_USER_GID=$(id -g) \
  --build-arg ARTIFACTORY_URL=${ARTIFACTORY_URL} \
  --build-arg ARTIFACTORY_PUBLIC_INSTALL_MEDIA_BASE=${ARTIFACTORY_PUBLIC_INSTALL_MEDIA_BASE} \
  --build-arg GERRIT_REPO_BASE=${GERRIT_REPO_BASE} \
  -t petalinux-sdk \
  -t petalinux-sdk:"$(get-plsdk-image-tag)" \
  "$PLSDK_DOCKER_CONTEXT_PATH"

if [ $? -ne 0 ]
then
  echo ""
  echo "ERROR: Failed to build PetaLinux SDK Docker image!"
  unset -f plsdk
  if ! id -nGz | grep -qzxF docker &&  [ "$(id -u)" != "0" ]; then
    echo "Guessing the reason for the failure:"
    if getent group docker | grep &>/dev/null "\b$(whoami)\b"; then
      echo "You seem to be a member of the docker group, but do not seem to have"
      echo "logged out and back in again to make your changes take effect."
      echo "Do that (or if you are impatient: su $(whoami))"
    else
      echo "You do not seem to be a member of the docker group."
      echo "Add yourself to the docker group to enable running docker without root."
      echo "sudo usermod -aG docker \"$(whoami)\""
    fi
  fi
  return 1
fi

# Keep a separate SSH-config to prevent incompatibilities between host SSH and docker SSH
# Create a template config if no config exists
create-ssh-config-if-needed
# Function no longer needed, unset to prevent showing up in tab-completion
unset -f create-ssh-config-if-needed

test -f $HOME/.netrc && cp -n "$HOME/.netrc" "$DOCKERHOME/.plsdk" || echo "No .netrc file found"

# Copy user profile skeleton files to the SDK home directory
# to get some default aliases. Use "no clobber" (-n) to preserve
# any manual changes
docker run --rm -v "$DOCKERHOME/.plsdk:/home" -u "$(id -u):$(id -g)" petalinux-sdk:"$(get-plsdk-image-tag)" cp -n -r /etc/skel/. /home

echo ""
echo "PetaLinux SDK Docker image usage:"
echo "  plsdk [command [arg]...]"
echo "    - Runs interactive bash shell if command is omitted"
echo "    - Use plenv wrapper for running PetaLinux commands:"
echo "      plsdk plenv <petalinux-cmd> [arg]..."
echo "    - Current directory becomes /work in container"
