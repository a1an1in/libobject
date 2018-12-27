#include <jni.h>

JNIEXPORT jstring JNICALL Java_org_libsdl_app_SDLActivity_stringFromJNI(
        JNIEnv *env,
        jobject  this ) {

    return (*env)->NewStringUTF(env, "hello world");
}
