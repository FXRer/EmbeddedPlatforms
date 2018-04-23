VERSION 5.00
Begin VB.Form frm_config 
   Caption         =   "Configure"
   ClientHeight    =   3615
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   6240
   LinkTopic       =   "Form1"
   ScaleHeight     =   3615
   ScaleWidth      =   6240
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton btn_updatedb 
      Caption         =   "Update Db"
      Enabled         =   0   'False
      Height          =   495
      Left            =   4800
      TabIndex        =   6
      Top             =   2160
      Width           =   1335
   End
   Begin VB.CommandButton btn_update 
      Caption         =   "Request Update"
      Height          =   495
      Left            =   3360
      TabIndex        =   5
      Top             =   2160
      Width           =   1380
   End
   Begin VB.TextBox txt_info 
      Height          =   1440
      Left            =   3360
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      TabIndex        =   3
      Top             =   504
      Width           =   2700
   End
   Begin VB.CommandButton btn_close 
      Caption         =   "Close"
      Height          =   432
      Left            =   3360
      TabIndex        =   2
      Top             =   3024
      Width           =   2700
   End
   Begin VB.ListBox list_conf 
      Height          =   2790
      ItemData        =   "frm_config.frx":0000
      Left            =   84
      List            =   "frm_config.frx":0002
      TabIndex        =   0
      Top             =   504
      Width           =   3120
   End
   Begin VB.Label lbl_info 
      Alignment       =   2  'Center
      Caption         =   "Device info"
      Height          =   264
      Left            =   3360
      TabIndex        =   4
      Top             =   168
      Width           =   2700
   End
   Begin VB.Label lbl_conf 
      Alignment       =   2  'Center
      Caption         =   "Devices on the bus"
      Height          =   264
      Left            =   84
      TabIndex        =   1
      Top             =   168
      Width           =   3120
   End
End
Attribute VB_Name = "frm_config"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub btn_close_Click()
Hide
End Sub

'Private Sub btn_remove_Click()
'list_conf.RemoveItem list_conf.ListIndex
'txt_info.Text = ""
'End Sub

Private Sub btn_update_Click()
Dim x As Long
Dim k As Integer
Dim tx_id As Long

btn_update.Enabled = False
btn_close.Enabled = False
btn_updatedb.Enabled = True

txt_info.Text = ""

DataEnvironment1.rsCommand2.Open

'Loop to clear all flags
If Not (DataEnvironment1.rsCommand2.BOF And DataEnvironment1.rsCommand2.EOF) Then
    DataEnvironment1.rsCommand2.MoveFirst
    Do While Not DataEnvironment1.rsCommand2.EOF
        DataEnvironment1.rsCommand2!FLAG = False
        DataEnvironment1.rsCommand2.update
        
        DataEnvironment1.rsCommand2.MoveNext
    Loop
End If

HelloIndex = 0
For k = 1 To 63
    x = CAN_Address2ID(tx_id, k, CAN_GETSERIALNUM)
    If x <> 0 Then
        MsgBox "Errore nella chiamata a CAN_Address2ID"
    Else
        MsgBuffer(0) = 0
        x = CAN_TxMsg(Asc("C"), tx_id, 0, 0, MsgBuffer(0))
        If x <> 0 Then
            MsgBox "Error: message transmission failed"
        End If
    End If
    
    Sleep 50
Next

'For k = 1 To 500
'    Sleep 50
'    DoEvents
'Next


End Sub

Private Sub UpdateDbNodesFromBuf()
Dim k As Integer
Dim found As Boolean

If HelloIndex > 0 Then
  For k = 0 To HelloIndex - 1
    'Search for SerNum
    found = False
    'DataEnvironment1.rsCommand2.Resync

    If Not (DataEnvironment1.rsCommand2.BOF And DataEnvironment1.rsCommand2.EOF) Then
        'xstr = "SERIALNUMBER=" + Trim(Str(HelloBuffer_SerNum(k)))
        'DataEnvironment1.rsCommand2.Find xstr

        DataEnvironment1.rsCommand2.MoveFirst
        Do While Not DataEnvironment1.rsCommand2.EOF
            If DataEnvironment1.rsCommand2!SERIALNUMBER = HelloBuffer_SerNum(k) Then
                Exit Do
            End If
            DataEnvironment1.rsCommand2.MoveNext
        Loop

        If Not DataEnvironment1.rsCommand2.EOF Then
            found = True
        End If
    End If
    
    If Not found Then
        DataEnvironment1.rsCommand2.AddNew
        DataEnvironment1.rsCommand2!SERIALNUMBER = HelloBuffer_SerNum(k)
        DataEnvironment1.rsCommand2!MULTICAST = 0
        DataEnvironment1.rsCommand2!NALIAS = "No Alias"
    End If
    DataEnvironment1.rsCommand2!ADDRESS = HelloBuffer_Address(k)
    DataEnvironment1.rsCommand2!NTYPE = Hellobuffer_Type(k)
    If HelloBuffer_Address(k) = 63 Then
        DataEnvironment1.rsCommand2!NALIAS = "Not Configured"
    End If
    DataEnvironment1.rsCommand2!FLAG = True

    DataEnvironment1.rsCommand2.update
  Next
End If
HelloIndex = 0

End Sub

Private Sub btn_updatedb_Click()
UpdateDbNodesFromBuf

'loop to remove node that didn't answer
'If Not (DataEnvironment1.rsCommand2.BOF And DataEnvironment1.rsCommand2.EOF) Then
'    DataEnvironment1.rsCommand2.MoveFirst
'    Do While Not DataEnvironment1.rsCommand2.EOF
'        If Not DataEnvironment1.rsCommand2!FLAG Then
'            DataEnvironment1.rsCommand2.Delete
'        End If
'        DataEnvironment1.rsCommand2.MoveNext
'    Loop
'End If

UpdateList
DataEnvironment1.rsCommand2.Close

'btn_first_Click
btn_update.Enabled = True
btn_close.Enabled = True
btn_updatedb.Enabled = False

End Sub

Private Sub Form_Load()
DataEnvironment1.rsCommand2.Open
UpdateDbNodesFromBuf
UpdateList
DataEnvironment1.rsCommand2.Close
End Sub

Private Sub UpdateList()
Dim x As Long
Dim k As Integer
Dim xstr As String

list_conf.Clear

If Not (DataEnvironment1.rsCommand2.BOF And DataEnvironment1.rsCommand2.EOF) Then
    DataEnvironment1.rsCommand2.MoveFirst
    Do While Not DataEnvironment1.rsCommand2.EOF
        If DataEnvironment1.rsCommand2!NTYPE = 0 Then
            xstr = "NODE:"
        Else
            xstr = "STAR:"
        End If
        xstr = xstr + Str(DataEnvironment1.rsCommand2!SERIALNUMBER)
        xstr = xstr + "@" + Str(DataEnvironment1.rsCommand2!ADDRESS)
        xstr = xstr + " " + Mid(DataEnvironment1.rsCommand2!NALIAS, 1, 80)
    
        list_conf.AddItem xstr
        DataEnvironment1.rsCommand2.MoveNext
    Loop
    DataEnvironment1.rsCommand2.MoveFirst
End If

End Sub

Private Sub list_conf_Click()
Dim Pos As Integer
Dim sernum As Integer
Dim naddr As Integer
Dim idx As Integer
Dim xstr As String
idx = list_conf.ListIndex
Pos = InStr(1, list_conf.Text, ":", vbBinaryCompare)
If Pos <> 0 Then
    xstr = Mid(list_conf.Text, Pos + 1)
    sernum = val(xstr)
    Pos = InStr(Pos, list_conf.Text, "@", vbBinaryCompare)
    If Pos <> 0 Then
        xstr = Mid(list_conf.Text, Pos + 1)
        naddr = val(xstr)
        
        If sernum <> 0 And naddr <> 0 Then
            'xstr = "Type: " + Mid(list_conf.Text, 1, 4) + Chr(13) + Chr(10)
            'xstr = xstr + "Serial Number: " + Str(sernum) + Chr(13) + Chr(10)
            'xstr = xstr + "Device Address: " + Str(naddr) + Chr(13) + Chr(10)
            'txt_info.Text = xstr
        
            DataEnvironment1.rsCommand2.Open
            DataEnvironment1.rsCommand2.Find "SERIALNUMBER=" + Trim(Str(sernum))
            If DataEnvironment1.rsCommand2.EOF Then
                MsgBox "Record not found"
                txt_info.Text = ""
            Else
                xstr = "Type: "
                If DataEnvironment1.rsCommand2!NTYPE = 1 Then
                    xstr = xstr + "STAR"
                Else
                    xstr = xstr + "NODE"
                End If
                xstr = xstr + Chr(13) + Chr(10)
                xstr = xstr + "Serial Number: " + Str(DataEnvironment1.rsCommand2!SERIALNUMBER) + Chr(13) + Chr(10)
                xstr = xstr + "Primary Address: " + Str(DataEnvironment1.rsCommand2!ADDRESS) + Chr(13) + Chr(10)
                xstr = xstr + "Secondary Address: " + Str(DataEnvironment1.rsCommand2!MULTICAST) + Chr(13) + Chr(10)
                xstr = xstr + "Alias: " + DataEnvironment1.rsCommand2!NALIAS
                txt_info.Text = xstr
            End If
            DataEnvironment1.rsCommand2.Close
        End If
    End If
End If
End Sub

Private Sub list_conf_DblClick()
Dim Pos As Integer
Dim sernum As Integer
Dim naddr As Integer
Dim idx As Integer
Dim xstr As String
idx = list_conf.ListIndex
Pos = InStr(1, list_conf.Text, ":", vbBinaryCompare)
If Pos <> 0 Then
    xstr = Mid(list_conf.Text, Pos + 1)
    sernum = val(xstr)
    Pos = InStr(Pos, list_conf.Text, "@", vbBinaryCompare)
    If Pos <> 0 Then
        xstr = Mid(list_conf.Text, Pos + 1)
        naddr = val(xstr)
        
        If sernum <> 0 And naddr <> 0 Then
            DataEnvironment1.rsCommand2.Open
            DataEnvironment1.rsCommand2.Find "SERIALNUMBER=" + Trim(Str(sernum))
            If DataEnvironment1.rsCommand2.EOF Then
                MsgBox "Record not found"
            Else
                frm_setaddr.Show 1
            End If
            UpdateList
            DataEnvironment1.rsCommand2.Close
            txt_info.Text = ""
        End If
    End If
End If
End Sub

