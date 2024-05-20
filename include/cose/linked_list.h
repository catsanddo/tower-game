#ifndef CE_LINKED_LIST_H
#define CE_LINKED_LIST_H

#define CE_ForEach(T,c,f) CE_ForEach_N(T,c,f,next)
#define CE_ForEach_N(T,c,f,next) for (T *(c) = (f); (c) != 0; (c)=(c)->next)

#define CE_SLLPush_N(f,n,next) ((f)==0? \
        ((f)=(n), (n)->next = 0): \
        ((n)->next = (f), (f)=(n)))
#define CE_SLLPush(f,n) CE_SLLPush_N(f,n,next)
#define CE_SLLPop_N(f,n,next) ((f)==0? \
        ((n)=0): \
        ((n)=(f),(f)=(f)->next))
#define CE_SLLPop(f,n) CE_SLLPop_N(f,n,next)

#define CE_DLLPushStart_NP(f,l,n,next,prev) ((f)==0? \
        ((f)=(l)=(n), (n)->next = (n)->prev = 0): \
        ((n)->next = (f), (f)->prev = (n), (n)->prev = 0, (f)=(n)))
#define CE_DLLPushStart(f,l,n) CE_DLLPushStart_NP(f,l,n,next,prev)
#define CE_DLLPushEnd_NP(f,l,n,next,prev) CE_DLLPushStart_NP(l,f,n,prev,next)
#define CE_DLLPushEnd(f,l,n) CE_DLLPushEnd_NP(f,l,n,next,prev)

#define CE_DLLPopStart_NP(f,l,n,next,prev) ((f)==0? \
        ((n)=0): \
        (f)==(l)? \
        ((n)=(f), (f)=(l)=0): \
        ((n)=(f), (f)=(f)->next, (f)->prev = 0))
#define CE_DLLPopStart(f,l,n) CE_DLLPopStart_NP(f,l,n,next,prev)
#define CE_DLLPopEnd_NP(f,l,n,next,prev) CE_DLLPopStart_NP(l,f,n,prev,next)
#define CE_DLLPopEnd(f,l,n) CE_DLLPopEnd_NP(f,l,n,next,prev)

#endif
