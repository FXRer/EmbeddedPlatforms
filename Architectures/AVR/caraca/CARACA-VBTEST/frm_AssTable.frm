VERSION 5.00
Begin VB.Form frm_AssTable 
   Caption         =   "Association Table"
   ClientHeight    =   3945
   ClientLeft      =   780
   ClientTop       =   1050
   ClientWidth     =   7245
   LinkTopic       =   "Form1"
   ScaleHeight     =   3945
   ScaleWidth      =   7245
   Begin VB.CommandButton btn_upload 
      Caption         =   "Upload"
      Height          =   348
      Left            =   4452
      TabIndex        =   41
      ToolTipText     =   "Upload Ass.Table from STAR"
      Top             =   3528
      Width           =   852
   End
   Begin VB.CommandButton btn_download 
      Caption         =   "Download"
      Height          =   348
      Left            =   4452
      TabIndex        =   40
      ToolTipText     =   "Download Ass.Table to STAR"
      Top             =   3108
      Width           =   852
   End
   Begin VB.CommandButton btn_last 
      Caption         =   "Last"
      Height          =   348
      Left            =   1092
      TabIndex        =   39
      Top             =   3528
      Width           =   768
   End
   Begin VB.CommandButton btn_first 
      Caption         =   "First"
      Height          =   348
      Left            =   252
      TabIndex        =   38
      Top             =   3528
      Width           =   768
   End
   Begin VB.CommandButton btn_remove 
      Caption         =   "Remove"
      Height          =   348
      Left            =   3024
      TabIndex        =   35
      Top             =   3528
      Width           =   852
   End
   Begin VB.CommandButton btn_add 
      Caption         =   "Add"
      Height          =   348
      Left            =   3024
      TabIndex        =   34
      Top             =   3108
      Width           =   852
   End
   Begin VB.CommandButton btn_close 
      Caption         =   "Close"
      Height          =   768
      Left            =   5880
      TabIndex        =   33
      Top             =   3108
      Width           =   1104
   End
   Begin VB.CommandButton btn_next 
      Caption         =   "Next"
      Height          =   348
      Left            =   1092
      TabIndex        =   32
      Top             =   3108
      Width           =   768
   End
   Begin VB.CommandButton btn_prev 
      Caption         =   "Previous"
      Height          =   348
      Left            =   252
      TabIndex        =   31
      Top             =   3108
      Width           =   768
   End
   Begin VB.TextBox txt_adata3 
      DataField       =   "ADATA3"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   6132
      TabIndex        =   30
      Top             =   2436
      Width           =   600
   End
   Begin VB.TextBox txt_adata2 
      DataField       =   "ADATA2"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   5460
      TabIndex        =   29
      Top             =   2436
      Width           =   600
   End
   Begin VB.TextBox txt_adata1 
      DataField       =   "ADATA1"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   4788
      TabIndex        =   28
      Top             =   2436
      Width           =   600
   End
   Begin VB.TextBox txt_len3 
      DataField       =   "A_LEN"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   3360
      TabIndex        =   26
      Top             =   2436
      Width           =   600
   End
   Begin VB.TextBox txt_rtr3 
      DataField       =   "A_RTR"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   2688
      TabIndex        =   25
      Top             =   2436
      Width           =   600
   End
   Begin VB.TextBox txt_addr3 
      DataField       =   "A_ADDR"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   2016
      TabIndex        =   24
      Top             =   2436
      Width           =   600
   End
   Begin VB.TextBox txt_func3 
      DataField       =   "A_FUNC"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   1344
      TabIndex        =   23
      Top             =   2436
      Width           =   600
   End
   Begin VB.TextBox txt_delay 
      DataField       =   "DELAY"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   1344
      TabIndex        =   21
      Top             =   1848
      Width           =   600
   End
   Begin VB.TextBox txt_edata3 
      DataField       =   "EDATA3"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   6132
      TabIndex        =   18
      Top             =   1260
      Width           =   600
   End
   Begin VB.TextBox txt_edata2 
      DataField       =   "EDATA2"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   5460
      TabIndex        =   17
      Top             =   1260
      Width           =   600
   End
   Begin VB.TextBox txt_edata1 
      DataField       =   "EDATA1"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   4788
      TabIndex        =   16
      Top             =   1260
      Width           =   600
   End
   Begin VB.TextBox txt_datac 
      DataField       =   "EMASK_COUNT"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   4788
      TabIndex        =   15
      Top             =   672
      Width           =   600
   End
   Begin VB.TextBox txt_len2 
      DataField       =   "EMATCH_LEN"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   3360
      TabIndex        =   13
      Top             =   1260
      Width           =   600
   End
   Begin VB.TextBox txt_rtr2 
      DataField       =   "EMATCH_RTR"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   2688
      TabIndex        =   12
      Top             =   1260
      Width           =   600
   End
   Begin VB.TextBox txt_addr2 
      DataField       =   "EMATCH_ADDR"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   2016
      TabIndex        =   11
      Top             =   1260
      Width           =   600
   End
   Begin VB.TextBox txt_func2 
      DataField       =   "EMATCH_FUNC"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   1344
      TabIndex        =   10
      Top             =   1260
      Width           =   600
   End
   Begin VB.TextBox txt_len1 
      DataField       =   "EMASK_LEN"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   3360
      TabIndex        =   4
      Top             =   672
      Width           =   600
   End
   Begin VB.TextBox txt_rtr1 
      DataField       =   "EMASK_RTR"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   2688
      TabIndex        =   3
      Top             =   672
      Width           =   600
   End
   Begin VB.TextBox txt_addr1 
      DataField       =   "EMASK_ADDR"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   2016
      TabIndex        =   2
      Top             =   672
      Width           =   600
   End
   Begin VB.TextBox txt_func1 
      DataField       =   "EMASK_FUNC"
      DataMember      =   "Command1"
      DataSource      =   "DataEnvironment1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   432
      Left            =   1344
      TabIndex        =   1
      Top             =   672
      Width           =   600
   End
   Begin VB.Label lbl_count 
      Caption         =   "99"
      Height          =   180
      Left            =   2520
      TabIndex        =   37
      Top             =   3444
      Width           =   264
   End
   Begin VB.Line Line3 
      X1              =   2520
      X2              =   2268
      Y1              =   3276
      Y2              =   3612
   End
   Begin VB.Label lbl_idx 
      Caption         =   "00"
      Height          =   180
      Left            =   2100
      TabIndex        =   36
      Top             =   3276
      Width           =   264
   End
   Begin VB.Line Line2 
      X1              =   252
      X2              =   6972
      Y1              =   2352
      Y2              =   2352
   End
   Begin VB.Line Line1 
      X1              =   252
      X2              =   6972
      Y1              =   1764
      Y2              =   1764
   End
   Begin VB.Shape Shape1 
      BorderStyle     =   6  'Inside Solid
      BorderWidth     =   2
      Height          =   2868
      Left            =   252
      Top             =   168
      Width           =   6732
   End
   Begin VB.Label Label10 
      Caption         =   "Data Bytes"
      Height          =   432
      Left            =   4284
      TabIndex        =   27
      Top             =   2436
      Width           =   432
   End
   Begin VB.Label Label9 
      Caption         =   "Action"
      Height          =   180
      Left            =   336
      TabIndex        =   22
      Top             =   2520
      Width           =   852
   End
   Begin VB.Label Label8 
      Caption         =   "Delay"
      Height          =   264
      Left            =   336
      TabIndex        =   20
      Top             =   1932
      Width           =   600
   End
   Begin VB.Label Label7 
      Caption         =   "Data Count"
      Height          =   348
      Left            =   4284
      TabIndex        =   19
      Top             =   672
      Width           =   432
   End
   Begin VB.Label Label6 
      Caption         =   "Data Bytes"
      Height          =   432
      Left            =   4284
      TabIndex        =   14
      Top             =   1260
      Width           =   432
   End
   Begin VB.Label Label5 
      Caption         =   "Event Match"
      Height          =   180
      Left            =   336
      TabIndex        =   9
      Top             =   1344
      Width           =   936
   End
   Begin VB.Label Label4 
      Caption         =   "Length"
      Height          =   264
      Left            =   3360
      TabIndex        =   8
      Top             =   252
      Width           =   516
   End
   Begin VB.Label Label3 
      Caption         =   " RTR"
      Height          =   264
      Left            =   2772
      TabIndex        =   7
      Top             =   252
      Width           =   432
   End
   Begin VB.Label Label2 
      Caption         =   "Addr."
      Height          =   180
      Left            =   2100
      TabIndex        =   6
      Top             =   252
      Width           =   432
   End
   Begin VB.Label Label1 
      Caption         =   "Func."
      Height          =   180
      Left            =   1344
      TabIndex        =   5
      Top             =   252
      Width           =   432
   End
   Begin VB.Label lbl_mask 
      Caption         =   "Event Mask"
      Height          =   180
      Left            =   336
      TabIndex        =   0
      Top             =   756
      Width           =   936
   End
End
Attribute VB_Name = "frm_AssTable"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub btn_add_Click()
DataEnvironment1.rsCommand1.AddNew
DataEnvironment1.rsCommand1!STAR_ADDRESS = frm_test.spin_addr.Value
DataEnvironment1.rsCommand1!EMASK_FUNC = 31
DataEnvironment1.rsCommand1!EMASK_ADDR = 63
DataEnvironment1.rsCommand1!EMASK_RTR = 1
DataEnvironment1.rsCommand1!EMASK_LEN = 7
DataEnvironment1.rsCommand1!EMASK_COUNT = 0
DataEnvironment1.rsCommand1.update
lbl_count.Caption = Str(DataEnvironment1.rsCommand1.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand1.AbsolutePosition)
DataEnvironment1.rsCommand1.Close
DataEnvironment1.rsCommand1.Open
End Sub

Private Sub btn_close_Click()
Unload Me
End Sub

Private Sub btn_download_Click()
Dim x As Long
Dim k As Integer
Dim ass_idx As Integer
Dim tx_id As Long
Dim tx_length As Integer
Dim tx_flags As Long
Dim tx_addr As Integer
Dim tx_func As Integer

'Check for table empty
If DataEnvironment1.rsCommand1.BOF And DataEnvironment1.rsCommand1.EOF Then
    Exit Sub
End If

'Look for last record and record count
ass_idx = 0
DataEnvironment1.rsCommand1.MoveFirst
Do While Not DataEnvironment1.rsCommand1.EOF And ass_idx < ASSENTRY_MAX
    DataEnvironment1.rsCommand1.MoveNext
    ass_idx = ass_idx + 1
Loop

'Write Table terminator first
For k = 0 To 1
    x = CAN_Address2ID(tx_id, frm_test.spin_addr.Value, CAN_WRITE_EEP)
    If x <> 0 Then
        MsgBox "Errore nella chiamata a CAN_Address2ID"
    Else
        MsgBuffer(0) = ASSTABLE_START + ass_idx * ASSENTRY_SIZE + k  'eeprom address
        MsgBuffer(1) = 0
        x = CAN_TxMsg(Asc("C"), tx_id, 2, 0, MsgBuffer(0))
        If x <> 0 Then
            MsgBox "Errore nell'invio del messaggio"
        End If
    End If
    Sleep 50
Next

ass_idx = 0
DataEnvironment1.rsCommand1.MoveFirst
Do While Not DataEnvironment1.rsCommand1.EOF And ass_idx < ASSENTRY_MAX
    'Read entry from database
    tx_func = DataEnvironment1.rsCommand1!EMASK_FUNC
    tx_addr = DataEnvironment1.rsCommand1!EMASK_ADDR
    x = CAN_Address2ID(tx_id, tx_addr, tx_func)
    x = tx_id * 2 + DataEnvironment1.rsCommand1!EMASK_RTR
    x = x * 16 + DataEnvironment1.rsCommand1!EMASK_LEN
    AssBuffer(0) = x Mod 256    'lsB
    AssBuffer(1) = x \ 256      'msB
    
    tx_func = DataEnvironment1.rsCommand1!EMATCH_FUNC
    tx_addr = DataEnvironment1.rsCommand1!EMATCH_ADDR
    x = CAN_Address2ID(tx_id, tx_addr, tx_func)
    x = tx_id * 2 + DataEnvironment1.rsCommand1!EMATCH_RTR
    x = x * 16 + DataEnvironment1.rsCommand1!EMATCH_LEN
    AssBuffer(2) = x Mod 256    'lsB
    AssBuffer(3) = x \ 256      'msB

    AssBuffer(4) = DataEnvironment1.rsCommand1!EMASK_COUNT

    AssBuffer(5) = DataEnvironment1.rsCommand1!EDATA1
    AssBuffer(6) = DataEnvironment1.rsCommand1!EDATA2
    AssBuffer(7) = DataEnvironment1.rsCommand1!EDATA3

    x = DataEnvironment1.rsCommand1!DELAY
    AssBuffer(8) = x Mod 256    'lsB
    AssBuffer(9) = x \ 256      'msB

    tx_func = DataEnvironment1.rsCommand1!A_FUNC
    tx_addr = DataEnvironment1.rsCommand1!A_ADDR
    x = CAN_Address2ID(tx_id, tx_addr, tx_func)
    x = tx_id * 2 + DataEnvironment1.rsCommand1!A_RTR
    x = x * 16 + DataEnvironment1.rsCommand1!A_LEN
    AssBuffer(10) = x Mod 256    'lsB
    AssBuffer(11) = x \ 256      'msB

    AssBuffer(12) = DataEnvironment1.rsCommand1!ADATA1
    AssBuffer(13) = DataEnvironment1.rsCommand1!ADATA2
    AssBuffer(14) = DataEnvironment1.rsCommand1!ADATA3

    'Write an entry of AssTable in the eeprom of current STAR
    For k = 0 To 15
        x = CAN_Address2ID(tx_id, frm_test.spin_addr.Value, CAN_WRITE_EEP)
        If x <> 0 Then
            MsgBox "Errore nella chiamata a CAN_Address2ID"
        Else
            MsgBuffer(0) = ASSTABLE_START + ass_idx * ASSENTRY_SIZE + k  'eeprom address
            MsgBuffer(1) = AssBuffer(k)
            x = CAN_TxMsg(Asc("C"), tx_id, 2, 0, MsgBuffer(0))
            If x <> 0 Then
                MsgBox "Errore nell'invio del messaggio"
            End If
        End If
        Sleep 20
    Next
    
    Sleep 100
    ass_idx = ass_idx + 1
    
    DataEnvironment1.rsCommand1.MoveNext
Loop

btn_first_Click
End Sub

Private Sub btn_first_Click()
DataEnvironment1.rsCommand1.MoveFirst
lbl_count.Caption = Str(DataEnvironment1.rsCommand1.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand1.AbsolutePosition)
End Sub

Private Sub btn_last_Click()
DataEnvironment1.rsCommand1.MoveLast
lbl_count.Caption = Str(DataEnvironment1.rsCommand1.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand1.AbsolutePosition)
End Sub

Private Sub btn_next_Click()
If Not DataEnvironment1.rsCommand1.EOF Then
    DataEnvironment1.rsCommand1.MoveNext
    If DataEnvironment1.rsCommand1.EOF Then
        DataEnvironment1.rsCommand1.MovePrevious
    End If
End If
lbl_count.Caption = Str(DataEnvironment1.rsCommand1.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand1.AbsolutePosition)
End Sub

Private Sub btn_prev_Click()
If Not DataEnvironment1.rsCommand1.BOF Then
    DataEnvironment1.rsCommand1.MovePrevious
    If DataEnvironment1.rsCommand1.BOF Then
        DataEnvironment1.rsCommand1.MoveNext
    End If
End If
lbl_count.Caption = Str(DataEnvironment1.rsCommand1.RecordCount)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand1.AbsolutePosition)
End Sub

Private Sub btn_remove_Click()
If Not DataEnvironment1.rsCommand1.BOF And Not DataEnvironment1.rsCommand1.EOF Then
    DataEnvironment1.rsCommand1.Delete
    btn_prev_Click
    btn_next_Click
End If
End Sub

Private Sub btn_upload_Click()
Dim x As Long
Dim k As Integer
Dim ass_idx As Integer
Dim tx_id As Long
Dim tx_length As Integer
Dim tx_flags As Long
Dim tx_addr As Integer
Dim tx_func As Integer

'First remove current records
If Not (DataEnvironment1.rsCommand1.BOF And DataEnvironment1.rsCommand1.EOF) Then
    DataEnvironment1.rsCommand1.MoveFirst
    Do While Not DataEnvironment1.rsCommand1.EOF
        DataEnvironment1.rsCommand1.Delete
        DataEnvironment1.rsCommand1.MoveNext
    Loop
End If

ass_idx = 0

Do
    'Clear flag
    AssOk = False
    AssCounter = 0
    AssBase = ASSTABLE_START + ass_idx * ASSENTRY_SIZE
    
    'Read an entry of AssTable in the eeprom of current STAR
    For k = 0 To 15
        x = CAN_Address2ID(tx_id, frm_test.spin_addr.Value, CAN_READ_EEP)
        If x <> 0 Then
            MsgBox "Errore nella chiamata a CAN_Address2ID"
        Else
            MsgBuffer(0) = AssBase + k  'eeprom address
            x = CAN_TxMsg(Asc("C"), tx_id, 1, 0, MsgBuffer(0))
            If x <> 0 Then
                MsgBox "Errore nell'invio del messaggio"
            End If
        End If
        Sleep 10
    Next
    ass_idx = ass_idx + 1

    'Wait for reply
    k = 0
    Do While (k < 200 And Not AssOk)
        k = k + 1
        DoEvents
        Sleep 10
    Loop

    If Not AssOk Then
        MsgBox "Timeout: no answer from STAR"
        AssBuffer(0) = 0
        AssBuffer(1) = 0
    Else
        'First two bytes = 0 mean END of TABLE
        If AssBuffer(0) <> 0 Or AssBuffer(1) <> 0 Then
            'Fill a new record
            DataEnvironment1.rsCommand1.AddNew
            DataEnvironment1.rsCommand1!STAR_ADDRESS = frm_test.spin_addr.Value
            For k = 0 To 15
                DataEnvironment1.rsCommand1.Fields(k + 2).Value = AssBuffer(k)
            Next
            tx_id = (AssBuffer(0) + AssBuffer(1) * 256) \ 32
            x = CAN_ID2Address(tx_id, tx_addr, tx_func)
            DataEnvironment1.rsCommand1!EMASK_FUNC = tx_func
            DataEnvironment1.rsCommand1!EMASK_ADDR = tx_addr
            tx_id = (AssBuffer(0) \ 16) Mod 2
            DataEnvironment1.rsCommand1!EMASK_RTR = tx_id
            tx_id = AssBuffer(0) Mod 16
            DataEnvironment1.rsCommand1!EMASK_LEN = tx_id

            DataEnvironment1.rsCommand1!EMASK_COUNT = AssBuffer(4)

            tx_id = (AssBuffer(2) + AssBuffer(3) * 256) \ 32
            x = CAN_ID2Address(tx_id, tx_addr, tx_func)
            DataEnvironment1.rsCommand1!EMATCH_FUNC = tx_func
            DataEnvironment1.rsCommand1!EMATCH_ADDR = tx_addr
            tx_id = (AssBuffer(2) \ 16) Mod 2
            DataEnvironment1.rsCommand1!EMATCH_RTR = tx_id
            tx_id = AssBuffer(2) Mod 16
            DataEnvironment1.rsCommand1!EMATCH_LEN = tx_id

            DataEnvironment1.rsCommand1!EDATA1 = AssBuffer(5)
            DataEnvironment1.rsCommand1!EDATA2 = AssBuffer(6)
            DataEnvironment1.rsCommand1!EDATA3 = AssBuffer(7)

            tx_id = (AssBuffer(8) + AssBuffer(9) * 256)
            DataEnvironment1.rsCommand1!DELAY = tx_id

            tx_id = (AssBuffer(10) + AssBuffer(11) * 256) \ 32
            x = CAN_ID2Address(tx_id, tx_addr, tx_func)
            DataEnvironment1.rsCommand1!A_FUNC = tx_func
            DataEnvironment1.rsCommand1!A_ADDR = tx_addr
            tx_id = (AssBuffer(10) \ 16) Mod 2
            DataEnvironment1.rsCommand1!A_RTR = tx_id
            tx_id = AssBuffer(10) Mod 16
            DataEnvironment1.rsCommand1!A_LEN = tx_id

            DataEnvironment1.rsCommand1!ADATA1 = AssBuffer(12)
            DataEnvironment1.rsCommand1!ADATA2 = AssBuffer(13)
            DataEnvironment1.rsCommand1!ADATA3 = AssBuffer(14)

            DataEnvironment1.rsCommand1.update
        End If
    End If
Loop Until (AssBuffer(0) = 0 And AssBuffer(1) = 0) Or ass_idx = 15

btn_first_Click
End Sub

Private Sub Form_Load()
DataEnvironment1.rsCommand1.Filter = "STAR_ADDRESS = " + Str(frm_test.spin_addr.Value)
lbl_idx.Caption = Str(DataEnvironment1.rsCommand1.AbsolutePosition)
lbl_count.Caption = Str(DataEnvironment1.rsCommand1.RecordCount)
End Sub

