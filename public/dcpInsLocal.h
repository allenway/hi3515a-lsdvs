#ifndef __DcpInsLocal_H__
#define __DcpInsLocal_H__

#define DIL_BASE	0xA000	// dcp ins local base,
    
#define DIL_FORWARD	             ( DIL_BASE + 0x01) // 转发指令
#define DIL_ALARM_UPLOAD	     ( DIL_BASE + 0x02) // 报警消息上传
#define DIL_HDD_STAT_ERR_REPORT	 ( DIL_BASE + 0x03) // 硬盘状态异常消息上报
#define DIL_PARAM_CHANGED_REPORT ( DIL_BASE + 0x04) // 参数发生改变上报

#endif // __DcpInsLocal_H__

