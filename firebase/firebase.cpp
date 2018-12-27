#include "firebase.h"

#if defined(__ANDROID__)
#include "thread_jandroid.h"
extern jobject _godot_instance;
#endif

firebase::App* Firebase::app_ptr = NULL;

Firebase::Firebase() {
    if(app_ptr == NULL) {
#if defined(__ANDROID__)
        JNIEnv *env = ThreadAndroid::get_env();
        app_ptr = firebase::App::Create(firebase::AppOptions(), env, _godot_instance);
#else 
        app_ptr = firebase::App::Create(firebase::AppOptions());
#endif
    }
}

firebase::App* Firebase::AppId() {
    if(app_ptr == NULL) {
        Firebase *inst = new Firebase();
    }
    return app_ptr;
}

void Firebase::_bind_methods() {
    //ClassDB::bind_method(D_METHOD("AppId"), &Firebase::AppId);
}
