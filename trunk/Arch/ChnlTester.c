#if CHNL_TESTER

#include "ChnlTester.h"

static Chnl	__g_Chnl;
static Chnl* g_pChnl = &__g_Chnl;

PktDesc g_PktDesc;

void ChnlTester_init(MsgPostFun postMsg, Transfer * pTransfer)
{
	Chnl_Init(g_pChnl, 1, const PktDesc * pPktDesc, postMsg, pTransfer);
}

#endif	//f CHNL_TESTER

