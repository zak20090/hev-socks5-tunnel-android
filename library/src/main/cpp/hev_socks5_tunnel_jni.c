#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define LOG_TAG "HevSocks5TunnelJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

/* External functions from hev-socks5-tunnel */
extern int hev_socks5_tunnel_main(int argc, char *argv[]);
extern void hev_socks5_tunnel_quit(void);
extern void hev_socks5_tunnel_stats(size_t *tx_packets, size_t *tx_bytes,
                               size_t *rx_packets, size_t *rx_bytes);

/* Global state */
static volatile int tunnel_running = 0;
static pthread_mutex_t tunnel_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Helper function to get FD from FileDescriptor */
JNIEXPORT jint JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_getFdFromFileDescriptor(
    JNIEnv *env, jobject thiz, jobject fileDescriptor) {
    
    jclass fdClass;
    jfieldID fdField;
    jint fd;
    
    if (fileDescriptor == NULL) {
        LOGE("FileDescriptor is null");
        return -1;
    }
    
    fdClass = (*env)->FindClass(env, "java/io/FileDescriptor");
    if (fdClass == NULL) {
        LOGE("Failed to find FileDescriptor class");
        return -1;
    }
    
    fdField = (*env)->GetFieldID(env, fdClass, "descriptor", "I");
    if (fdField == NULL) {
        LOGE("Failed to find descriptor field");
        (*env)->DeleteLocalRef(env, fdClass);
        return -1;
    }
    
    fd = (*env)->GetIntField(env, fileDescriptor, fdField);
    (*env)->DeleteLocalRef(env, fdClass);
    
    LOGD("Extracted FD: %d", fd);
    return fd;
}

/* Write config to temporary file */
static char* writeConfigToTempFile(const char *config, size_t config_len) {
    char tmpFile[] = "/data/local/tmp/hev-socks5-tunnel-XXXXXX";
    char *result;
    int fd;
    ssize_t written;
    
    fd = mkstemp(tmpFile);
    if (fd < 0) {
        LOGE("Failed to create temp file: %s", strerror(errno));
        return NULL;
    }
    
    written = write(fd, config, config_len);
    close(fd);
    
    if (written != (ssize_t)config_len) {
        LOGE("Failed to write config to temp file");
        unlink(tmpFile);
        return NULL;
    }
    
    LOGI("Config written to: %s", tmpFile);
    result = strdup(tmpFile);
    return result;
}

/* Start tunnel from config file */
JNIEXPORT jint JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeStart(
    JNIEnv *env, jobject thiz, jstring configPath, jint tunFd) {
    
    const char *configPathStr;
    char arg0[] = "hev-socks5-tunnel";
    char arg1[] = "-c";
    char *argv[4];
    char tunFdEnv[32];
    int result;
    
    pthread_mutex_lock(&tunnel_mutex);
    if (tunnel_running) {
        LOGW("Tunnel is already running");
        pthread_mutex_unlock(&tunnel_mutex);
        return -1;
    }
    tunnel_running = 1;
    pthread_mutex_unlock(&tunnel_mutex);
    
    configPathStr = (*env)->GetStringUTFChars(env, configPath, NULL);
    if (configPathStr == NULL) {
        LOGE("Failed to get config path string");
        tunnel_running = 0;
        return -1;
    }
    
    LOGI("Starting tunnel with config: %s, TUN FD: %d", configPathStr, tunFd);
    
    /* Prepare arguments for hev-socks5-tunnel */
    argv[0] = arg0;
    argv[1] = arg1;
    argv[2] = (char*)configPathStr;
    argv[3] = NULL;
    
    /* Set TUN FD environment variable */
    snprintf(tunFdEnv, sizeof(tunFdEnv), "%d", tunFd);
    setenv("HEV_SOCKS5_TUNNEL_TUN_FD", tunFdEnv, 1);
    
    /* Run the tunnel */
    result = hev_socks5_tunnel_main(3, argv);
    
    (*env)->ReleaseStringUTFChars(env, configPath, configPathStr);
    
    pthread_mutex_lock(&tunnel_mutex);
    tunnel_running = 0;
    pthread_mutex_unlock(&tunnel_mutex);
    
    LOGI("Tunnel exited with code: %d", result);
    return result;
}

/* Start tunnel from config string */
JNIEXPORT jint JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeStartFromString(
    JNIEnv *env, jobject thiz, jstring configYaml, jint tunFd) {
    
    const char *configStr;
    char *tempFile;
    char arg0[] = "hev-socks5-tunnel";
    char arg1[] = "-c";
    char *argv[4];
    char tunFdEnv[32];
    jsize config_len;
    int result;
    
    pthread_mutex_lock(&tunnel_mutex);
    if (tunnel_running) {
        LOGW("Tunnel is already running");
        pthread_mutex_unlock(&tunnel_mutex);
        return -1;
    }
    tunnel_running = 1;
    pthread_mutex_unlock(&tunnel_mutex);
    
    configStr = (*env)->GetStringUTFChars(env, configYaml, NULL);
    if (configStr == NULL) {
        LOGE("Failed to get config string");
        tunnel_running = 0;
        return -1;
    }
    
    LOGI("Starting tunnel with inline config, TUN FD: %d", tunFd);
    LOGD("Config:\n%s", configStr);
    
    config_len = (*env)->GetStringUTFLength(env, configYaml);
    
    /* Write config to temporary file */
    tempFile = writeConfigToTempFile(configStr, config_len);
    (*env)->ReleaseStringUTFChars(env, configYaml, configStr);
    
    if (tempFile == NULL) {
        LOGE("Failed to create config file");
        tunnel_running = 0;
        return -1;
    }
    
    /* Prepare arguments */
    argv[0] = arg0;
    argv[1] = arg1;
    argv[2] = tempFile;
    argv[3] = NULL;
    
    /* Set TUN FD environment variable */
    snprintf(tunFdEnv, sizeof(tunFdEnv), "%d", tunFd);
    setenv("HEV_SOCKS5_TUNNEL_TUN_FD", tunFdEnv, 1);
    
    /* Run the tunnel */
    result = hev_socks5_tunnel_main(3, argv);
    
    /* Clean up temp file */
    unlink(tempFile);
    free(tempFile);
    
    pthread_mutex_lock(&tunnel_mutex);
    tunnel_running = 0;
    pthread_mutex_unlock(&tunnel_mutex);
    
    LOGI("Tunnel exited with code: %d", result);
    return result;
}

/* Stop the tunnel */
JNIEXPORT void JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeStop(
    JNIEnv *env, jobject thiz) {
    
    LOGI("Requesting tunnel stop");
    hev_socks5_tunnel_quit();
}

/* Get tunnel statistics */
JNIEXPORT jlongArray JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeGetStats(
    JNIEnv *env, jobject thiz) {
    
    size_t tx_packets = 0, tx_bytes = 0, rx_packets = 0, rx_bytes = 0;
    jlongArray result;
    jlong stats[4];
    
    pthread_mutex_lock(&tunnel_mutex);
    if (tunnel_running) {
        hev_socks5_tunnel_stats(&tx_packets, &tx_bytes, &rx_packets, &rx_bytes);
    }
    pthread_mutex_unlock(&tunnel_mutex);
    
    result = (*env)->NewLongArray(env, 4);
    if (result == NULL) {
        LOGE("Failed to create stats array");
        return NULL;
    }
    
    stats[0] = (jlong)tx_bytes;
    stats[1] = (jlong)rx_bytes;
    stats[2] = (jlong)tx_packets;
    stats[3] = (jlong)rx_packets;
    
    (*env)->SetLongArrayRegion(env, result, 0, 4, stats);
    return result;
}

/* JNI_OnLoad */
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    
    LOGI("JNI_OnLoad called");
    
    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("Failed to get JNI environment");
        return JNI_ERR;
    }
    
    LOGI("HevSocks5Tunnel JNI loaded successfully");
    return JNI_VERSION_1_6;
}
