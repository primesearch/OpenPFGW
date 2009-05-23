; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CPFGW_ServerV2Dlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "PFGW_ServerV2.h"

ClassCount=3
Class1=CPFGW_ServerV2App
Class2=CPFGW_ServerV2Dlg
Class3=CAboutDlg

ResourceCount=7
Resource1=IDR_TRAY_MENU
Resource2=IDR_MAINFRAME
Resource3=IDR_CLIENTGRID_POPUP_MENU
Resource4=IDD_PFGW_SERVERV2_DIALOG
Resource5=IDR_FILESGRID_POPUP_MENU
Resource6=IDR_MAIN_MENU
Resource7=IDD_ABOUTBOX

[CLS:CPFGW_ServerV2App]
Type=0
HeaderFile=PFGW_ServerV2.h
ImplementationFile=PFGW_ServerV2.cpp
Filter=N

[CLS:CPFGW_ServerV2Dlg]
Type=0
HeaderFile=PFGW_ServerV2Dlg.h
ImplementationFile=PFGW_ServerV2Dlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CPFGW_ServerV2Dlg

[CLS:CAboutDlg]
Type=0
HeaderFile=PFGW_ServerV2Dlg.h
ImplementationFile=PFGW_ServerV2Dlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_PFGW_SERVERV2_DIALOG]
Type=1
Class=CPFGW_ServerV2Dlg
ControlCount=10
Control1=IDC_ADD_FILE_EDIT,edit,1350631552
Control2=IDC_ADD_FILE_LINES_EDIT,edit,1350631552
Control3=IDC_ADD_FILE,button,1342242817
Control4=IDC_TEXT_MSG,combobox,1344339970
Control5=IDC_STATIC,button,1342177287
Control6=IDC_STATIC,button,1342177287
Control7=IDC_NUM_CLIENTS,static,1350701313
Control8=IDC_STATIC,static,1342308352
Control9=IDC_CLIENTS_GRID,MFCGridCtrl,1342373888
Control10=IDC_INP_FILES_GRID,MFCGridCtrl,1342242816

[MNU:IDR_TRAY_MENU]
Type=1
Class=?
Command1=ID_FILE_TEST
CommandCount=1

[MNU:IDR_MAIN_MENU]
Type=1
Class=?
Command1=IDEXIT
Command2=IDC_USE_TRAY_ICON
Command3=IDC_CLEAR_ICON_ERROR
CommandCount=3

[MNU:IDR_CLIENTGRID_POPUP_MENU]
Type=1
Class=?
Command1=IDC_REMOVE_CLIENT_FROM_GRID
Command2=IDC_RENAME_CLIENT_ON_NEXT_CONTACT
Command3=IDC_SEND_CLIENT_NOTE
Command4=IDC_MERGE_SELECT_CLIENT
Command5=IDC_MERGE_SELECTED_CLIENT_HERE
Command6=IDC_CLEAR_CLIENT_STATS
Command7=IDC_VIEW_CLIENT_NEW_PRIMES
Command8=IDC_ARCHIVE_CLIENT_PRIMES
Command9=IDC_VIEW_ALL_CLIENT_PRIMES
CommandCount=9

[MNU:IDR_FILESGRID_POPUP_MENU]
Type=1
Class=?
Command1=IDC_MOVE_FILE_DOWN
Command2=IDC_MOVE_FILE_UP
CommandCount=2

