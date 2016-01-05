/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件提供了对对讲发送缓冲区进行操作的相关的接口函数
**  date           :  2013.12.20
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _TALKBACK_LIST_H
#define _TALKBACK_LIST_H

#include "ttypes.h"
//
// 对讲音频发送
//
void WaitTalkbackSendList();
void SignalTalkbackSendList();
int PopTalkbackSendList( TALKBACK_NODE *tFrame );
int PutTalkbackSendList( TALKBACK_NODE *tFrame );
void ClearTalkbackSendList();

//
// 对讲音频获取
//
void WaitTalkbackRecvList();
void SignalTalkbackRecvList();
int PopTalkbackRecvList( TALKBACK_NODE *tFrame );
int PutTalkbackRecvList( TALKBACK_NODE *tFrame );
void ClearTalkbackRecvList();

//
// 调用此函数会使线程进入阻塞状态，直到有客户端连接成功。
//
void WaitTalkbackClient();
void BoardcastTalkbackClient();

#endif  // _TALKBACK_LIST_H

