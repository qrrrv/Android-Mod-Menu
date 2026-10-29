#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"
#include "dobby.h"

// Target main lib
#define targetLibName OBFUSCATE("libil2cpp.so")

// Subway Surf 3.70.3: адреса и логика патчей лежат в одном файле
#include "Includes/SsFeatures.hpp"

// Номера функций идут по порядку (Category/ButtonLink не считаются):
// 0 = Не умирать, 1 = Проход сквозь, 2 = Своя скорость, 3 = ползунок скорости
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
            OBFUSCATE("Toggle_Не умирать"),
            OBFUSCATE("Toggle_Проход сквозь"),
            OBFUSCATE("Toggle_Своя скорость"),
            OBFUSCATE("SeekBar_Скорость_110_400"),
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jlong Lvalue, jboolean boolean, jstring text) {
    SsChanges((int) featNum, (int) value, (bool) boolean);
}

// we will run our hacks in a new thread so our while loop doesn't block process main thread
void hack_thread() {
    // libil2cpp.so is not loaded into memory immediately
    while (!isLibraryLoaded(targetLibName)) {
        sleep(1);
    }
    LOGI(OBFUSCATE("Done"));
}

// Functions with `__attribute__((constructor))` are executed immediately when System.loadLibrary("lib_name") is called.
__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
