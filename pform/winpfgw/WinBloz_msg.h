// winbloz_msg.h

// this file contains message numbers which are used by the PFWin32GUIOutput class and the WinPFGWDlg class.
// these message being sent "simulate" printf type stuff.

#if !defined (__WinBloz_MSG_H__)
#define __WinBloz_MSG_H__

#if !defined (_MSC_VER)
#define WM_USER 1000
#define SendMessage(a,b,c,d)
#endif

#define WinPFGW_MSG WM_USER+1000

#define M_STDERR 1
#define M_PRINTF 2
#define M_THREAD_EXITING 3



#endif  // #if !defined (__WinBloz_MSG_H__)
