#include <pch/pch-cpp.hpp>
// 有需要的实现复制到这里
// 注意访问全局变量或类静态变量的情况 这里不会访问到GameAssembly的变量

#include <hybridclr/interpreter/InterpreterUtil.h>
#include <vm/Object.h>
#include <vm/Image.h>
#include <hybridclr/metadata/MetadataUtil.h>
#define IL2CPP_EXPORT __declspec(dllimport)
#include <il2cpp-api.h>
#include <il2cpp-mono-api.h>

// 调用GameAssembly导出的符号

// Class
namespace il2cpp
{
    namespace vm
    {
        Il2CppClass* Class::FromIl2CppType(const Il2CppType* type, bool throwOnError)
        {
            return il2cpp_class_from_il2cpp_type(type);
        }

        int32_t Class::GetValueSize(Il2CppClass* klass, uint32_t* align)
        {
            return il2cpp_class_value_size(klass, align);
        }
    }
}

// Object
namespace il2cpp
{
    namespace vm
    {
        Il2CppObject* Object::Box(Il2CppClass* typeInfo, void* val)
        {
            return il2cpp_value_box(typeInfo, val);
        }
    }
}

// Image
namespace il2cpp
{
    namespace vm
    {
        Il2CppImage* Image::GetCorlib()
        {
            return (Il2CppImage*)il2cpp_get_corlib();
        }
    }
}

// Exception
namespace il2cpp
{
    namespace vm
    {
        NORETURN void Exception::Raise(Il2CppException* ex, MethodInfo* lastManagedFrame)
        {
            return il2cpp_raise_exception(ex);
        }

        Il2CppException* Exception::FromNameMsg(const Il2CppImage* image, const char* name_space, const char* name, const char* msg)
        {
            return il2cpp_exception_from_name_msg(image, name_space, name, msg);
        }

        Il2CppException* Exception::GetExecutionEngineException(const char* msg)
        {
            return Exception::FromNameMsg(Image::GetCorlib(), "System", "ExecutionEngineException", msg);
        }
    }
}

// 以下直接复制原实现

#include <hybridclr/interpreter/InterpreterUtil.cpp>

namespace hybridclr
{
    namespace metadata
    {
        bool IsValueType(const Il2CppType* type)
        {
            switch (type->type)
            {
            case IL2CPP_TYPE_BOOLEAN:
            case IL2CPP_TYPE_I1:
            case IL2CPP_TYPE_U1:
            case IL2CPP_TYPE_CHAR:
            case IL2CPP_TYPE_I2:
            case IL2CPP_TYPE_U2:
            case IL2CPP_TYPE_I4:
            case IL2CPP_TYPE_U4:
            case IL2CPP_TYPE_R4:
            case IL2CPP_TYPE_I8:
            case IL2CPP_TYPE_U8:
            case IL2CPP_TYPE_R8:
            case IL2CPP_TYPE_I:
            case IL2CPP_TYPE_U:
            case IL2CPP_TYPE_TYPEDBYREF:
            case IL2CPP_TYPE_VALUETYPE: return true;
            case IL2CPP_TYPE_GENERICINST: return type->data.generic_class->type->type == IL2CPP_TYPE_VALUETYPE;
            default: return false;
            }
        }
    }
}
