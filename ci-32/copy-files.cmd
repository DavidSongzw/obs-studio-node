copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-core-file-l1-2-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-core-file-l2-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-core-localization-l1-2-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-core-processthreads-l1-1-1.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-core-synch-l1-2-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-core-timezone-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-convert-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-environment-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-filesystem-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-heap-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-locale-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-math-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-multibyte-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-runtime-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-stdio-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-string-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-time-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\api-ms-win-crt-utility-l1-1-0.dll" "%SLFullDistributePath%\obs-studio-node\"

copy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.CRT\vcruntime140.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.CRT\msvcp140.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.CRT\concrt140.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\ucrtbase.dll" "%SLFullDistributePath%\obs-studio-node\"
copy "C:\Windows\System32\VCRUNTIME140_1.dll" "%SLFullDistributePath%\obs-studio-node\"
REM msdia140.dll is used to convert pdb files to upload to Sentry
REM copy "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\DIA SDK\bin\msdia140.dll" "%BUILD_SOURCESDIRECTORY%\"