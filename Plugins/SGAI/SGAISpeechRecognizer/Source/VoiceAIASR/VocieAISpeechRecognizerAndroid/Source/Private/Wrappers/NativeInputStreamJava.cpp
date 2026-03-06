#include "NativeInputStreamJava.h"

extern "C" JNIEXPORT int JNICALL Java_com_sgs_gameai_NativeInputStream_nativeRead(JNIEnv *env, jclass /*clazz*/,
                                                                                  jlong handle, jbyteArray buffer,
                                                                                  jint offset, jint length)
{
    int returnValue = 0;
    IInputStream *inputStream = (IInputStream *)(handle);
    if (inputStream != nullptr)
    {
        jbyte *BufferPtr = env->GetByteArrayElements(buffer, NULL);
        returnValue = inputStream->Read((uint8 *)BufferPtr, offset, length);
        env->ReleaseByteArrayElements(buffer, BufferPtr, 0);
    }
    return returnValue;
}

FNativeInputStreamJava::FNativeInputStreamJava(IInputStream *ptr)
    : FJavaClassObject("com/sgs/gameai/NativeInputStream", "(J)V", (long)(ptr))
{
}

FNativeInputStreamJava::~FNativeInputStreamJava()
{
}
