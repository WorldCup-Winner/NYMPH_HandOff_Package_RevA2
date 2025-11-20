@echo off
echo [clean] Cleaning build artifacts...

if exist dist rmdir /s /q dist
mkdir dist

echo [clean] Build artifacts cleaned.

