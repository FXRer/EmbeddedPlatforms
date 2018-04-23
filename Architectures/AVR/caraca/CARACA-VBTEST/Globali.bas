Attribute VB_Name = "mod_globals"
Option Explicit

Public ComPort As Integer
Public MsgBuffer(8) As Byte

Public Const ASSTABLE_START = 12
Public Const ASSENTRY_MAX = 14      'last valid asstable entry
Public Const ASSENTRY_SIZE = 16

Public AssBuffer(ASSENTRY_SIZE) As Long
Public AssCounter As Integer
Public AssBase As Integer
Public AssOk As Boolean

Public Const HELLOENTRY_MAX = 200

Public HelloBuffer_SerNum(HELLOENTRY_MAX) As Integer
Public HelloBuffer_Address(HELLOENTRY_MAX) As Integer
Public Hellobuffer_Type(HELLOENTRY_MAX) As Byte
Public HelloIndex As Integer

'CARACA function codes
Public Const CAN_SETOUTPUT = 1 'A Set Outputs
Public Const CAN_SETDIMMER = 2 'A Set dimmer
Public Const CAN_SETINPUTMASK = 3 'A Set input mask
' *  4 :A Set thermostat thresholds
' *
Public Const CAN_GETOUTPUT = 7 'A Request output
Public Const CAN_GETDIMMER = 8 'A Request dimmer
Public Const CAN_GETINPUT = 9  'A Request input
Public Const CAN_READTEMP = 10 'A Request thermometer
' *
Public Const CAN_INPUTCHANGE = 14 'E Input change
Public Const CAN_RC5KEY = 15      'E RC5 key code
' * 16 :E Thermostat change
Public Const CAN_SENDOUTPUT = 17 'E Send output
Public Const CAN_SENDDIMMER = 18 'E Send dimmer
Public Const CAN_SENDINPUT = 19 'E Send input
Public Const CAN_SENDTEMP = 20  'E Send thermometer
' *
Public Const CAN_WRITE_EEP = 25 'A Write eeprom data
Public Const CAN_READ_EEP = 26  'A Request eeprom data
Public Const CAN_SEND_EEP = 27  'E Send eeprom data
' *
Public Const CAN_SETADDRESS = 29       'A Set address
Public Const CAN_SENDSERIALNUM = 30    'E Send Hello message (serial num)
Public Const CAN_GETSERIALNUM = 31     'A Request Hello message (serial num)


Public Const MSG_RTR = 1




