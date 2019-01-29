#include "auth.h"

bool FirebaseAuth::inited = false;
firebase::auth::Auth* FirebaseAuth::auth = NULL;
firebase::auth::User* FirebaseAuth::user = NULL;
firebase::auth::User::UserProfile FirebaseAuth::profile;

FirebaseAuth::FirebaseAuth() {
    if(!inited) {
        firebase::App* app = Firebase::AppId();
        if(app != NULL) {
            auth = firebase::auth::Auth::GetAuth(app);
            inited = true;
        }
    }
}

void FirebaseAuth::OnCreateUserCallback(const firebase::Future<firebase::auth::User*>& result, void* user_data) {
    // The callback is called when the Future enters the `complete` state.
    assert(result.status() == firebase::kFutureStatusComplete);
    if (result.error() == firebase::auth::kAuthErrorNone) {
        user = *result.result();
        print_line(String("Create user succeeded with name ") + user->display_name().c_str());
        user->UpdateUserProfile(profile);
        emit_signal("logged_in");
    } else {
        FirebaseAuth::user = NULL;
        print_line(String("Created user failed with error ") + result.error_message());
    }
}

void FirebaseAuth::sign_in_anonymously()
{
    print_line("Start anonymous sign in");
    firebase::Future<firebase::auth::User*> result = auth->SignInAnonymously();
    result.OnCompletion([](const firebase::Future<firebase::auth::User*>& result, void* user_data) {
                            ((FirebaseAuth*)user_data)->OnCreateUserCallback(result, user_data);
                        }, this);
}

void FirebaseAuth::sign_in_facebook(String token)
{
    print_line("Start sign in to Facebook");
    firebase::auth::Credential credential = firebase::auth::FacebookAuthProvider::GetCredential(token.utf8().ptr());
    firebase::auth::User* current_user = auth->current_user();
    if(current_user != NULL) {
        // link facebook account
        firebase::Future<firebase::auth::User*> result = current_user->LinkWithCredential(credential);
        result.OnCompletion([](const firebase::Future<firebase::auth::User*>& result, void* user_data) {
                                ((FirebaseAuth*)user_data)->OnCreateUserCallback(result, user_data);
                            }, this);
    } else {
        // regular sign in
        firebase::Future<firebase::auth::User*> result = auth->SignInWithCredential(credential);
        result.OnCompletion([](const firebase::Future<firebase::auth::User*>& result, void* user_data) {
                                ((FirebaseAuth*)user_data)->OnCreateUserCallback(result, user_data);
                            }, this);
    }
    //firebase::Future<firebase::auth::User*> result = auth->SignInWithCredentialLastResult();
}

void FirebaseAuth::unlink_facebook()
{
    firebase::auth::User* current_user = auth->current_user();
    firebase::Future<firebase::auth::User*> result = current_user->Unlink("Facebook");
}

bool FirebaseAuth::is_logged_in()
{
    return user != NULL;
}

String FirebaseAuth::user_name()
{
    return String(user->display_name().c_str());
}

String FirebaseAuth::email()
{
    return String(user->email().c_str());
}

String FirebaseAuth::uid()
{
    return String(user->uid().c_str());
}

void FirebaseAuth::sign_out()
{
    firebase::auth::SignOut();
}

void FirebaseAuth::_bind_methods() {
    ClassDB::bind_method(D_METHOD("sign_in_anonymously"), &FirebaseAuth::sign_in_anonymously);
    ClassDB::bind_method(D_METHOD("sign_in_facebook", "param"), &FirebaseAuth::sign_in_facebook);
    ClassDB::bind_method(D_METHOD("unlink_facebook"), &FirebaseAuth::unlink_facebook);
    ClassDB::bind_method(D_METHOD("is_logged_in"), &FirebaseAuth::is_logged_in);
    ClassDB::bind_method(D_METHOD("user_name"), &FirebaseAuth::user_name);
    ClassDB::bind_method(D_METHOD("email"), &FirebaseAuth::email);
    ClassDB::bind_method(D_METHOD("uid"), &FirebaseAuth::uid);
    ClassDB::bind_method(D_METHOD("sign_out"), &FirebaseAuth::sign_out);
    ADD_SIGNAL(MethodInfo("logged_in"));
}


