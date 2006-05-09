#include "PimeKernel.h"

#define FTR_CRID 'PIme'
#define FTR_KERNEL_NUMBER 0

typedef struct tagPrvListener
{
    PimeListenerFunctionPtr func;
    void *arg;
    struct tagPrvListener* next;
} PrvListenerType, *PrvListenerPtr;

typedef struct tagPrvKernel
{
    Char currentImeName[16];
    Char currentImeShortName[5];
    Char *currentCode;
    Char *currentWord;
    
    PrvListenerPtr listeners;
    
    // ��ֹ�ݹ���õı�־����ֹ��NotifyʱListener�ص���������Kernel������������µ�״̬�仯�����µݹ���á�
    // ������Ͻ�����Listener�ص�������ֻ������û��״̬��Kernel�ӿں�����
    Boolean notifying;
    
} PrvKernelType, *PrvKernelPtr;

#define KERNEL_H(x) ((PimeKernelHandle)x)
#define KERNEL_P(x) ((PrvKernelPtr)x)

static void PrvNotifyListeners(PrvKernelPtr kptr);
static void PrvMatchWord(PrvKernelPtr kptr);

static void PrvFreeListeners(PrvListenerPtr lptr);

// �ں˿���
PimeKernelHandle PimeKernelStart()
{
    PrvKernelPtr kptr;
    
    if (PimeKernelGet() != NULL) return KERNEL_H(NULL); // ���Kernel�Ѿ����������򷵻�NULL��
    
    kptr = (PrvKernelPtr)MemPtrNew(sizeof(PrvKernelType));
    
    if (kptr)
    {
        kptr->currentCode = (Char *)MemPtrNew(5); // �������뷨���4���ַ�����ӽ�β0��
        kptr->currentWord = (Char *)MemPtrNew(3); // �������뷨���2���ַ���һ�����֣��� ��ӽ�β0��
        
        kptr->listeners = NULL;
        kptr->notifying = false;
    }
    
    FtrSet(FTR_CRID, FTR_KERNEL_NUMBER, (UInt32)kptr);
    
    return KERNEL_H(kptr);
}

void PimeKernelStop(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    if (kptr)
    {
        PrvFreeListeners(kptr->listeners);
        
        MemPtrFree(kptr->currentWord);
        MemPtrFree(kptr->currentCode);
        
        MemPtrFree(kptr);
        
        FtrUnregister(FTR_CRID, FTR_KERNEL_NUMBER);
    }
}

PimeKernelHandle PimeKernelGet()
{
    PrvKernelPtr kptr;
    
    if (FtrGet(FTR_CRID, FTR_KERNEL_NUMBER, (UInt32 *)(&kptr)) != errNone)
        return KERNEL_H(0);
    
    return KERNEL_H(kptr);
}

// �ص�����
Boolean PimeKernelListenerRegister(PimeKernelHandle kernel, PimeListenerFunctionPtr pListenerFunc, void *arg)
{
    Boolean result = false;
    PrvKernelPtr kptr = KERNEL_P(kernel);
    PrvListenerPtr newPtr = (PrvListenerPtr)MemPtrNew(sizeof(PrvListenerType));
    
    if (newPtr != NULL)
    {
        newPtr->func = pListenerFunc;
        newPtr->arg = arg;
        newPtr->next = kptr->listeners;
        kptr->listeners = newPtr;
        
        result = true;
    }
    
    return result;
}

void PimeKernelListenerUnregister(PimeKernelHandle kernel, PimeListenerFunctionPtr pListenerFunc)
{
    PrvListenerPtr matchedPtr = NULL;
    PrvListenerPtr lptr = KERNEL_P(kernel)->listeners;
    
    while (lptr != NULL)
    {
        if (lptr->func == pListenerFunc)
        {
            matchedPtr = lptr;
            lptr = lptr->next;
            
            MemPtrFree(matchedPtr);
            
            break;
        }
        
        lptr = lptr->next;
    }
}

// ��ʼ��
void PimeKernelInit(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    StrCopy(kptr->currentImeName, "�������뷨");
    StrCopy(kptr->currentImeShortName, "����");
    StrCopy(kptr->currentCode, "");
    StrCopy(kptr->currentWord, "");
    
    PrvNotifyListeners(kptr);
}

// ���뷨
void PimeKernelImeSwitchToNext(PimeKernelHandle kernel)
{
    // ֻ���������뷨��û���������뷨���л���
}

const Char *PimeKernelImeGetName(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    return kptr->currentImeName;
}

const Char *PimeKernelImeGetShortName(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    return kptr->currentImeShortName;
}

// ����
void PimeKernelCodeClear(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    StrCopy(kptr->currentCode, "");
    
    PrvNotifyListeners(kptr);
}

Boolean PimeKernelCodeAppend(PimeKernelHandle kernel, Char ch)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    UInt16 insPos;
    Boolean result;
    
    result = false;
    
    if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))
    {
        insPos = StrLen(kptr->currentCode);
        if (insPos < 4) // �������뷨���볤Ϊ4
        {
            kptr->currentCode[insPos] = ch;
            kptr->currentCode[insPos + 1] = '\0';
            
            PrvMatchWord(kptr);
            
            PrvNotifyListeners(kptr);
            
            result = true;
        }
    }
    
    return result;
}

Boolean PimeKernelCodeDelete(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    UInt16 insPos;
    Boolean result;
    
    result = false;
    
    insPos = StrLen(kptr->currentCode);
    if (insPos > 0)
    {
        kptr->currentCode[insPos - 1] = '\0';
        
        PrvNotifyListeners(kptr);
        
        result = true;
    }
    
    return result;
}

const Char *PimeKernelCodeGet(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    return kptr->currentCode;
}

// ��ѡ��
Boolean PimeKernelHintGetFirst(PimeKernelHandle kernel, PimeHintIteratorPtr it)
{
    return false;
}

Boolean PimeKernelHintGetNext(PimeKernelHandle kernel, PimeHintIteratorPtr it)
{
    return false;
}

Boolean PimeKernelHintGetPrevious(PimeKernelHandle kernel, PimeHintIteratorPtr it)
{
    return false;
}

// �������ִ�
void PimeKernelWordSet(PimeKernelHandle kernel, const Char *word)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    StrCopy(kptr->currentWord, word);
    
    PrvNotifyListeners(kptr);
}

const Char *PimeKernelWordGet(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    return kptr->currentWord;
}

// ˽�к���
static void PrvNotifyListeners(PrvKernelPtr kptr)
{
    PrvListenerPtr lptr;
    
    if (kptr->notifying)
    {
        ErrDisplay("Recursive notifications!");
        
        return;
    }
    
    kptr->notifying = true;
    
    lptr = kptr->listeners;
    
    while (lptr != NULL)
    {
        lptr->func(KERNEL_H(kptr), lptr->arg);
        lptr = lptr->next;
    }
    
    kptr->notifying = false;
}

#define CHAR_TO_HEX(ch) ((ch >= '0' && ch <= '9') ? (ch - '0') : (ch - 'a' + 0x0A))

static void PrvMatchWord(PrvKernelPtr kptr)
{
    if (StrLen(kptr->currentCode) == 4)
    {
        kptr->currentWord[0] = (CHAR_TO_HEX(kptr->currentCode[0]) << 8) & CHAR_TO_HEX(kptr->currentCode[1]);
        kptr->currentWord[1] = (CHAR_TO_HEX(kptr->currentCode[2]) << 8) & CHAR_TO_HEX(kptr->currentCode[3]);
        kptr->currentWord[2] = '\0';
    }
    else
        kptr->currentWord[0] = '\0';
}

static void PrvFreeListeners(PrvListenerPtr lptr)
{
    // ���������õݹ��ܼ򵥣��������ǵ�Palm�϶�ջ�����ƣ�������ѭ��ʵ�֡�
    PrvListenerPtr cur, next;
    
    cur = lptr;
    
    while (cur != NULL)
    {
        next = cur->next;
        MemPtrFree(cur);
        cur = next;
    }
}
