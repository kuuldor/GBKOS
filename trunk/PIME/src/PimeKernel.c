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
    
    // 防止递归调用的标志：防止在Notify时Listener回调函数中向Kernel发送命令，引起新的状态变化，导致递归调用。
    // 从设计上讲，在Listener回调函数中只允许调用获得状态的Kernel接口函数。
    Boolean notifying;
    
} PrvKernelType, *PrvKernelPtr;

#define KERNEL_H(x) ((PimeKernelHandle)x)
#define KERNEL_P(x) ((PrvKernelPtr)x)

static void PrvNotifyListeners(PrvKernelPtr kptr);
static void PrvMatchWord(PrvKernelPtr kptr);

static void PrvFreeListeners(PrvListenerPtr lptr);

// 内核控制
PimeKernelHandle PimeKernelStart()
{
    PrvKernelPtr kptr;
    
    if (PimeKernelGet() != NULL) return KERNEL_H(NULL); // 如果Kernel已经启动过，则返回NULL。
    
    kptr = (PrvKernelPtr)MemPtrNew(sizeof(PrvKernelType));
    
    if (kptr)
    {
        kptr->currentCode = (Char *)MemPtrNew(5); // 内码输入法最多4个字符，外加结尾0。
        kptr->currentWord = (Char *)MemPtrNew(3); // 内码输入法最多2个字符（一个汉字）， 外加结尾0。
        
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

// 回调函数
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

// 初始化
void PimeKernelInit(PimeKernelHandle kernel)
{
    PrvKernelPtr kptr = KERNEL_P(kernel);
    
    StrCopy(kptr->currentImeName, "内码输入法");
    StrCopy(kptr->currentImeShortName, "内码");
    StrCopy(kptr->currentCode, "");
    StrCopy(kptr->currentWord, "");
    
    PrvNotifyListeners(kptr);
}

// 输入法
void PimeKernelImeSwitchToNext(PimeKernelHandle kernel)
{
    // 只有内码输入法，没有其他输入法供切换。
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

// 编码
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
        if (insPos < 4) // 内码输入法的码长为4
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

// 候选字
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

// 可输入字词
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

// 私有函数
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
    // 本来这里用递归会很简单，不过考虑到Palm上堆栈的限制，所以用循环实现。
    PrvListenerPtr cur, next;
    
    cur = lptr;
    
    while (cur != NULL)
    {
        next = cur->next;
        MemPtrFree(cur);
        cur = next;
    }
}
