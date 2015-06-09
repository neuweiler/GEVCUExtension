/*
 * Sys_Messages.h
 *
*/

#ifndef SYSMSG_H_
#define SYSMSG_H_


enum SystemMessage {
    MSG_STARTUP = 0x3000,
    MSG_SOFT_FAULT = 0x3100,
    MSG_HARD_FAULT = 0x3150,
    MSG_DISABLE = 0x3200,
    MSG_ENABLE = 0x3300,
    MSG_SET_PARAM = 0x4000,
    MSG_CONFIG_CHANGE = 0x4001,
    MSG_COMMAND = 0x4002
};

#endif
