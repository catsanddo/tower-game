#ifndef CE_CORO_H
#define CE_CORO_H

#define CE_coroBEGIN static int state; switch (state) { case 0:
#define CE_coroRETURN(val) do { state = __LINE__; return (val); case __LINE__: } while(0)
#define CE_coroFINISH }

#endif
