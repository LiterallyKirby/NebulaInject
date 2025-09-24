//
// This code was copied from UDP-CPP:
// https://github.com/UnknownDetectionParty/UDP-CPP
//

#ifndef PHANTOM_ABSTRACTCLASS_H
#define PHANTOM_ABSTRACTCLASS_H

#include "../Phantom.h"
#include "CM.h"
#include "Mapping.h"
#include <jvmti.h>

class AbstractClass {
public:
  AbstractClass(Phantom *phantom, const char *clsName);

  // This stuff is just to make using JNI easier (it's already easy, just likes
  // to be easier) It doesn't cover everything, but covers most basic shit

  jobject getClassLoader() const;
  jclass getClass(const char *clsName) const;
  jclass getClass() const;

  // Boolean
  template <class T>
  jboolean getBoolean(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallBooleanMethod(parent, method, values);
  }
  template <class T> jboolean getBoolean(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticBooleanMethod(cls, method, values);
  }
  jboolean getBoolean(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallBooleanMethod(parent, method);
  }
  jboolean getBoolean(jmethodID method) const {
    return phantom->getEnv()->CallStaticBooleanMethod(cls, method);
  }
  jboolean getBoolean(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetBooleanField(parent, field);
  }
  jboolean getBoolean(jfieldID field) const {
    return phantom->getEnv()->GetStaticBooleanField(cls, field);
  }

  // Byte
  template <class T>
  jbyte getByte(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallByteMethod(parent, method, values);
  }
  template <class T> jbyte getByte(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticByteMethod(cls, method, values);
  }
  jbyte getByte(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallByteMethod(parent, method);
  }
  jbyte getByte(jmethodID method) const {
    return phantom->getEnv()->CallStaticByteMethod(cls, method);
  }
  jbyte getByte(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetByteField(parent, field);
  }
  jbyte getByte(jfieldID field) const {
    return phantom->getEnv()->GetStaticByteField(cls, field);
  }

  // Char
  template <class T>
  jchar getChar(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallCharMethod(parent, method, values);
  }
  template <class T> jchar getChar(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticCharMethod(cls, method, values);
  }
  jchar getChar(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallCharMethod(parent, method);
  }
  jchar getChar(jmethodID method) const {
    return phantom->getEnv()->CallStaticCharMethod(cls, method);
  }
  jchar getChar(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetCharField(parent, field);
  }
  jchar getChar(jfieldID field) const {
    return phantom->getEnv()->GetStaticCharField(cls, field);
  }

  // Short
  template <class T>
  jshort getShort(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallShortMethod(parent, method, values);
  }
  template <class T> jshort getShort(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticShortMethod(cls, method, values);
  }
  jshort getShort(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallShortMethod(parent, method);
  }
  jshort getShort(jmethodID method) const {
    return phantom->getEnv()->CallStaticShortMethod(cls, method);
  }
  jshort getShort(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetShortField(parent, field);
  }
  jshort getShort(jfieldID field) const {
    return phantom->getEnv()->GetStaticShortField(cls, field);
  }

  // Int
  template <class T>
  jint getInt(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallIntMethod(parent, method, values);
  }
  template <class T> jint getInt(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticIntMethod(cls, method, values);
  }
  jint getInt(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallIntMethod(parent, method);
  }
  jint getInt(jmethodID method) const {
    return phantom->getEnv()->CallStaticIntMethod(cls, method);
  }
  jint getInt(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetIntField(parent, field);
  }
  jint getInt(jfieldID field) const {
    return phantom->getEnv()->GetStaticIntField(cls, field);
  }

  // Long
  template <class T>
  jlong getLong(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallLongMethod(parent, method, values);
  }
  template <class T> jlong getLong(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticLongMethod(cls, method, values);
  }
  jlong getLong(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallLongMethod(parent, method);
  }
  jlong getLong(jmethodID method) const {
    return phantom->getEnv()->CallStaticLongMethod(cls, method);
  }
  jlong getLong(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetLongField(parent, field);
  }
  jlong getLong(jfieldID field) const {
    return phantom->getEnv()->GetStaticLongField(cls, field);
  }

  // Float
  template <class T>
  jfloat getFloat(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallFloatMethod(parent, method, values);
  }
  template <class T> jfloat getFloat(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticFloatMethod(cls, method, values);
  }
  jfloat getFloat(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallFloatMethod(parent, method);
  }
  jfloat getFloat(jmethodID method) const {
    return phantom->getEnv()->CallStaticFloatMethod(cls, method);
  }
  jfloat getFloat(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetFloatField(parent, field);
  }
  jfloat getFloat(jfieldID field) const {
    return phantom->getEnv()->GetStaticFloatField(cls, field);
  }

  // Double
  template <class T>
  jdouble getDouble(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallDoubleMethod(parent, method, values);
  }
  template <class T> jdouble getDouble(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticDoubleMethod(cls, method, values);
  }
  jdouble getDouble(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallDoubleMethod(parent, method);
  }
  jdouble getDouble(jmethodID method) const {
    return phantom->getEnv()->CallStaticDoubleMethod(cls, method);
  }
  jdouble getDouble(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetDoubleField(parent, field);
  }
  jdouble getDouble(jfieldID field) const {
    return phantom->getEnv()->GetStaticDoubleField(cls, field);
  }

  // Object
  template <class T>
  jobject getObject(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallObjectMethod(parent, method, values);
  }
  template <class T> jobject getObject(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticObjectMethod(cls, method, values);
  }
  jobject getObject(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallObjectMethod(parent, method);
  }
  jobject getObject(jmethodID method) const {
    return phantom->getEnv()->CallStaticObjectMethod(cls, method);
  }
  jobject getObject(jobject parent, jfieldID field) const {
    return phantom->getEnv()->GetObjectField(parent, field);
  }
  jobject getObject(jfieldID field) const {
    return phantom->getEnv()->GetStaticObjectField(cls, field);
  }

  // Array
  int getArrayLength(jobjectArray arr) const {
    return phantom->getEnv()->GetArrayLength(arr);
  }
  jobject getArrayElement(jobjectArray arr, jsize index) const {
    return phantom->getEnv()->GetObjectArrayElement(arr, index);
  }

  // Void
  template <class T>
  void callMethod(jobject parent, jmethodID method, T values...) const {
    return phantom->getEnv()->CallVoidMethod(parent, method, values);
  }
  template <class T> void callMethod(jmethodID method, T values...) const {
    return phantom->getEnv()->CallStaticVoidMethod(cls, method, values);
  }
  void callMethod(jobject parent, jmethodID method) const {
    return phantom->getEnv()->CallVoidMethod(parent, method);
  }
  void callMethod(jmethodID method) const {
    return phantom->getEnv()->CallStaticVoidMethod(cls, method);
  }

  void setBoolean(jobject parent, jfieldID field, jboolean value) {
    phantom->getEnv()->SetBooleanField(parent, field, value);
  }
  void setBoolean(jfieldID field, jboolean value) {
    phantom->getEnv()->SetStaticBooleanField(cls, field, value);
  }

  void setByte(jobject parent, jfieldID field, jbyte value) {
    phantom->getEnv()->SetByteField(parent, field, value);
  }
  void setByte(jfieldID field, jbyte value) {
    phantom->getEnv()->SetStaticByteField(cls, field, value);
  }

  void setChar(jobject parent, jfieldID field, jchar value) {
    phantom->getEnv()->SetCharField(parent, field, value);
  }
  void setChar(jfieldID field, jchar value) {
    phantom->getEnv()->SetStaticCharField(cls, field, value);
  }

  void setShort(jobject parent, jfieldID field, jshort value) {
    phantom->getEnv()->SetShortField(parent, field, value);
  }
  void setShort(jfieldID field, jshort value) {
    phantom->getEnv()->SetStaticShortField(cls, field, value);
  }

  void setInt(jobject parent, jfieldID field, jint value) {
    phantom->getEnv()->SetIntField(parent, field, value);
  }
  void setInt(jfieldID field, jint value) {
    phantom->getEnv()->SetStaticIntField(cls, field, value);
  }

  void setLong(jobject parent, jfieldID field, jlong value) {
    phantom->getEnv()->SetLongField(parent, field, value);
  }
  void setLong(jfieldID field, jlong value) {
    phantom->getEnv()->SetStaticLongField(cls, field, value);
  }

  void setFloat(jobject parent, jfieldID field, jfloat value) {
    phantom->getEnv()->SetFloatField(parent, field, value);
  }
  void setFloat(jfieldID field, jfloat value) {
    phantom->getEnv()->SetStaticFloatField(cls, field, value);
  }

  void setDouble(jobject parent, jfieldID field, jdouble value) {
    phantom->getEnv()->SetDoubleField(parent, field, value);
  }
  void setDouble(jfieldID field, jdouble value) {
    phantom->getEnv()->SetStaticDoubleField(cls, field, value);
  }

  void setObject(jobject parent, jfieldID field, jobject value) {
    phantom->getEnv()->SetObjectField(parent, field, value);
  }
  void setObject(jfieldID field, jobject value) {
    phantom->getEnv()->SetStaticObjectField(cls, field, value);
  }

protected:
  const char *clsKey;
  jclass cls;
  Phantom *phantom;
  // Field getter that uses the mapping class so only a clear-text name needs to
  // be defined. "name" : Clear-text name used by 'Mapping.h' to define the
  // field. Return: JNI field wrapper

 


jfieldID getFieldID(const char* name) {
    CM* cm = Mapping::getClass(clsKey);
    if (!cm) {
        std::cerr << "[getFieldID] Mapping::getClass failed for " << clsKey << "\n";
        return nullptr;
    }

    if (cm->fields.empty()) {
        std::cerr << "[getFieldID] Fields map is empty for class: " << clsKey << "\n";
        return nullptr;
    }

    auto it = cm->fields.find(std::string(name));
    if (it == cm->fields.end()) {
        std::cerr << "[getFieldID] Field not found: " << name
                  << " in class " << clsKey << "\n";
        return nullptr;
    }

    Mem& field = it->second;
    return getFieldID(field.name, field.desc, field.isStatic);
}




  // Method getter that uses the mapping class so only a clear-text name needs
  // to be defined. "name" : Clear-text name used by 'Mapping.h' to define the
  // method. Return: JNI method wrapper

  jmethodID getMethodID(const char *name) {
    CM *cm = Mapping::getClass(clsKey);

    auto it = cm->methods.find(std::string(name));
    if (it == cm->methods.end()) {
      // Method not found, handle gracefully
      // Could log, throw, or return nullptr
      return nullptr;
    }

    Mem &method = it->second;
    return getMethodID(method.name, method.desc, method.isStatic);
  }
protected:
    bool isValidField(jfieldID field) const {
        return field != nullptr;
    }
    
    bool isValidMethod(jmethodID method) const {
        return method != nullptr;
    }
private:
  // Return: JNI field wrapper
  jfieldID getFieldID(const char *name, const char *sig, bool _static) {
    return _static ? phantom->getEnv()->GetStaticFieldID(cls, name, sig)
                   : phantom->getEnv()->GetFieldID(cls, name, sig);
  }
  // Return: JNI method wrapper
  jmethodID getMethodID(const char *name, const char *sig, bool _static) {
    return _static ? phantom->getEnv()->GetStaticMethodID(cls, name, sig)
                   : phantom->getEnv()->GetMethodID(cls, name, sig);
  }
};

#endif // PHANTOM_ABSTRACTCLASS_H
