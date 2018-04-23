Attribute VB_Name = "Profile"
Declare Function GetPrivateProfileString Lib "kernel32" Alias "GetPrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As Any, ByVal lpDefault As String, ByVal lpReturnedString As String, ByVal nSize As Long, ByVal lpFileName As String) As Long
Declare Function WritePrivateProfileString Lib "kernel32" Alias "WritePrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As Any, ByVal lpString As Any, ByVal lpFileName As String) As Long

Public Function ReadFormattedFile(ByVal FileName As String, ByVal Key As String, ByVal Default As String) As String
Dim ReturnedString As String
Dim i As Integer
Dim Pos As Long
Dim Pos1 As Long
Dim SectionName As String
Dim cd As String

cd = CurDir
Pos1 = InStr(cd, "\")
Pos = InStr(FileName, "\")
If Pos = 0 Then     'relative path
    If Pos1 = 0 Then
        FileName = cd + FileName
    Else
        If Right(cd, 1) = "\" Then
            FileName = cd + FileName
        Else
            FileName = cd + "\" + FileName
        End If
    End If
End If

ReturnedString = Space(128)
If Not GetPrivateProfileString("Parameters", Key, Default, ReturnedString, 128, FileName) <> True Then
    MsgBox ("File " + FileName + " non valido o assente.")
End If
ReturnedString = Trim(ReturnedString)
'caviamo i null (non ho trovato di meglio...)
For i = 1 To Len(ReturnedString)
    If Asc(Mid(ReturnedString, i, 1)) = 0 Then
        Mid(ReturnedString, i, 1) = " "
    End If
Next i
ReadFormattedFile = Trim(ReturnedString)
End Function


Public Sub WriteConfig(ByVal ProfileName As String, ByVal Key As String, ByVal Value As String)
Dim Pos As Long
Dim Pos1 As Long
Dim cd As String

cd = CurDir
Pos1 = InStr(cd, "\")
Pos = InStr(ProfileName, "\")
If Pos = 0 Then     'relative path
    If Pos1 = 0 Then
        ProfileName = cd + ProfileName
    Else
        If Right(cd, 1) = "\" Then
            ProfileName = cd + ProfileName
        Else
            ProfileName = cd + "\" + ProfileName
        End If
    End If
End If
Pos = WritePrivateProfileString("Parameters", Key, Value, ProfileName)
End Sub
