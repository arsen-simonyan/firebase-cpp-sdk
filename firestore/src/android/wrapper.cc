#include "firestore/src/android/wrapper.h"

#include <iterator>

#include "app/src/assert.h"
#include "firestore/src/android/field_path_android.h"
#include "firestore/src/android/field_value_android.h"
#include "firestore/src/android/util_android.h"
#include "firestore/src/include/firebase/firestore.h"
#include "firestore/src/jni/env.h"
#include "firestore/src/jni/hash_map.h"

namespace firebase {
namespace firestore {
namespace {

using jni::Array;
using jni::Env;
using jni::HashMap;
using jni::Local;
using jni::Object;
using jni::String;

}  // namespace

Wrapper::Wrapper(FirestoreInternal* firestore, const Object& obj)
    : firestore_(firestore), obj_(obj) {
  FIREBASE_ASSERT(obj);
}

Wrapper::Wrapper() {
  Firestore* firestore = Firestore::GetInstance();
  FIREBASE_ASSERT(firestore != nullptr);
  firestore_ = firestore->internal_;
  FIREBASE_ASSERT(firestore_ != nullptr);
}

Wrapper::Wrapper(Wrapper* rhs) : Wrapper() {
  if (rhs) {
    firestore_ = rhs->firestore_;
    FIREBASE_ASSERT(firestore_ != nullptr);
    obj_ = rhs->obj_;
  }
}

Wrapper::~Wrapper() = default;

Local<HashMap> Wrapper::MakeJavaMap(Env& env, const MapFieldValue& data) const {
  Local<HashMap> result = HashMap::Create(env);
  for (const auto& kv : data) {
    Local<String> key = env.NewStringUtf(kv.first);
    const Object& value = ToJava(kv.second);
    result.Put(env, key, value);
  }
  return result;
}

Wrapper::UpdateFieldPathArgs Wrapper::MakeUpdateFieldPathArgs(
    Env& env, const MapFieldPathValue& data) const {
  auto iter = data.begin();
  auto end = data.end();
  FIREBASE_DEV_ASSERT_MESSAGE(iter != end, "data must be non-empty");

  Local<Object> first_field = FieldPathConverter::Create(env, iter->first);
  const Object& first_value = ToJava(iter->second);
  ++iter;

  const auto size = std::distance(iter, data.end()) * 2;
  Local<Array<Object>> varargs = env.NewArray(size, Object::GetClass());

  int index = 0;
  for (; iter != end; ++iter) {
    Local<Object> field = FieldPathConverter::Create(env, iter->first);
    const Object& value = ToJava(iter->second);

    varargs.Set(env, index++, field);
    varargs.Set(env, index++, value);
  }

  return UpdateFieldPathArgs{Move(first_field), first_value, Move(varargs)};
}

Object Wrapper::ToJava(const FieldValue& value) {
  return FieldValueInternal::ToJava(value);
}

}  // namespace firestore
}  // namespace firebase
