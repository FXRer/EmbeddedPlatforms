VERSION 5.00
Object = "{FE0065C0-1B7B-11CF-9D53-00AA003C9CB6}#1.1#0"; "COMCT232.OCX"
Begin VB.Form frm_test 
   Caption         =   "CARACA Test v1.13  http://caraca.sourceforge.net"
   ClientHeight    =   5955
   ClientLeft      =   420
   ClientTop       =   1155
   ClientWidth     =   6915
   LinkTopic       =   "Form1"
   ScaleHeight     =   5955
   ScaleWidth      =   6915
   Begin VB.CheckBox chk_star 
      Caption         =   "Filter STAR"
      Height          =   348
      Left            =   6132
      TabIndex        =   34
      ToolTipText     =   "Show only events from current STAR"
      Top             =   4620
      Width           =   768
   End
   Begin VB.CheckBox chk_node 
      Caption         =   "Filter NODE"
      Height          =   348
      Left            =   6132
      TabIndex        =   33
      ToolTipText     =   "Show only events from current NODE"
      Top             =   4200
      Width           =   768
   End
   Begin VB.CommandButton btn_table 
      Caption         =   "AssTable"
      Height          =   360
      Left            =   5628
      TabIndex        =   32
      Top             =   1176
      Width           =   852
   End
   Begin VB.CommandButton btn_clearlog 
      Caption         =   "Clear"
      Height          =   348
      Left            =   6132
      TabIndex        =   31
      Top             =   5544
      Width           =   768
   End
   Begin VB.CheckBox chk_log 
      Caption         =   "Enable log"
      Height          =   432
      Left            =   6132
      TabIndex        =   30
      ToolTipText     =   "Enable log for CARACA events"
      Top             =   5040
      Value           =   1  'Checked
      Width           =   852
   End
   Begin VB.TextBox txt_log 
      Height          =   1692
      Left            =   84
      Locked          =   -1  'True
      MaxLength       =   1024
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   29
      ToolTipText     =   "Message log"
      Top             =   4200
      Width           =   5976
   End
   Begin VB.CommandButton btn_sn 
      Caption         =   "SerNum"
      Height          =   348
      Left            =   5712
      TabIndex        =   28
      ToolTipText     =   "Request the Serial Number"
      Top             =   2856
      Width           =   768
   End
   Begin VB.CheckBox chk_update 
      Alignment       =   1  'Right Justify
      Caption         =   "Periodic update"
      Height          =   264
      Left            =   3948
      TabIndex        =   27
      ToolTipText     =   "Check to enable the periodic temperature udpate"
      Top             =   2940
      Width           =   1524
   End
   Begin VB.Timer tmr_read 
      Interval        =   1000
      Left            =   6636
      Top             =   1428
   End
   Begin VB.Frame frm_keys 
      Caption         =   "NODE Keys input"
      Height          =   1008
      Left            =   240
      TabIndex        =   22
      ToolTipText     =   "Click to update key status"
      Top             =   3000
      Width           =   3132
      Begin VB.Shape Shape1 
         BorderColor     =   &H80000003&
         BorderWidth     =   2
         Height          =   492
         Index           =   3
         Left            =   2280
         Shape           =   4  'Rounded Rectangle
         Top             =   360
         Width           =   732
      End
      Begin VB.Label lbl_key 
         Caption         =   "OFF"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Index           =   3
         Left            =   2400
         TabIndex        =   26
         ToolTipText     =   "Click to update Key status"
         Top             =   480
         Width           =   492
      End
      Begin VB.Shape Shape1 
         BorderColor     =   &H80000003&
         BorderWidth     =   2
         Height          =   492
         Index           =   2
         Left            =   1560
         Shape           =   4  'Rounded Rectangle
         Top             =   360
         Width           =   732
      End
      Begin VB.Label lbl_key 
         Caption         =   "OFF"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Index           =   2
         Left            =   1680
         TabIndex        =   25
         ToolTipText     =   "Click to update"
         Top             =   480
         Width           =   492
      End
      Begin VB.Shape Shape1 
         BorderColor     =   &H80000003&
         BorderWidth     =   2
         Height          =   492
         Index           =   1
         Left            =   840
         Shape           =   4  'Rounded Rectangle
         Top             =   360
         Width           =   732
      End
      Begin VB.Label lbl_key 
         Caption         =   "OFF"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Index           =   1
         Left            =   960
         TabIndex        =   24
         ToolTipText     =   "Click to update"
         Top             =   480
         Width           =   492
      End
      Begin VB.Shape Shape1 
         BorderColor     =   &H80000003&
         BorderWidth     =   2
         Height          =   492
         Index           =   0
         Left            =   120
         Shape           =   4  'Rounded Rectangle
         Top             =   360
         Width           =   732
      End
      Begin VB.Label lbl_key 
         Caption         =   "OFF"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   252
         Index           =   0
         Left            =   240
         TabIndex        =   23
         ToolTipText     =   "Click to update"
         Top             =   480
         Width           =   492
      End
   End
   Begin VB.PictureBox Picture1 
      Height          =   765
      Left            =   3480
      Picture         =   "caraca_test.frx":0000
      ScaleHeight     =   705
      ScaleWidth      =   3300
      TabIndex        =   21
      ToolTipText     =   "http://www.LancOS.com"
      Top             =   3276
      Width           =   3360
   End
   Begin VB.Frame frame_temp 
      Caption         =   "Temperature"
      Height          =   1068
      Left            =   3444
      TabIndex        =   19
      ToolTipText     =   "Click to update temperature"
      Top             =   1764
      Width           =   2052
      Begin VB.Label lbl_temp 
         Alignment       =   2  'Center
         Caption         =   "0°C"
         BeginProperty Font 
            Name            =   "Arial Black"
            Size            =   22.5
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   612
         Left            =   84
         TabIndex        =   20
         ToolTipText     =   "Click to update tempearature"
         Top             =   336
         Width           =   1812
      End
   End
   Begin VB.Timer tmr_termo 
      Enabled         =   0   'False
      Interval        =   10000
      Left            =   6636
      Top             =   1764
   End
   Begin VB.Frame frame_node 
      Caption         =   "NODE Relays output"
      Height          =   1092
      Left            =   252
      TabIndex        =   14
      ToolTipText     =   "Click to update Node Relay status"
      Top             =   1764
      Width           =   3132
      Begin VB.CommandButton btn_NodeRele 
         BackColor       =   &H000000FF&
         Caption         =   "OFF"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   3
         Left            =   2280
         MaskColor       =   &H0000FF00&
         Style           =   1  'Graphical
         TabIndex        =   18
         ToolTipText     =   "Switch NODE relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_NodeRele 
         BackColor       =   &H000000FF&
         Caption         =   "OFF"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   2
         Left            =   1560
         MaskColor       =   &H0000FF00&
         Style           =   1  'Graphical
         TabIndex        =   17
         ToolTipText     =   "Switch NODE relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_NodeRele 
         BackColor       =   &H000000FF&
         Caption         =   "OFF"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   1
         Left            =   840
         MaskColor       =   &H0000FF00&
         Style           =   1  'Graphical
         TabIndex        =   16
         ToolTipText     =   "Switch NODE relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_NodeRele 
         BackColor       =   &H000000FF&
         Caption         =   "OFF"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   0
         Left            =   120
         MaskColor       =   &H0000FF00&
         Style           =   1  'Graphical
         TabIndex        =   15
         ToolTipText     =   "Switch NODE relay ON/OFF"
         Top             =   240
         Width           =   732
      End
   End
   Begin ComCtl2.UpDown spin_addr 
      Height          =   372
      Left            =   6240
      TabIndex        =   10
      ToolTipText     =   "Change STAR address"
      Top             =   672
      Width           =   240
      _ExtentX        =   423
      _ExtentY        =   661
      _Version        =   327681
      Value           =   1
      BuddyControl    =   "txt_addr"
      BuddyDispid     =   196633
      OrigLeft        =   6600
      OrigTop         =   960
      OrigRight       =   6840
      OrigBottom      =   1572
      Max             =   63
      Min             =   1
      SyncBuddy       =   -1  'True
      BuddyProperty   =   65537
      Enabled         =   -1  'True
   End
   Begin VB.Frame frame_star 
      Caption         =   "STAR Relays output"
      Height          =   1092
      Left            =   252
      TabIndex        =   0
      ToolTipText     =   "Click to update Star Relays status"
      Top             =   168
      Width           =   5292
      Begin VB.CommandButton btn_StarRele 
         BackColor       =   &H0000FF00&
         Caption         =   "ON"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   6
         Left            =   4440
         Style           =   1  'Graphical
         TabIndex        =   7
         ToolTipText     =   "Switch STAR relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_StarRele 
         BackColor       =   &H0000FF00&
         Caption         =   "ON"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   5
         Left            =   3720
         Style           =   1  'Graphical
         TabIndex        =   6
         ToolTipText     =   "Switch STAR relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_StarRele 
         BackColor       =   &H0000FF00&
         Caption         =   "ON"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   4
         Left            =   3000
         Style           =   1  'Graphical
         TabIndex        =   5
         ToolTipText     =   "Switch STAR relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_StarRele 
         BackColor       =   &H0000FF00&
         Caption         =   "ON"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   3
         Left            =   2280
         Style           =   1  'Graphical
         TabIndex        =   4
         ToolTipText     =   "Switch STAR relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_StarRele 
         BackColor       =   &H0000FF00&
         Caption         =   "ON"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   2
         Left            =   1560
         Style           =   1  'Graphical
         TabIndex        =   3
         ToolTipText     =   "Switch STAR relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_StarRele 
         BackColor       =   &H0000FF00&
         Caption         =   "ON"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   1
         Left            =   840
         Style           =   1  'Graphical
         TabIndex        =   2
         ToolTipText     =   "Switch STAR relay ON/OFF"
         Top             =   240
         Width           =   732
      End
      Begin VB.CommandButton btn_StarRele 
         BackColor       =   &H0000FF00&
         Caption         =   "ON"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   732
         Index           =   0
         Left            =   120
         MaskColor       =   &H0000FF00&
         Style           =   1  'Graphical
         TabIndex        =   1
         ToolTipText     =   "Switch STAR relay ON/OFF"
         Top             =   240
         Width           =   732
      End
   End
   Begin ComCtl2.UpDown spin_nodeaddr 
      Height          =   372
      Left            =   6240
      TabIndex        =   11
      ToolTipText     =   "Change NODE address"
      Top             =   2292
      Width           =   240
      _ExtentX        =   423
      _ExtentY        =   661
      _Version        =   327681
      Value           =   63
      BuddyControl    =   "txt_nodeaddr"
      BuddyDispid     =   196632
      OrigLeft        =   6600
      OrigTop         =   960
      OrigRight       =   6840
      OrigBottom      =   1572
      Max             =   63
      SyncBuddy       =   -1  'True
      BuddyProperty   =   65537
      Enabled         =   -1  'True
   End
   Begin VB.Shape Shape3 
      Height          =   1530
      Left            =   90
      Top             =   90
      Width           =   6810
   End
   Begin VB.Shape Shape2 
      Height          =   2490
      Left            =   90
      Top             =   1680
      Width           =   6810
   End
   Begin VB.Label lbl_nodeaddr 
      Caption         =   "NODE address"
      Height          =   372
      Left            =   5760
      TabIndex        =   13
      Top             =   1812
      Width           =   732
   End
   Begin VB.Label txt_nodeaddr 
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "63"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   372
      Left            =   5760
      TabIndex        =   12
      Top             =   2292
      Width           =   492
   End
   Begin VB.Label txt_addr 
      BackColor       =   &H80000005&
      BorderStyle     =   1  'Fixed Single
      Caption         =   "1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   372
      Left            =   5760
      TabIndex        =   9
      Top             =   672
      Width           =   732
   End
   Begin VB.Label lbl_staraddr 
      Caption         =   "STAR address"
      Height          =   372
      Left            =   5760
      TabIndex        =   8
      Top             =   192
      Width           =   732
   End
   Begin VB.Menu m_setup 
      Caption         =   "Setup"
   End
   Begin VB.Menu m_config 
      Caption         =   "Configure"
   End
End
Attribute VB_Name = "frm_test"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Const GREEN_COLOR = &HFF00&
Const RED_COLOR = &HFF&

Const MAXSTARRELAY = 6
Const MAXNODERELAY = 3

Dim StarRelayStatus(MAXSTARRELAY + 1) As Boolean
Dim NodeRelayStatus(MAXNODERELAY + 1) As Boolean
Dim NodeOutPin(MAXNODERELAY + 1) As Byte
Dim StarOutPin(MAXSTARRELAY + 1) As Byte

Dim rMsgBuffer(8) As Byte

Private Sub btn_clearlog_Click()
txt_log.Text = ""
End Sub

Private Sub btn_NodeRele_Click(Index As Integer)
Dim n As Long
Dim x As Long
Dim val As Boolean

val = Not NodeRelayStatus(Index)

'3 = assign, 2 = toggle mask, 1 = set mask, 0 = clear mask
If val Then
    MsgBuffer(0) = 1
Else
    MsgBuffer(0) = 0
End If
MsgBuffer(1) = NodeOutPin(Index)

x = CAN_Address2ID(n, spin_nodeaddr.Value, CAN_SETOUTPUT)
If x <> 0 Then
    MsgBox "Errore nella chiamata a CAN_Address2ID"
Else
    x = CAN_TxMsg(Asc("C"), n, 2, 0, MsgBuffer(0))
    If x <> 0 Then
        MsgBox "Error: message transmission failed " + Str(x)
    Else
        NodeRelayStatus(Index) = val
        RefreshNodeRelays False, 0
    End If
End If
End Sub

Private Sub btn_NodeRele_KeyPress(Index As Integer, KeyAscii As Integer)
If KeyAscii = 8 Then
    UpdateNodeRelays
End If
End Sub

Private Sub btn_sn_Click()
Dim tx_id As Long
Dim x As Long

x = CAN_Address2ID(tx_id, spin_nodeaddr.Value, CAN_GETSERIALNUM)
If x <> 0 Then
    MsgBox "Errore nella chiamata a CAN_Address2ID"
Else
    MsgBuffer(0) = 0
    x = CAN_TxMsg(Asc("C"), tx_id, 0, 0, MsgBuffer(0))
    If x <> 0 Then
        MsgBox "Error: message transmission failed " + Str(x)
    End If
End If
End Sub

Private Sub btn_StarRele_Click(Index As Integer)
Dim n As Long
Dim x As Long
Dim val As Boolean

val = Not StarRelayStatus(Index)

'3 = assign, 2 = toggle mask, 1 = set mask, 0 = clear mask
If val Then
    MsgBuffer(0) = 1
Else
    MsgBuffer(0) = 0
End If
MsgBuffer(1) = StarOutPin(Index)

x = CAN_Address2ID(n, spin_addr.Value, CAN_SETOUTPUT)
If x <> 0 Then
    MsgBox "Errore nella chiamata a CAN_Address2ID"
Else
    x = CAN_TxMsg(Asc("C"), n, 2, 0, MsgBuffer(0))
    If x <> 0 Then
        MsgBox "Error: message transmission failed " + Str(x)
    Else
        StarRelayStatus(Index) = val
        RefreshStarRelays False, 0
    End If
End If
End Sub

Private Sub btn_StarRele_KeyPress(Index As Integer, KeyAscii As Integer)
If KeyAscii = 8 Then
    UpdateStarRelays
End If
End Sub

Private Sub btn_table_Click()
frm_AssTable.Show 0
End Sub

Private Sub Form_Load()
Dim n As Long
Dim x As Integer
Dim s As String
If CurDir = "C:\Programmi\Microsoft Visual Studio\VB98" Then
    ChDir "C:\ProgettiCVS\caraca\CARACA-VBTEST"
    'ChDir "c:\myprogetti\caraca\CARACA-VBTEST"
End If

ComPort = val(ReadFormattedFile("caraca.ini", "COMPort", "3"))  'Use COM3 by default

Open "CaracaLog.txt" For Append Access Write As #1

HelloIndex = 0

n = CAN_Init(ComPort)
If n <> 0 Then
    MsgBox "Error: COM port open failed, try to select another one"
    frm_setup.Show 1
End If

NodeOutPin(0) = 1
NodeOutPin(1) = 2
NodeOutPin(2) = 8
NodeOutPin(3) = 32

x = 1
For n = 0 To MAXSTARRELAY
    StarOutPin(n) = x
    x = x * 2
Next

spin_addr.Value = val(ReadFormattedFile("caraca.ini", "CurrentSTAR", "1"))
spin_nodeaddr.Value = val(ReadFormattedFile("caraca.ini", "CurrentNODE", "2"))

'DataEnvironment1.Connection1.Open

'Init relays status array
For n = 0 To MAXSTARRELAY
    StarRelayStatus(n) = False
Next
UpdateStarRelays

'Init relays status array
For n = 0 To MAXNODERELAY
    NodeRelayStatus(n) = False
Next
UpdateNodeRelays

End Sub

Private Sub Form_Unload(Cancel As Integer)

'Write parameters in the INI file
WriteConfig "caraca.ini", "CurrentNODE", Trim(Str(spin_nodeaddr.Value))
WriteConfig "caraca.ini", "CurrentSTAR", Trim(Str(spin_addr.Value))

Close #1

Unload frm_setup
Unload frm_config
Unload frm_setaddr
Unload frm_AssTable
CAN_End
End Sub

Private Sub RefreshStarRelays(ByVal update As Boolean, ByVal val As Byte)
Dim j As Integer

For j = 0 To MAXSTARRELAY
    If update Then
        If (val Mod 2) > 0 Then
            StarRelayStatus(j) = True
        Else
            StarRelayStatus(j) = False
        End If
        val = val \ 2
    End If
    
    If StarRelayStatus(j) Then
        btn_StarRele(j).BackColor = RED_COLOR
        btn_StarRele(j).Caption = "OFF"
    Else
        btn_StarRele(j).BackColor = GREEN_COLOR
        btn_StarRele(j).Caption = "ON"
    End If
Next
End Sub

Private Sub UpdateStarRelays()
Dim n As Long
Dim x As Long

'MsgBuffer(0) = 3    '3 = assign, 2 = toggle mask, 1 = set mask, 0 = clear mask
'MsgBuffer(1) = n

x = CAN_Address2ID(n, spin_addr.Value, CAN_GETOUTPUT)
If x <> 0 Then
    MsgBox "Errore nella chiamata a CAN_Address2ID"
Else
    x = CAN_TxMsg(Asc("C"), n, 0, 0, MsgBuffer(0))
    If x <> 0 Then
        MsgBox "Error: message transmission failed " + Str(x)
    End If
End If

Sleep 50
End Sub

Private Sub RefreshNodeRelays(ByVal update As Boolean, ByVal val As Byte)
Dim j As Integer

For j = 0 To MAXNODERELAY
    If update Then
        If (val Mod 2) > 0 Then
            NodeRelayStatus(j) = True
        Else
            NodeRelayStatus(j) = False
        End If
        val = val \ 2
        If j = 1 Or j = 2 Then
            val = val \ 2
        End If
    End If

    If NodeRelayStatus(j) Then
        btn_NodeRele(j).BackColor = GREEN_COLOR
        btn_NodeRele(j).Caption = "ON"
    Else
        btn_NodeRele(j).BackColor = RED_COLOR
        btn_NodeRele(j).Caption = "OFF"
    End If
Next

End Sub

Private Sub UpdateNodeRelays()
Dim n As Long
Dim x As Long

'MsgBuffer(0) = 3    '3 = assign, 2 = toggle mask, 1 = set mask, 0 = clear mask
'MsgBuffer(1) = n

x = CAN_Address2ID(n, spin_nodeaddr.Value, CAN_GETOUTPUT)
If x <> 0 Then
    MsgBox "Errore nella chiamata a CAN_Address2ID"
Else
    x = CAN_TxMsg(Asc("C"), n, 0, 0, MsgBuffer(0))
    If x <> 0 Then
        MsgBox "Error: message transmission failed " + Str(x)
    End If
End If

Sleep 50
End Sub

Private Sub frame_node_Click()
UpdateNodeRelays
End Sub

Private Sub frame_star_Click()
UpdateStarRelays
End Sub

Private Sub frame_temp_Click()
lbl_temp_Click
End Sub

Private Sub frm_keys_Click()
lbl_key_Click 0
End Sub

Private Sub lbl_key_Click(Index As Integer)
Dim tx_id As Long
Dim x As Long

x = CAN_Address2ID(tx_id, spin_nodeaddr.Value, CAN_GETINPUT)
If x <> 0 Then
    MsgBox "Errore nella chiamata a CAN_Address2ID"
Else
    MsgBuffer(0) = 0
    x = CAN_TxMsg(Asc("C"), tx_id, 1, 0, MsgBuffer(0))
    If x <> 0 Then
        MsgBox "Error: message transmission failed " + Str(x)
    End If
End If
End Sub

Private Sub lbl_temp_Click()
Dim tx_id As Long
Dim x As Long

x = CAN_Address2ID(tx_id, spin_nodeaddr.Value, CAN_READTEMP)
If x <> 0 Then
    MsgBox "Errore nella chiamata a CAN_Address2ID"
Else
    MsgBuffer(0) = 7
    x = CAN_TxMsg(Asc("C"), tx_id, 1, 0, MsgBuffer(0))
    If x <> 0 Then
        MsgBox "Error: message transmission failed " + Str(x)
    End If
End If
End Sub

Private Sub chk_update_Click()
tmr_termo.Enabled = chk_update.Value
End Sub

Private Sub m_config_Click()
frm_config.Show 0
End Sub

Private Sub m_setup_Click()
frm_setup.Show 1
End Sub

Private Sub tmr_read_Timer()
Dim x As Long
Dim fx As Double
Dim xstr As String
Dim n As Integer
Dim rx_type As Byte
Dim rx_id As Long
Dim rx_length As Integer
Dim rx_flags As Long
Dim rx_addr As Integer
Dim rx_func As Integer
Dim found As Boolean
Dim log_ok As Boolean
Dim log_str As String
Dim temp_val As Integer

Dim datestr As String
datestr = FormatDateTime(Date, vbShortDate) + " "
datestr = datestr + FormatDateTime(Time, vbLongTime)

x = CAN_Timeout(1)
Do While x = 0
    rMsgBuffer(0) = 0
    x = CAN_RxMsg(rx_type, rx_id, rx_length, rx_flags, rMsgBuffer(0))
    If x = 0 Then
      If rx_type = Asc("C") Then
        x = CAN_ID2Address(rx_id, rx_addr, rx_func)
    
        'Test if we need to log the message
        log_ok = False
        If chk_log.Value = 1 Then
            If chk_node.Value = 0 And chk_star.Value = 0 Then
                log_ok = True
            Else
                If chk_node.Value = 1 And rx_addr = spin_nodeaddr.Value Then
                    log_ok = True
                End If
                If chk_star.Value = 1 And rx_addr = spin_addr.Value Then
                    log_ok = True
                End If
            End If
        End If

        log_str = datestr + " "
        
        'Message log
        If log_ok Then
            Select Case rx_func
            Case CAN_SETOUTPUT
                log_str = log_str + "Set Output"
            Case CAN_INPUTCHANGE
                log_str = log_str + "Input Change"
            Case CAN_SENDOUTPUT
                log_str = log_str + "Output State"
            Case CAN_SENDINPUT
                log_str = log_str + "Input State"
            Case CAN_SENDTEMP
                log_str = log_str + "Temperature"
            Case CAN_SENDSERIALNUM
                log_str = log_str + "Hello msg"
            Case CAN_SEND_EEP
                log_str = log_str + "EEprom"
            Case CAN_RC5KEY
                log_str = log_str + "RC5 Key"
            Case Else   ' Altri valori.
               log_str = log_str + "Uknown"
            End Select
            
            log_str = log_str + " *** Func=" + Str(rx_func) + " Addr=" + Str(rx_addr)
            For n = 0 To rx_length - 1
                log_str = log_str + " Data= " + Str(rMsgBuffer(n))
            Next
            Write #1, log_str
            log_str = log_str + Chr(13) + Chr(10)
            'MsgBox "MaxLen: " + Str(txt_log.MaxLength) + " Len: " + Str(Len(txt_log.Text))
            txt_log.Text = log_str + txt_log.Text
        End If

        If rx_func = CAN_SENDSERIALNUM Then
            x = rMsgBuffer(0)
            x = x * 256
            x = x + rMsgBuffer(1)
            
            If HelloIndex < HELLOENTRY_MAX Then
                HelloBuffer_SerNum(HelloIndex) = x
                HelloBuffer_Address(HelloIndex) = rx_addr
                Hellobuffer_Type(HelloIndex) = rMsgBuffer(2)

                HelloIndex = HelloIndex + 1
            End If
        End If

        'Update NODE controls
        If rx_addr = spin_nodeaddr.Value Then
            Select Case rx_func
            Case CAN_INPUTCHANGE
                n = rMsgBuffer(1) - 1
                If rMsgBuffer(0) = 1 Then
                    lbl_key(n).Caption = "ON"  'key press
                Else
                    lbl_key(n).Caption = "OFF"   'key release
                End If
            Case CAN_SENDOUTPUT
                RefreshNodeRelays True, rMsgBuffer(0)
            Case CAN_SENDINPUT
                If rMsgBuffer(0) = 0 Then   'Input status if 0, input mask if 1
                    For n = 0 To 3
                        x = rMsgBuffer(1) Mod 2
                        If x = 0 Then
                            lbl_key(n).Caption = "ON"
                        Else
                            lbl_key(n).Caption = "OFF"
                        End If
                        rMsgBuffer(1) = rMsgBuffer(1) \ 2
                    Next
                    'MsgBox "Input status: " + Str(rMsgBuffer(1))
                End If
            Case CAN_SENDTEMP
                If rMsgBuffer(0) = 7 Then
                    If rMsgBuffer(1) > 127 Then
                        temp_val = rMsgBuffer(1) - 256
                    Else
                        temp_val = rMsgBuffer(1)
                    End If
                    If rMsgBuffer(2) = 0 Then
                        lbl_temp.Caption = Str(temp_val) + ".0" + "°C"
                    Else
                        lbl_temp.Caption = Str(temp_val + 0.5) + "°C"
                    End If
                End If
                fx = rMsgBuffer(1)
                If rMsgBuffer(2) <> 0 Then
                    fx = fx + 0.5
                End If
            Case CAN_SEND_EEP
            Case CAN_RC5KEY
            End Select
        
        'Update star controls
        ElseIf rx_addr = spin_addr.Value Then
            Select Case rx_func
            Case CAN_SENDOUTPUT
                RefreshStarRelays True, rMsgBuffer(0)
            Case CAN_SEND_EEP
                If AssCounter < ASSENTRY_SIZE _
                    And AssBase >= ASSTABLE_START And AssBase < ASSTABLE_START + ASSENTRY_MAX * ASSENTRY_SIZE _
                    And rMsgBuffer(0) >= AssBase And rMsgBuffer(0) < AssBase + ASSENTRY_SIZE Then
                    
                    AssBuffer(rMsgBuffer(0) - AssBase) = rMsgBuffer(1)
                    AssCounter = AssCounter + 1
                    If AssCounter = ASSENTRY_SIZE Then
                        AssOk = True
                    End If
                End If
            End Select
        End If
      ElseIf rx_type = Asc("E") Then
        If chk_log.Value = 1 Then
            log_str = datestr + " *** Error " + Str(rMsgBuffer(3)) + " RxCnt " + Str(rMsgBuffer(0)) + " TxCnt " + Str(rMsgBuffer(1)) + " Bits " + Str(rMsgBuffer(2))
            Write #1, log_str
            log_str = log_str + Chr(13) + Chr(10)
            txt_log.Text = log_str + txt_log.Text
        End If
      End If
    End If
Loop
End Sub

Private Sub tmr_termo_Timer()
lbl_temp_Click
End Sub

