# SHMessageBoxCheck
#   Works like MessageBox but includes a checkbox that gives the user the option not to show the message box again.
#   In that case the return value (first value on the stack) is always set to the last user choice
#
# See
#   http://nsis.sourceforge.net/SHMessageBoxCheck   (documentation)
#   https://msdn.microsoft.com/library/windows/desktop/bb773836.aspx   (implementation details)
#

# types to indicate the buttons displayed in the message box
!define MB_OK                0x00000000
!define MB_OKCANCEL          0x00000001
!define MB_ABORTRETRYIGNORE  0x00000002 # not officially supported, use at your own risk!
!define MB_YESNOCANCEL       0x00000003 # not officially supported, use at your own risk!
!define MB_YESNO             0x00000004
!define MB_RETRYCANCEL       0x00000005 # not officially supported, use at your own risk!
!define MB_CANCELTRYCONTINUE 0x00000006 # not officially supported, use at your own risk!
!define MB_HELP              0x00004000 # not officially supported, use at your own risk!

# types to display an icon in the message box
!define MB_ICONHAND          0x00000010
!define MB_ICONQUESTION      0x00000020 # MS bug: Same as MB_ICONEXCLAMATION
!define MB_ICONEXCLAMATION   0x00000030
!define MB_ICONINFORMATION   0x00000040


# return values
!define IDOK        1
!define IDCANCEL    2
!define IDABORT     3
!define IDRETRY     4
!define IDIGNORE    5
!define IDYES       6
!define IDNO        7
!define IDCONTINUE 11
!define IDTRYAGAIN 10



# the user's previous choice (i.e. the button clicked in the message box)
Var _lastReturnValue

# The value that the call to SHMessageBoxCheck should return when the user chose not to display the message box again
!define _DEFAULT 9999

# Windows XP does not expose the function name, so we have to specify the function by ordinal value
!ifdef NSIS_UNICODE
    !define _SHMessageBoxCheck_Ordinal 191
!else
    !define _SHMessageBoxCheck_Ordinal 185
!endif



!macro SHMessageBoxCheckInit _UNIQUE_STRING
    # SHMessageBoxCheck stores the user's choice not to display the message box again in the registry, see
    #   HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DontShowMeThisDialogAgain
    !ifdef _PSZ_REG_VAL
        !error "Only call SHMessageBoxCheckInit once and make sure to call SHMessageBoxCheckCleanup before using it again"
    !else
        # the unique string used to identify this message (and name of the registry value used to store the checkbox status)
        !define _PSZ_REG_VAL ${_UNIQUE_STRING}
    !endif

    # make sure the registry value is not yet set (for whatever reason)
    ${SHMessageBoxCheckCleanup}
!macroend
!define SHMessageBoxCheckInit "!insertmacro SHMessageBoxCheckInit"


!macro SHMessageBoxCheckCleanup
    # delete the registry key that is used to store the checkbox status so we can start fresh next time
    DeleteRegValue HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DontShowMeThisDialogAgain" "${_PSZ_REG_VAL}"
!macroend
!define SHMessageBoxCheckCleanup "!insertmacro SHMessageBoxCheckCleanup"


!macro SHMessageBoxCheck _CAPTION _TEXT _TYPE
    # this would be the simple way (by name)
    # System::Call "shlwapi::SHMessageBoxCheck(p $HWNDPARENT, t '${_TEXT}', t '${_CAPTION}', i ${_TYPE}, i ${_DEFAULT}, t '${_PSZ_REG_VAL}') i .r0"

    # for backwards-compatibility we get the process address by specifying the function's ordinal value
    System::Call "kernel32::GetModuleHandle(t 'shlwapi.dll') p .s"
    System::Call "kernel32::GetProcAddress(p s, i ${_SHMessageBoxCheck_Ordinal}) p .r0"
    System::Call "::$0(p $HWNDPARENT, t '${_TEXT}', t '${_CAPTION}', i ${_TYPE}, i ${_DEFAULT}, t '${_PSZ_REG_VAL}') i .r0"

    # save the user's choice (unless the default value was returned - then don't update and return the saved choice)
    StrCmp $0 ${_DEFAULT} +2 0
        StrCpy $_lastReturnValue $0

    # push the return value to the stack
    Push $_lastReturnValue
!macroend
!define SHMessageBoxCheck "!insertmacro SHMessageBoxCheck"