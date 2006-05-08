#ifndef __PIME_KERNEL_H__
#define __PIME_KERNEL_H__

#include <PalmOS.h>

// 输入法内核句柄
typedef void *PimeKernelHandle;

// 内核变化通知回调函数类型
typedef Boolean PimeListenerFunctionType(PimeKernelHandle kernel, void *arg);
typedef PimeListenerFunctionType *PimeListenerFunctionPtr;

// 候选字
typedef void *PimeHintIteratorHandle;

typedef struct tagPimeHintIterator
{
    const Char *hint;
    PimeHintIteratorHandle handle;
} PimeHintIteratorType, *PimeHintIteratorPtr;

// 内核控制
PimeKernelHandle PimeKernelStart();
void PimeKernelStop(PimeKernelHandle kernel);
PimeKernelHandle PimeKernelGet();

// 回调函数
Boolean PimeKernelListenerRegister(PimeKernelHandle kernel, PimeListenerFunctionPtr pListenerFunc, void *arg);
void PimeKernelListenerUnregister(PimeKernelHandle kernel, PimeListenerFunctionPtr pListenerFunc);

// 初始化
void PimeKernelInit(PimeKernelHandle kernel);

// 输入法
void PimeKernelImeSwitchToNext(PimeKernelHandle kernel);
const Char *PimeKernelImeGetName(PimeKernelHandle kernel);
const Char *PimeKernelImeGetShortName(PimeKernelHandle kernel);

// 编码
void PimeKernelCodeClear(PimeKernelHandle kernel);
Boolean PimeKernelCodeAppend(PimeKernelHandle kernel, Char ch);
Boolean PimeKernelCodeDelete(PimeKernelHandle kernel);
const Char *PimeKernelCodeGet(PimeKernelHandle kernel);

// 候选字
Boolean PimeKernelHintGetFirst(PimeKernelHandle kernel, PimeHintIteratorPtr it);
Boolean PimeKernelHintGetNext(PimeKernelHandle kernel, PimeHintIteratorPtr it);
Boolean PimeKernelHintGetPrevious(PimeKernelHandle kernel, PimeHintIteratorPtr it);

// 可输入字词
void PimeKernelWordSet(PimeKernelHandle kernel, const Char *word);
const Char *PimeKernelWordGet(PimeKernelHandle kernel);

#endif // __PIME_KERNEL_H__