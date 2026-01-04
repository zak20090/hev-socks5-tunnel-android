#include <jni.h>
#include <android/log.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <pthread.h>

#define LOG_TAG "HevSocks5TunnelJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// External functions from hev-socks5-tunnel
extern "C" {
    int hev_socks5_tunnel_main(int argc, char *argv[]);
    void hev_socks5_tunnel_quit(void);
    void hev_socks5_tunnel_stats(size_t *tx_packets, size_t *tx_bytes,
                                   size_t *rx_packets, size_t *rx_bytes);
}

// Global state
static volatile bool tunnel_running = false;
static pthread_mutex_t tunnel_mutex = PTHREAD_MUTEX_INITIALIZER;

// Helper function to get FD from FileDescriptor
extern "C" JNIEXPORT jint JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_getFdFromFileDescriptor(
    JNIEnv *env, jobject /* this */, jobject fileDescriptor) {
    
    if (fileDescriptor == nullptr) {
        LOGE("FileDescriptor is null");
        return -1;
    }
    
    jclass fdClass = env->FindClass("java/io/FileDescriptor");
    if (fdClass == nullptr) {
        LOGE("Failed to find FileDescriptor class");
        return -1;
    }
    
    jfieldID fdField = env->GetFieldID(fdClass, "descriptor", "I");
    if (fdField == nullptr) {
        LOGE("Failed to find descriptor field");
        env->DeleteLocalRef(fdClass);
        return -1;
    }
    
    jint fd = env->GetIntField(fileDescriptor, fdField);
    env->DeleteLocalRef(fdClass);
    
    LOGD("Extracted FD: %d", fd);
    return fd;
}

// Write config to temporary file
static std::string writeConfigToTempFile(const std::string &config) {
    char tmpFile[] = "/data/local/tmp/hev-socks5-tunnel-XXXXXX";
    int fd = mkstemp(tmpFile);
    if (fd < 0) {
        LOGE("Failed to create temp file: %s", strerror(errno));
        return "";
    }
    
    ssize_t written = write(fd, config.c_str(), config.length());
    close(fd);
    
    if (written != (ssize_t)config.length()) {
        LOGE("Failed to write config to temp file");
        unlink(tmpFile);
        return "";
    }
    
    LOGI("Config written to: %s", tmpFile);
    return std::string(tmpFile);
}

// Start tunnel from config file
extern "C" JNIEXPORT jint JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeStart(
    JNIEnv *env, jobject /* this */, jstring configPath, jint tunFd) {
    
    pthread_mutex_lock(&tunnel_mutex);
    if (tunnel_running) {
        LOGW("Tunnel is already running");
        pthread_mutex_unlock(&tunnel_mutex);
        return -1;
    }
    tunnel_running = true;
    pthread_mutex_unlock(&tunnel_mutex);
    
    const char *configPathStr = env->GetStringUTFChars(configPath, nullptr);
    if (configPathStr == nullptr) {
        LOGE("Failed to get config path string");
        tunnel_running = false;
        return -1;
    }
    
    LOGI("Starting tunnel with config: %s, TUN FD: %d", configPathStr, tunFd);
    
    // Prepare arguments for hev-socks5-tunnel
    char arg0[] = "hev-socks5-tunnel";
    char arg1[] = "-c";
    char *argv[] = {
        arg0,
        arg1,
        const_cast<char*>(configPathStr),
        nullptr
    };
    
    // Set TUN FD environment variable
    char tunFdEnv[32];
    snprintf(tunFdEnv, sizeof(tunFdEnv), "%d", tunFd);
    setenv("HEV_SOCKS5_TUNNEL_TUN_FD", tunFdEnv, 1);
    
    // Run the tunnel
    int result = hev_socks5_tunnel_main(3, argv);
    
    env->ReleaseStringUTFChars(configPath, configPathStr);
    
    pthread_mutex_lock(&tunnel_mutex);
    tunnel_running = false;
    pthread_mutex_unlock(&tunnel_mutex);
    
    LOGI("Tunnel exited with code: %d", result);
    return result;
}

// Start tunnel from config string
extern "C" JNIEXPORT jint JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeStartFromString(
    JNIEnv *env, jobject /* this */, jstring configYaml, jint tunFd) {
    
    pthread_mutex_lock(&tunnel_mutex);
    if (tunnel_running) {
        LOGW("Tunnel is already running");
        pthread_mutex_unlock(&tunnel_mutex);
        return -1;
    }
    tunnel_running = true;
    pthread_mutex_unlock(&tunnel_mutex);
    
    const char *configStr = env->GetStringUTFChars(configYaml, nullptr);
    if (configStr == nullptr) {
        LOGE("Failed to get config string");
        tunnel_running = false;
        return -1;
    }
    
    LOGI("Starting tunnel with inline config, TUN FD: %d", tunFd);
    LOGD("Config:\n%s", configStr);
    
    // Write config to temporary file
    std::string tempFile = writeConfigToTempFile(configStr);
    env->ReleaseStringUTFChars(configYaml, configStr);
    
    if (tempFile.empty()) {
        LOGE("Failed to create config file");
        tunnel_running = false;
        return -1;
    }
    
    // Prepare arguments
    char arg0[] = "hev-socks5-tunnel";
    char arg1[] = "-c";
    char *argv[] = {
        arg0,
        arg1,
        const_cast<char*>(tempFile.c_str()),
        nullptr
    };
    
    // Set TUN FD environment variable
    char tunFdEnv[32];
    snprintf(tunFdEnv, sizeof(tunFdEnv), "%d", tunFd);
    setenv("HEV_SOCKS5_TUNNEL_TUN_FD", tunFdEnv, 1);
    
    // Run the tunnel
    int result = hev_socks5_tunnel_main(3, argv);
    
    // Clean up temp file
    unlink(tempFile.c_str());
    
    pthread_mutex_lock(&tunnel_mutex);
    tunnel_running = false;
    pthread_mutex_unlock(&tunnel_mutex);
    
    LOGI("Tunnel exited with code: %d", result);
    return result;
}

// Stop the tunnel
extern "C" JNIEXPORT void JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeStop(
    JNIEnv * /* env */, jobject /* this */) {
    
    LOGI("Requesting tunnel stop");
    hev_socks5_tunnel_quit();
}

// Get tunnel statistics
extern "C" JNIEXPORT jlongArray JNICALL
Java_cc_hev_socks5_tunnel_HevSocks5Tunnel_nativeGetStats(
    JNIEnv *env, jobject /* this */) {
    
    size_t tx_packets = 0, tx_bytes = 0, rx_packets = 0, rx_bytes = 0;
    
    pthread_mutex_lock(&tunnel_mutex);
    if (tunnel_running) {
        hev_socks5_tunnel_stats(&tx_packets, &tx_bytes, &rx_packets, &rx_bytes);
    }
    pthread_mutex_unlock(&tunnel_mutex);
    
    jlongArray result = env->NewLongArray(4);
    if (result == nullptr) {
        LOGE("Failed to create stats array");
        return nullptr;
    }
    
    jlong stats[4] = {
        static_cast<jlong>(tx_bytes),
        static_cast<jlong>(rx_bytes),
        static_cast<jlong>(tx_packets),
        static_cast<jlong>(rx_packets)
    };
    
    env->SetLongArrayRegion(result, 0, 4, stats);
    return result;
}

// JNI_OnLoad
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /* reserved */) {
    LOGI("JNI_OnLoad called");
    
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        LOGE("Failed to get JNI environment");
        return JNI_ERR;
    }
    
    LOGI("HevSocks5Tunnel JNI loaded successfully");
    return JNI_VERSION_1_6;
}
