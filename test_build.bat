@echo off
setlocal
set ROOT=%~dp0

echo =========================================
echo GSEA - Build and Test Script
echo =========================================
echo.

echo 1. Cleaning previous build...
mingw32-make clean
echo.

echo 2. Compiling project...
mingw32-make
if errorlevel 1 (
    echo [X] Compilation failed!
    exit /b 1
)
echo [OK] Compilation successful!
echo.

if not exist "%ROOT%output" mkdir "%ROOT%output"

REM Ensure test input exists
if not exist "%ROOT%test\test_data.txt" (
    echo [X] Test input not found: "%ROOT%test\test_data.txt"
    exit /b 1
)

echo 3. Testing compression...
echo Running: "%ROOT%bin\gsea.exe" -c -i "%ROOT%test\test_data.txt" -o "%ROOT%output\test1.huf"
"%ROOT%bin\gsea.exe" -c -i "%ROOT%test\test_data.txt" -o "%ROOT%output\test1.huf"
if errorlevel 1 (
    echo [X] Compression failed!
    exit /b 1
)
echo [OK] Compression successful!
echo.

echo 4. Testing decompression...
echo Running: "%ROOT%bin\gsea.exe" -d -i "%ROOT%output\test1.huf" -o "%ROOT%output\test1_restored.txt"
"%ROOT%bin\gsea.exe" -d -i "%ROOT%output\test1.huf" -o "%ROOT%output\test1_restored.txt"
if errorlevel 1 (
    echo [X] Decompression failed!
    exit /b 1
)
echo [OK] Decompression successful!
echo.

echo 5. Verifying data integrity...
fc "%ROOT%test\test_data.txt" "%ROOT%output\test1_restored.txt" > nul
if errorlevel 1 (
    echo [X] Data integrity check failed!
    exit /b 1
)
echo [OK] Data integrity verified!
echo.

REM Encryption tests (require key)
echo 6. Testing encryption...
echo Running: "%ROOT%bin\gsea.exe" -e -i "%ROOT%test\test_data.txt" -o "%ROOT%output\test2.aes" -k test123
"%ROOT%bin\gsea.exe" -e -i "%ROOT%test\test_data.txt" -o "%ROOT%output\test2.aes" -k test123
if errorlevel 1 (
    echo [X] Encryption failed!
    exit /b 1
)
echo [OK] Encryption successful!
echo.

echo 7. Testing decryption...
echo Running: "%ROOT%bin\gsea.exe" -u -i "%ROOT%output\test2.aes" -o "%ROOT%output\test2_restored.txt" -k test123
"%ROOT%bin\gsea.exe" -u -i "%ROOT%output\test2.aes" -o "%ROOT%output\test2_restored.txt" -k test123
if errorlevel 1 (
    echo [X] Decryption failed!
    exit /b 1
)
echo [OK] Decryption successful!
echo.

echo 8. Verifying encryption integrity...
fc "%ROOT%test\test_data.txt" "%ROOT%output\test2_restored.txt" > nul
if errorlevel 1 (
    echo [X] Encryption integrity check failed!
    exit /b 1
)
echo [OK] Encryption integrity verified!
echo.

echo =========================================
echo [OK] TESTS COMPLETED (check outputs above)
echo =========================================
echo.
dir "%ROOT%output"
echo.
pause
endlocal

