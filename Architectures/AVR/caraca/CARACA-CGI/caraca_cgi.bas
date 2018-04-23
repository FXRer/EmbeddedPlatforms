Attribute VB_Name = "Caraca"
'========================
' CARACA_CGI.BAS
'========================
'
' CGI support for the CARACA Project
'
' Version: 0.5 Beta Release (05-22-2000)
'
' Features:
'           Control a CARACA Star with 1 Node from Web
'           also can record the status of relays just
'           in case a blackout come to visit us, hehe ;-)
'
' Author:  Massimo Graziani (massimo.graziani@tin.it)
'
' This version include the CGI4VB.BAS
' Author:  Kevin O'Brien <obrienk@pobox.com>
'                        <obrienk@ix.netcom.com>
'
' This is a modified version of CARACA_TEST v 1.01
' Copyright (C) 1999-2000  Claudio Lanconelli
'
'  e-mail: lanconel@cs.unibo.it
'  http://caraca.sourceforge.net
'
'-------------------------------------------------------------------------
'
' This program is free software; you can redistribute it and/or
' modify it under the terms of the GNU  General Public License
' as published by the Free Software Foundation; either version2 of
' the License, or (at your option) any later version.
'
' This program is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
' General Public License for more details.
'
' You should have received a copy of the GNU  General Public License
' along with this program (see COPYING);     if not, write to the
' Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
'
'-------------------------------------------------------------------------
'=========================================================================
'
' How to use
' ----------
'
' 1) Copy the CARACA.EXE and CaracaLib.dll on /cgi-bin/ real directory.
'
' 2) If needed copy the runtime files of Visual Basic.
'
' 2) Connect RS232 -> CANDONGLE -> STAR -> NODE.
'
' 3) Run the Internet browser.
'
' 4) From the browser type this URL:
'
'       http://yourserver/cgi-bin/caraca.exe
'
'       (this example display the status)
'
' You can send some parameters like:
'
'       http://yourserver/cgi-bin/caraca.exe?command
'
' Commands list:
'
' Nothing = Display the status
' Reset   = Reset the Star and Node
' Sw1     = Turn ON/OFF StarSwitch 1
' Sw2     = Turn ON/OFF StarSwitch 2
' Sw3     = Turn ON/OFF StarSwitch 3
' Sw4     = Turn ON/OFF StarSwitch 4
' Sw5     = Turn ON/OFF StarSwitch 5
' Sw6     = Turn ON/OFF StarSwitch 6
' Sw7     = Turn ON/OFF StarSwitch 7
' Nw1     = Turn ON/OFF NodeSwitch 1
' Nw2     = Turn ON/OFF NodeSwitch 2
' Nw3     = Turn ON/OFF NodeSwitch 3
' Nw4     = Turn ON/OFF NodeSwitch 4
'
'
' This work fine with NT IIS and O'Reilly WebSite
' but I think it work with all WebServer on Intel
' based machine.
'
' Have a good time!
'
' Massimo Graziani
'
' http://www.geocities.com/SouthBeach/Sands/4181/index2.html
'
'=======================================================================
'
Option Explicit
'----------------
'Windows APIs
Declare Sub Sleep Lib "kernel32" (ByVal n As Long)

'----------------
'CARACA Lib
Declare Function CAN_Init Lib "caracalib" (ByVal port As Long) As Long
Declare Sub CAN_End Lib "caracalib" ()
Declare Function CAN_Timeout Lib "caracalib" (ByVal n As Long) As Long
Declare Function CAN_RxMsg Lib "caracalib" (ByRef rx_id As Long, ByRef rx_length As Integer, ByRef rx_flags As Long, ByRef rx_data As Byte) As Long
Declare Function CAN_TxMsg Lib "caracalib" (ByVal tx_id As Long, ByVal tx_length As Integer, ByVal tx_flags As Long, ByRef tx_data As Byte) As Long

Declare Function CAN_Address2ID Lib "caracalib" (ByRef id As Long, ByVal node_address As Integer, ByVal function_code As Integer) As Long

Declare Function CAN_ID2Address Lib "caracalib" (ByVal id As Long, ByRef node_address As Integer, ByRef function_code As Integer) As Long

Const CAN_SETOUTPUT = 1
Const CAN_READTEMP = 2
Const CAN_SETTEMPTHR = 3
Const CAN_GETTEMPTHR = 4
Const CAN_READINPUT = 5
Const CAN_SETINPUTMASK = 6
Const CAN_KEYCHANGE = 10
Const CAN_RC5CODE = 11
Const CAN_TERMCHANGE = 12
Const CAN_SENDTEMP = 13

Const MSG_RTR = 1

Const MAXSTARRELAY = 6
Const MAXNODERELAY = 3
Const COMPORT = 2

Dim StarRelayStatus(MAXSTARRELAY + 1) As Boolean
Dim NodeRelayStatus(MAXNODERELAY + 1) As Boolean
Dim MsgBuffer(8) As Byte

Sub Cgi_Main() 'Here start the program
      Dim n As Long
      Dim rx_id As Long
      Dim rx_length As Integer
      Dim rx_flags As Long
      Dim tx_id As Long
      Dim x As Long
      Dim Command As String
      
      SendHeader "CARACA Web Controller v0.5 Beta"
      
      Command = UCase(CGI_QueryString)
      
      If Command <> "" _
         And Command <> "SW1" _
         And Command <> "SW2" _
         And Command <> "SW3" _
         And Command <> "SW4" _
         And Command <> "SW5" _
         And Command <> "SW6" _
         And Command <> "SW7" _
         And Command <> "NW1" _
         And Command <> "NW2" _
         And Command <> "NW3" _
         And Command <> "NW4" _
         And Command <> "RESET" Then
         
          Send "Syntax:<br>"
          Send "        <i>Null</i> = Display the status"
          Send ("<br>")
          Send "        <b>Reset</b> = Reset the Star and Node"
          Send ("<br>")
          For x = 0 To MAXSTARRELAY
              Send "        <b>Sw" + CStr(x + 1) + "</b> = Turn ON/OFF StarSwitch " + CStr(x + 1) + ""
              Send ("<br>")
          Next x
          For x = 0 To MAXNODERELAY
              Send "        <b>Nw" + CStr(x + 1) + "</b> = Turn ON/OFF NodeSwitch " + CStr(x + 1) + ""
              Send ("<br>")
          Next x
          Send "<br>"
          SendFooter
          CAN_End
          End
      End If
      
      n = CAN_Init(COMPORT)
      
      If n <> 0 Then
          Send "Serial port error!"
          Send "<br>"
          SendFooter
          CAN_End
          End
      End If
      
      Sleep 50

      If Dir(App.Path + "\Status.dat") = "" Then
         For n = 0 To MAXSTARRELAY
             StarRelayStatus(n) = True
         Next
         UpdateStarRelays
         
         Sleep 50
         
         For n = 0 To MAXNODERELAY
             NodeRelayStatus(n) = False
         Next
         UpdateNodeRelays
         
         DataWrite
      End If
      DataRead
      UpdateStarRelays
      UpdateNodeRelays
      Select Case UCase(Command)
            Case "SW1"
                 UpdateStarRelay (0)
            Case "SW2"
                 UpdateStarRelay (1)
            Case "SW3"
                 UpdateStarRelay (2)
            Case "SW4"
                 UpdateStarRelay (3)
            Case "SW5"
                 UpdateStarRelay (4)
            Case "SW6"
                 UpdateStarRelay (5)
            Case "SW7"
                 UpdateStarRelay (6)
            Case "NW1"
                 UpdateNodeRelay (0)
            Case "NW2"
                 UpdateNodeRelay (1)
            Case "NW3"
                 UpdateNodeRelay (2)
            Case "NW4"
                 UpdateNodeRelay (3)
            Case ""
                 Send ("")
            Case "RESET"
                 For n = 0 To MAXSTARRELAY
                     StarRelayStatus(n) = True
                 Next
                 UpdateStarRelays
                 Sleep 50
                 For n = 0 To MAXNODERELAY
                     NodeRelayStatus(n) = False
                 Next
                 UpdateNodeRelays
                 DataWrite
            Case Else
                 SendFooter
                 CAN_End
                 End
      End Select
      
      Sleep 50
      
      Send "Temperature: "
      
      MsgBuffer(0) = 0
      MsgBuffer(1) = 0
      
      x = CAN_Address2ID(tx_id, 2, CAN_READTEMP)
      
      If x <> 0 Then
          Send ("Call error!")
          Send "<br>"
          CAN_End
          SendFooter
          End
      Else
          MsgBuffer(0) = 7
          x = CAN_TxMsg(tx_id, 1, 0, MsgBuffer(0))
          If x <> 0 Then
              Send ("Tx error!")
              Send "<br>"
              CAN_End
              SendFooter
              End
          Else
              x = CAN_Address2ID(tx_id, 2, CAN_SENDTEMP)
              Do
                  MsgBuffer(0) = 0
                  x = CAN_RxMsg(rx_id, rx_length, rx_flags, MsgBuffer(0))
                  If x <> 0 Then
                      Send ("Rx error!")
                      Send "<br>"
                      CAN_End
                      SendFooter
                      End
                  End If
              Loop Until rx_id = tx_id
              If MsgBuffer(1) = 0 Then
                  Send (Str(MsgBuffer(0)) + ".0" + "°C")
              Else
                  Send (Str(MsgBuffer(0) + 0.5) + "°C")
              End If
          End If
      End If
       
      Send "<br><br>"
      
      For x = 0 To MAXSTARRELAY
          Send "StarRele" + CStr(x + 1) + " = " + IIf(StarRelayStatus(x), "ON", "OFF") + "<br>"
          Send ("<br>")
      Next x
      For x = 0 To MAXNODERELAY
          Send "NodeRele" + CStr(x + 1) + " = " + IIf(NodeRelayStatus(x), "ON", "OFF") + "<br>"
          Send ("<br>")
      Next x
      CAN_End
      SendFooter
End Sub

Function DataWrite() As Boolean
      Dim FileLog
      Dim x As Integer
      FileLog = FreeFile
      If Dir(App.Path + "\Status.dat") <> "" Then Kill App.Path + "\Status.dat"
      Open App.Path + "\Status.dat" For Append As #FileLog
      For x = 0 To MAXSTARRELAY
          Print #FileLog, IIf(StarRelayStatus(x), "1", "0")
      Next x
      For x = 0 To MAXNODERELAY
          Print #FileLog, IIf(NodeRelayStatus(x), "1", "0")
      Next x
      Close #FileLog
      If Dir(App.Path + "\Status.dat") <> "" Then DataWrite = True Else DataWrite = False
End Function

Function DataRead() As Boolean
      Dim FileLog
      Dim testo As String
      Dim x As Integer
      FileLog = FreeFile
      If Dir(App.Path + "\Status.dat") = "" Then
         DataRead = False
         Exit Function
      End If
      Open App.Path + "\Status.dat" For Input Access Read As #FileLog
      For x = 0 To MAXSTARRELAY
          Line Input #FileLog, testo
          If testo = "1" Then StarRelayStatus(x) = True Else StarRelayStatus(x) = False
      Next x
      For x = 0 To MAXNODERELAY
          Line Input #FileLog, testo
          If testo = "1" Then NodeRelayStatus(x) = True Else NodeRelayStatus(x) = False
      Next x
      Close #FileLog
End Function

Private Sub UpdateStarRelays()
      Dim j As Integer
      Dim n As Long
      Dim x As Long
      n = 0
      x = 1
      For j = 0 To MAXSTARRELAY
          If Not StarRelayStatus(j) Then
              n = n + x
          End If
          x = x * 2
      Next
      
      MsgBuffer(0) = 3    '3 = assign, 2 = toggle mask, 1 = set mask, 0 = clear mask
      MsgBuffer(1) = n
      
      x = CAN_Address2ID(n, 1, CAN_SETOUTPUT)
      If x <> 0 Then
          MsgBox "CAN_Address2ID error!"
      Else
          x = CAN_TxMsg(n, 2, 0, MsgBuffer(0))
          If x <> 0 Then
              MsgBox "Tx error!"
          End If
      End If
      
      Sleep 50

End Sub

Private Sub UpdateNodeRelays()
      Dim j As Integer
      Dim n As Long
      Dim x As Long
      
      n = 0
      x = 1
      For j = 0 To MAXNODERELAY
          If NodeRelayStatus(j) Then
              n = n + x
          End If
          x = x * 2
          If j = 1 Or j = 2 Then
              x = x * 2
          End If
      Next
      
      MsgBuffer(0) = 3    '3 = assign, 2 = toggle mask, 1 = set mask, 0 = clear mask
      MsgBuffer(1) = n
      
      x = CAN_Address2ID(n, 2, CAN_SETOUTPUT)
      If x <> 0 Then
          MsgBox "CAN_Address2ID error!"
      Else
          x = CAN_TxMsg(n, 2, 0, MsgBuffer(0))
          If x <> 0 Then
              MsgBox "Tx error!"
          End If
      End If
      
      Sleep 50
      
End Sub

Private Sub UpdateNodeRelay(Index As Integer)
      Dim val As Boolean
      val = Not NodeRelayStatus(Index)
      NodeRelayStatus(Index) = val
      Sleep 50
      UpdateNodeRelays
      Sleep 50
      DataWrite
End Sub

Private Sub UpdateStarRelay(Index As Integer)
      Dim val As Boolean
      val = Not StarRelayStatus(Index)
      StarRelayStatus(Index) = val
      Sleep 50
      UpdateStarRelays
      Sleep 50
      DataWrite
End Sub

