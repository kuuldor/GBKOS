#ifndef __PIME_KERNEL_H__
#define __PIME_KERNEL_H__

#include <PalmOS.h>

// ���뷨�ں˾��
typedef void *PimeKernelHandle;

// �ں˱仯֪ͨ�ص���������
typedef Boolean PimeListenerFunctionType(PimeKernelHandle kernel, void *arg);
typedef PimeListenerFunctionType *PimeListenerFunctionPtr;

// ��ѡ��
typedef void *PimeHintIteratorHandle;

typedef struct tagPimeHintIterator
{
    const Char *hint;
    PimeHintIteratorHandle handle;
} PimeHintIteratorType, *PimeHintIteratorPtr;

// �ں˿���
PimeKernelHandle PimeKernelStart();
void PimeKernelStop(PimeKernelHandle kernel);
PimeKernelHandle PimeKernelGet();

// �ص�����
Boolean PimeKernelListenerRegister(PimeKernelHandle kernel, PimeListenerFunctionPtr pListenerFunc, void *arg);
void PimeKernelListenerUnregister(PimeKernelHandle kernel, PimeListenerFunctionPtr pListenerFunc);

// ��ʼ��
void PimeKernelInit(PimeKernelHandle kernel);

// ���뷨
void PimeKernelImeSwitchToNext(PimeKernelHandle kernel);
const Char *PimeKernelImeGetName(PimeKernelHandle kernel);
const Char *PimeKernelImeGetShortName(PimeKernelHandle kernel);

// ����
void PimeKernelCodeClear(PimeKernelHandle kernel);
Boolean PimeKernelCodeAppend(PimeKernelHandle kernel, Char ch);
Boolean PimeKernelCodeDelete(PimeKernelHandle kernel);
const Char *PimeKernelCodeGet(PimeKernelHandle kernel);

// ��ѡ��
Boolean PimeKernelHintGetFirst(PimeKernelHandle kernel, PimeHintIteratorPtr it);
Boolean PimeKernelHintGetNext(PimeKernelHandle kernel, PimeHintIteratorPtr it);
Boolean PimeKernelHintGetPrevious(PimeKernelHandle kernel, PimeHintIteratorPtr it);

// �������ִ�
void PimeKernelWordSet(PimeKernelHandle kernel, const Char *word);
const Char *PimeKernelWordGet(PimeKernelHandle kernel);

#endif // __PIME_KERNEL_H__