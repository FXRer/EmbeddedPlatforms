VERSION 5.00
Begin VB.Form frm_setaddr 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Set address"
   ClientHeight    =   3060
   ClientLeft      =   2760
   ClientTop       =   3750
   ClientWidth     =   6930
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3060
   ScaleWidth      =   6930
   ShowInTaskbar   =   0   'False
   Begin VB.TextBox txt_newaddr 
      Height          =   348
      Left            =   3948
      TabIndex        =   21
      Top             =   504
      Width           =   432
   End
   Begin VB.CommandButton btn_prev 
      Caption         =   "Previous"
      Height          =   348
      Left            =   168
      TabIndex        =   16
      Top             =   2184
      Width           =   768
   End
   Begin VB.CommandButton btn_next 
      Caption         =   "Next"
      Height          =   348
      Left            =   1008
      TabIndex        =   15
      Top             =   2184
      Width           =   768
   End
   Begin VB.CommandButton btn_close 
      Caption         =   "Close"
      Height          =   768
      Left            =   5628
      TabIndex        =   14
      Top             =   2184
      Width           =   1188
   End
   Begin VB.CommandButton btn_remove 
      Caption         =   "Remove"
      Height          =   768
      Left            =   2856
      TabIndex        =   13
      Top             =   2184
      Width           =   1188
   End
   Begin VB.CommandButton btn_first 
      Caption         =   "First"
      Height          =   348
      Left            =   168
      TabIndex        =   12
      Top             =   2604
      Width           =   768
   End
   Begin VB.CommandButton btn_last 
      Caption         =   "Last"
      Height          =   348
      Left            =   1008
      TabIndex        =   11
      Top             =   2604
      Width           =   768
   End
   Begin VB.CommandButton btn_set 
      Caption         =   "Apply"
      Height          =   768
      Left            =   5628
      TabIndex        =   10
      ToolTipText     =   "Download Ass.Table to STAR"
      Top             =   504
      Width           =   1188
   End
   Begin VB.TextBox txtNALIAS 
      DataField       =   "NALIAS"
      DataMember      =   "Command2"
      DataSource      =   "DataEnvironment1"
      Height          =   285
      Left            =   1824
      TabIndex        =   9
      Top             =   1704
      Width           =   3375
   End
   Begin VB.TextBox txtNTYPE 
      DataField       =   "NTYPE"
      DataMember      =   "Command2"
      DataSource      =   "DataEnvironment1"
      Height          =   285
      Left            =   1824
      Locked          =   -1  'True
      TabIndex        =   7
      Top             =   1320
      Width           =   330
   End
   Begin VB.TextBox txtMULTICAST 
      DataField       =   "MULTICAST"
      DataMember      =   "Command2"
      DataSource      =   "DataEnvironment1"
      Height          =   285
      Left            =   1824
      TabIndex        =   5
      Top             =   936
      Width           =   330
   End
   Begin VB.TextBox txtADDRESS 
      DataField       =   "ADDRESS"
      DataMember      =   "Command2"
      DataSource      =   "DataEnvironment1"
      Height          =   285
      Left            =   1824
      Locked          =   -1  'True
      TabIndex        =   3
      Top             =   564
      Width           =   330
   End
   Begin VB.TextBox txtSERIALNUMBER 
      DataField       =   "SERIALNUMBER"
      DataMember      =   "Command2"
      DataSource      =   "DataEnvironment1"
      Height          =   285
      Left            =   1824
      Locked          =   -1  'True
      TabIndex        =   1
      Top             =   180
      Width           =   660
   End
   Begin VB.Label lbl_naddr 
      AutoSize        =   -1  'True
      Caption         =   "New Address"
      Height          =   192
      Left            =   2856
      TabIndex        =   20
      Top             =   588
      Width           =   972
   End
   Begin VB.Label lbl_type 
      Height          =   180
      Left            =   2352
      TabIndex        =   19
      Top             =   1344
      Width           =   2868
   End
   Begin VB.Label lbl_idx 
      Caption         =   "00"
      Height          =   180
      Left            =   2016
      TabIndex        =   18
      Top             =   2352
      Width           =   264
   End
   Begin VB.Line Line3 
      X1              =   2436
      X2              =   2184
      Y1              =   2352
      Y2              =   2688
   End
   Begin VB.Label lbl_count 
      Caption         =   "99"
      Height          =   180
      Left            =   2436
      TabIndex        =   17
      Top             =   2520
      Width           =   264
   End
   Begin VB.Label lblFieldLabel 
      Alignment       =   1  'Right Justify
      AutoSize        =   -1  'True
      Caption         =   "Description"
      Height          =   192
      Index           =   4
      Left            =   972
      TabIndex        =   8
      Top             =   1740
      Width           =   816
   End
   Begin VB.Label lblFieldLabel 
      Alignment       =   1  'Right Justify
      AutoSize        =   -1  'True
      Caption         =   "Node Type"
      Height          =   192
      Index           =   3
      Left            =   960
      TabIndex        =   6
      Top             =   1368
      Width           =   828
   End
   Begin VB.Label lblFieldLabel 
      Alignment       =   1  'Right Justify
      AutoSize        =   -1  'True
      Caption         =   "Secondary Address"
      Height          =   192
      Index           =   2
      Left            =   348
      TabIndex        =   4
      Top             =   984
      Width           =   1440
   End
   Begin VB.Label lblFieldLabel 
      Alignment       =   1  'Right Justify
      AutoSize        =   -1  'True
      Caption         =   "Current Address"
      Height          =   192
      Index           =   1
      Left            =   636
      TabIndex        =   2
      Top             =   600
      Width           =   1152
   End
   Begin VB.Label lblFieldLabel 
      Alignment       =   1  'Right Justify
      AutoSize        =   -1  'True
      Caption         =   "Serial Number"
      Height          =   192
      Index           =   0
      Left            =   756
      TabIndex        =   0
      Top             =   228
      Width           =   1032
   End
End
Attribute VB_Name = "frm_setaddr"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub btn_close_Click()
Unload Me
End Sub

Private Sub btn_first_Click()
DataEnvironment1.rsCommand2.MoveFirst
lbl_count.Caption = Str(DataEnvironment1.rsCommand2.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand2.AbsolutePosition)
End Sub

Private Sub btn_last_Click()
DataEnvironment1.rsCommand2.MoveLast
lbl_count.Caption = Str(DataEnvironment1.rsCommand2.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand2.AbsolutePosition)
End Sub

Private Sub btn_next_Click()
If Not DataEnvironment1.rsCommand2.EOF Then
    DataEnvironment1.rsCommand2.MoveNext
    If DataEnvironment1.rsCommand2.EOF Then
        DataEnvironment1.rsCommand2.MovePrevious
    End If
End If
lbl_count.Caption = Str(DataEnvironment1.rsCommand2.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand2.AbsolutePosition)
End Sub

Private Sub btn_prev_Click()
If Not DataEnvironment1.rsCommand2.BOF Then
    DataEnvironment1.rsCommand2.MovePrevious
    If DataEnvironment1.rsCommand2.BOF Then
        DataEnvironment1.rsCommand2.MoveNext
    End If
End If
lbl_count.Caption = Str(DataEnvironment1.rsCommand2.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand2.AbsolutePosition)
End Sub

Private Sub SetAddress(ByVal cur_addr As Integer, ByVal ser_num As Integer, ByVal new_addr As Integer, ByVal multi_addr As Integer)
Dim tx_id As Long
Dim x As Long

x = CAN_Address2ID(tx_id, cur_addr, CAN_SETADDRESS)
If x <> 0 Then
    MsgBox "Errore nella chiamata a CAN_Address2ID"
Else
    MsgBuffer(0) = ser_num \ 256
    MsgBuffer(1) = ser_num Mod 256
    MsgBuffer(2) = new_addr
    MsgBuffer(3) = multi_addr
    x = CAN_TxMsg(Asc("C"), tx_id, 4, 0, MsgBuffer(0))
    If x <> 0 Then
        MsgBox "Error: message transmission failed"
    End If
End If
End Sub

Private Sub btn_remove_Click()
If Not DataEnvironment1.rsCommand2.BOF And Not DataEnvironment1.rsCommand2.EOF Then
    DataEnvironment1.rsCommand2.Delete
    btn_prev_Click
    btn_next_Click
End If
End Sub

Private Sub btn_set_Click()
Dim ca As Integer
ca = val(txt_newaddr.Text)
If ca <= 0 Or ca > 63 Then
    MsgBox "Incorrect Address value (Allowed range: 1-63)"
Else
    SetAddress DataEnvironment1.rsCommand2!ADDRESS, DataEnvironment1.rsCommand2!SERIALNUMBER, ca, DataEnvironment1.rsCommand2!MULTICAST

    DataEnvironment1.rsCommand2!ADDRESS = ca
    DataEnvironment1.rsCommand2.update
End If
txt_newaddr.Text = ""
End Sub

Private Sub Form_Load()
'DataEnvironment1.rsCommand2.Open
lbl_count.Caption = Str(DataEnvironment1.rsCommand2.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand2.AbsolutePosition)
End Sub

Private Sub Form_Unload(Cancel As Integer)
'DataEnvironment1.rsCommand2.Close
End Sub

