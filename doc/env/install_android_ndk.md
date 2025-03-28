# Installing Android NDK on Ubuntu

Follow these steps to install Android NDK on Ubuntu:

---

## 1. Download Android NDK
Visit the [Android NDK official page](https://developer.android.com/ndk/downloads) and download the Linux version of the NDK.

Alternatively, use the following command to download it directly:
```bash
wget https://dl.google.com/android/repository/android-ndk-r27c-linux.zip
```

---

## 2. Extract the NDK
Extract the downloaded NDK zip file to a desired directory, such as `/opt/android-ndk`:
```bash
unzip android-ndk-r27c-linux.zip -d /opt/
mv /opt/android-ndk-r27c /opt/android-ndk
```

---

## 3. Configure Environment Variables
Add the NDK path to your environment variables for easy access.

Edit the `~/.bashrc` file:
```bash
nano ~/.bashrc
```

Add the following lines:
```bash
export ANDROID_NDK_HOME=/opt/android-ndk
export PATH=$ANDROID_NDK_HOME:$PATH
```

Save and exit the editor, then refresh the environment variables:
```bash
source ~/.bashrc
```

---

## 4. Verify Installation
Check if the NDK is installed correctly by running:
```bash
ndk-build --version
```

If the version information is displayed, the installation was successful.

---

## 5. Install Additional Tools (Optional)
If you need tools like CMake or other dependencies, install them using:
```bash
sudo apt update
sudo apt install cmake build-essential
```

---

You are now ready to use Android NDK for development and cross-compilation.
