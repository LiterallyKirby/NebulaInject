#include "Field.h"
#include "jni.h"

jobject Field::GetObjectField(JNIEnv* env, void* fieldOwner, bool staticField)
{
    if (!env || !fieldOwner) return nullptr;

    return staticField
        ? env->GetStaticObjectField((jclass)fieldOwner, (jfieldID)this)
        : env->GetObjectField((jobject)fieldOwner, (jfieldID)this);
}

float Field::GetFloatField(JNIEnv* env, void* fieldOwner, bool staticField)
{
    if (!env || !fieldOwner) return 0.0f;

    return staticField
        ? env->GetStaticFloatField((jclass)fieldOwner, (jfieldID)this)
        : env->GetFloatField((jobject)fieldOwner, (jfieldID)this);
}

double Field::GetDoubleField(JNIEnv* env, void* fieldOwner, bool staticField)
{
    if (!env || !fieldOwner) return 0.0;

    return staticField
        ? env->GetStaticDoubleField((jclass)fieldOwner, (jfieldID)this)
        : env->GetDoubleField((jobject)fieldOwner, (jfieldID)this);
}

int Field::GetIntField(JNIEnv* env, void* fieldOwner, bool staticField)
{
    if (!env || !fieldOwner) return 0;

    return staticField
        ? env->GetStaticIntField((jclass)fieldOwner, (jfieldID)this)
        : env->GetIntField((jobject)fieldOwner, (jfieldID)this);
}

bool Field::GetBooleanField(JNIEnv* env, void* fieldOwner, bool staticField)
{
    if (!env || !fieldOwner) return false;

    return staticField
        ? env->GetStaticBooleanField((jclass)fieldOwner, (jfieldID)this)
        : env->GetBooleanField((jobject)fieldOwner, (jfieldID)this);
}

void Field::SetObjectField(JNIEnv* env, void* fieldOwner, jobject buffer, bool staticField)
{
    if (!env || !fieldOwner) return;

    if (staticField)
        env->SetStaticObjectField((jclass)fieldOwner, (jfieldID)this, buffer);
    else
        env->SetObjectField((jobject)fieldOwner, (jfieldID)this, buffer);
}

void Field::SetFloatField(JNIEnv* env, void* fieldOwner, float buffer, bool staticField)
{
    if (!env || !fieldOwner) return;

    if (staticField)
        env->SetStaticFloatField((jclass)fieldOwner, (jfieldID)this, buffer);
    else
        env->SetFloatField((jobject)fieldOwner, (jfieldID)this, buffer);
}

void Field::SetDoubleField(JNIEnv* env, void* fieldOwner, double buffer, bool staticField)
{
    if (!env || !fieldOwner) return;

    if (staticField)
        env->SetStaticDoubleField((jclass)fieldOwner, (jfieldID)this, buffer);
    else
        env->SetDoubleField((jobject)fieldOwner, (jfieldID)this, buffer);
}

void Field::SetIntField(JNIEnv* env, void* fieldOwner, int buffer, bool staticField)
{
    if (!env || !fieldOwner) return;

    if (staticField)
        env->SetStaticIntField((jclass)fieldOwner, (jfieldID)this, buffer);
    else
        env->SetIntField((jobject)fieldOwner, (jfieldID)this, buffer);
}

void Field::SetBooleanField(JNIEnv* env, void* fieldOwner, bool buffer, bool staticField)
{
    if (!env || !fieldOwner) return;

    if (staticField)
        env->SetStaticBooleanField((jclass)fieldOwner, (jfieldID)this, buffer);
    else
        env->SetBooleanField((jobject)fieldOwner, (jfieldID)this, buffer);
}
