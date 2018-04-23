VERSION 5.00
Begin VB.Form frm_setup 
   Caption         =   "Setup"
   ClientHeight    =   2076
   ClientLeft      =   48
   ClientTop       =   336
   ClientWidth     =   2736
   LinkTopic       =   "Form1"
   ScaleHeight     =   2076
   ScaleWidth      =   2736
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton btn_set 
      Caption         =   "OK"
      Height          =   372
      Left            =   120
      TabIndex        =   6
      Top             =   1560
      Width           =   852
   End
   Begin VB.CommandButton btn_close 
      Caption         =   "Cancel"
      Height          =   372
      Left            =   1680
      TabIndex        =   5
      Top             =   1560
      Width           =   852
   End
   Begin VB.Frame frm_com 
      Caption         =   "COM Port"
      Height          =   1332
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   2412
      Begin VB.OptionButton Option4 
         Caption         =   "COM4"
         Height          =   252
         Left            =   1440
         TabIndex        =   4
         Top             =   840
         Width           =   732
      End
      Begin VB.OptionButton Option3 
         Caption         =   "COM3"
         Height          =   252
         Left            =   1440
         TabIndex        =   3
         Top             =   360
         Width           =   732
      End
      Begin VB.OptionButton Option2 
         Caption         =   "COM2"
         Height          =   252
         Left            =   240
         TabIndex        =   2
         Top             =   840
         Width           =   732
      End
      Begin VB.OptionButton Option1 
         Caption         =   "COM1"
         Height          =   252
         Left            =   240
         TabIndex        =   1
         Top             =   360
         Width           =   732
      End
   End
End
Attribute VB_Name = "frm_setup"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private com_no As Integer

Private Sub btn_close_Click()
Hide
End Sub

Private Sub btn_set_Click()
Dim n As Long
CAN_End
ComPort = com_no
n = CAN_Init(ComPort)
If n <> 0 Then
    MsgBox "Error: COM port open failed (" + Str(n) + ")"
Else
    WriteConfig "caraca.ini", "COMPort", Trim(Str(ComPort))
    Hide
End If
End Sub

Private Sub Form_Activate()
com_no = ComPort
If ComPort = 1 Then
    frm_setup.Option1 = True
ElseIf ComPort = 2 Then
    frm_setup.Option2 = True
ElseIf ComPort = 3 Then
    frm_setup.Option3 = True
Else
    frm_setup.Option4 = True
End If
End Sub

Private Sub Option1_Click()
com_no = 1
End Sub

Private Sub Option2_Click()
com_no = 2
End Sub

Private Sub Option3_Click()
com_no = 3
End Sub

Private Sub Option4_Click()
com_no = 4
End Sub
