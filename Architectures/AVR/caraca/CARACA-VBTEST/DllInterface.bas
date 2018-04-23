Attribute VB_Name = "DllInterface"
Option Explicit

'----------------
'Windows APIs
Declare Sub Sleep Lib "kernel32" (ByVal n As Long)

'----------------
'CARACA Lib
Declare Function CAN_Init Lib "caracalib" (ByVal port As Long) As Long
Declare Sub CAN_End Lib "caracalib" ()
Declare Function CAN_Timeout Lib "caracalib" (ByVal n As Long) As Long
Declare Function CAN_RxMsg Lib "caracalib" (ByRef rx_type As Byte, ByRef rx_id As Long, ByRef rx_length As Integer, ByRef rx_flags As Long, ByRef rx_data As Byte) As Long
Declare Function CAN_TxMsg Lib "caracalib" (ByVal tx_type As Byte, ByVal tx_id As Long, ByVal tx_length As Integer, ByVal tx_flags As Long, ByRef tx_data As Byte) As Long
'long WINAPI CAN_Address2ID(long *idp, short node, short function)
Declare Function CAN_Address2ID Lib "caracalib" (ByRef id As Long, ByVal node_address As Integer, ByVal function_code As Integer) As Long
'long WINAPI CAN_ID2Address(long id, short *nodep, short *functionp)
Declare Function CAN_ID2Address Lib "caracalib" (ByVal id As Long, ByRef node_address As Integer, ByRef function_code As Integer) As Long
