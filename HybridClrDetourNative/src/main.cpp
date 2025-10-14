#include <hybridclr/transform/Transform.h>
#include <metadata/GenericMetadata.h>
#include <vm/Class.h>
#include <vm/Exception.h>
#include <vm/String.h>
#include <vm/Field.h>
#include <hybridclr/interpreter/InterpreterUtil.h>
#include <hybridclr/transform/TransformContext.h>

#include <iostream>
#include <fmt/format.h>
#include <string>
#include <Zydis/Zydis.h>
#include <Windows.h>

#define EXPORT_FUNC extern "C" __declspec(dllexport)

using namespace hybridclr::interpreter;
using namespace hybridclr::metadata;
using namespace il2cpp::vm;

typedef BYTE byte;

ZyanU64 GetMethodLength(void* code)
{
    std::string disassemblyResult;
    ZyanU64 codeBase = (ZyanU64)code;
    // Initialize decoder context
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    // Initialize formatter. Only required when you actually plan to do instruction
    // formatting ("disassembling"), like we do here
    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    // Loop over the instructions in our buffer.
    // The runtime-address (instruction pointer) is chosen arbitrary here in order to better
    // visualize relative addressing
    ZyanU64 runtime_address = codeBase;
    ZyanUSize offset = 0;
    const ZyanUSize length = INT_MAX;
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
    while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, (void*)(codeBase + offset), length - offset, &instruction, operands)))
    {
        // Format & print the binary instruction structure to human-readable format
        char buffer[256];
        ZydisFormatterFormatInstruction(&formatter, &instruction, operands,
            instruction.operand_count_visible, buffer, sizeof(buffer), runtime_address, ZYAN_NULL);

        if (!strcmp(buffer, "int3") || !strcmp(buffer, "nop")) {
            break;
        }

        disassemblyResult += fmt::format("\n-- 0x{:08x}  {}", runtime_address, buffer);

        offset += instruction.length;
        runtime_address += instruction.length;
    }
    std::cout << disassemblyResult << std::endl;
    return offset;
}

int GetAllocPageSize() {
    static int lastRet = -1;
    if (lastRet == -1) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        lastRet = si.dwAllocationGranularity; // should be used with VirtualAlloc(MEM_RESERVE)
    }
    return lastRet;
}

int GetPageSize() {
    static int lastRet = -1;
    if (lastRet == -1) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        lastRet = si.dwPageSize; // should be used with VirtualAlloc(MEM_RESERVE)
    }
    return lastRet;
}

void* DuplicateMethod(void* code)
{
    ZyanU64 length = GetMethodLength(code);
    // reserve space for dobby
    void* reserved = VirtualAlloc(nullptr, length + GetAllocPageSize(), MEM_RESERVE, PAGE_NOACCESS);
    if (!reserved) {
        std::cout << "could not allocate reserved memory!" << std::endl;
        return nullptr;
    }
    VirtualFree(reserved, 0, MEM_RELEASE);
    void* newCode = VirtualAlloc(reserved, length, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!newCode) {
        std::cout << "could not allocate code memory!" << std::endl;
        return nullptr;
    }
    memcpy(newCode, code, length);
    if (VirtualProtect(newCode, length, PAGE_EXECUTE_READ, NULL)) {
        std::cout << "page could not change to PAGE_EXECUTE_READ: " << GetLastError() << std::endl;
    }
    return newCode;
}

InvokerMethod originInterpterInvoke = nullptr;

std::string GetMethodFullName(const MethodInfo* method)
{
    if (method->klass) {
        return std::string(method->klass->name) + "::" + method->name;
    }
    return method->name;
}

#ifdef HYBRIDCLR_UNITY_2021_OR_NEW

void RuntimeInvoker_TrueVoid_t4861ACF8F4594C3437BB48B6E56783494B843915(Il2CppMethodPointer methodPointer, const RuntimeMethod* methodMetadata, void* obj, void** args, void* returnAddress)
{
    typedef void (*Func)(void* obj, const RuntimeMethod* method);
    ((Func)methodPointer)(obj, methodMetadata);
}

static void InterpterInvoke(Il2CppMethodPointer methodPointer, const MethodInfo* method, void* __this, void** __args, void* __ret)
{
    bool isInstanceMethod = IsInstanceMethod(method);
    bool isRetVoid = IsReturnVoidMethod(method);
    if (isRetVoid && isInstanceMethod && method->parameters_count == 0) {
        RuntimeInvoker_TrueVoid_t4861ACF8F4594C3437BB48B6E56783494B843915(methodPointer, method, __this, __args, __ret);
        return;
    }

    std::cout << "method " << GetMethodFullName(method) << " fail to replace invoker_method!" << std::endl;
    originInterpterInvoke(methodPointer, method, __this, __args, __ret);
}

#else

#endif

bool inited = false;
void Setup() {
    if (inited)
        return;

    inited = true;

    freopen("CONOUT$", "w", stdout);
}

EXPORT_FUNC void PrepareDetourForHotfixMethod(MethodInfo* methodInfo)
{
    Setup();
    if (!methodInfo->isInterpterImpl) {
        std::cout << "method " << GetMethodFullName(methodInfo) << " is not a hotfix method" << std::endl;
        return;
    }
    std::cout << "method " << GetMethodFullName(methodInfo) << " is converting to native method" << std::endl;
    void* newCode = DuplicateMethod(methodInfo->methodPointer);
    if (!newCode) {
        return;
    }

    std::cout << fmt::format("method 0x{:08x} is duplicate to 0x{:08x}", (uint64_t)methodInfo->methodPointer, (uint64_t)newCode) << std::endl;

    bool isRetVoid = IsReturnVoidMethod(methodInfo);
    bool resolvedIsInstanceMethod = IsInstanceMethod(methodInfo);
    InterpMethodInfo* imi = (InterpMethodInfo*)methodInfo->interpData;
    void* newIlCode = nullptr;
    if (isRetVoid) {

    }
    else {
        LocationDataType locDataType = GetLocationDataTypeByType(methodInfo->return_type);
        if (IsNeedExpandLocationType(locDataType)) {
            //newIlCode = new byte[imi->codeLength + sizeof(IRCallNativeInstance_ret_expand)];
            //IRCallNativeInstance_ret_expand* ir = (IRCallNativeInstance_ret_expand*)newIlCode;
            //ir->type = resolvedIsInstanceMethod ? HiOpcodeEnum::CallNativeInstance_ret_expand : HiOpcodeEnum::CallNativeStatic_ret_expand;
            //ir->managed2NativeMethod = managed2NativeMethodDataIdx;
            //ir->methodInfo = methodDataIndex;
            //ir->argIdxs = argIdxDataIndex;
            //ir->ret = ctx.GetEvalStackTopOffset();
            //ir->retLocationType = (uint8_t)locDataType;
        }
        else {

        }
    }

    //VirtualInvokeData* virtualInvokeData = nullptr;
    //if (hybridclr::metadata::IsVirtualMethod(methodInfo->flags))
    //{
    //    if (!methodInfo->klass->initialized) {

    //        Class::SetupMethods(methodInfo->klass);
    //    }
    //    virtualInvokeData = &methodInfo->klass->vtable[methodInfo->slot];
    //}
    //if (virtualInvokeData) {
    //    if (virtualInvokeData->methodPtr == methodInfo->methodPointer) {
    //        virtualInvokeData->methodPtr = (Il2CppMethodPointer)newCode;
    //    }
    //    else {
    //        std::cout << "unexpected: virtual method " << GetMethodFullName(methodInfo) << " virtualInvokeData->methodPtr != methodInfo->methodPointer" << std::endl;
    //    }
    //}

    methodInfo->methodPointer = (Il2CppMethodPointer)newCode;
    methodInfo->virtualMethodPointer = (Il2CppMethodPointer)newCode;
    methodInfo->methodPointerCallByInterp = (Il2CppMethodPointer)newCode;
    methodInfo->virtualMethodPointerCallByInterp = (Il2CppMethodPointer)newCode;
    if (!originInterpterInvoke) {
        originInterpterInvoke = methodInfo->invoker_method;
    }
    methodInfo->invoker_method = &InterpterInvoke;
    methodInfo->isInterpterImpl = false;
}


