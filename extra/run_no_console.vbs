Set oShell = CreateObject ("Wscript.Shell") 
Dim strArgs
strArgs = "cmd /c run.bat"
oShell.Run strArgs, 0, false