# Create the output directory if it doesn't exist
New-Item -ItemType Directory -Force -Path "Binaries"

# Use Python to assemble the NASM files
python compile_nasm.py

# 1. Compile ALL C/C++ source files into object files
& "C:\Program Files\LLVM\bin\clang.exe" -target i686-pc-none-elf -ffreestanding -m32 -mno-sse -fno-stack-protector -O2 -g -c "kernel.cpp" -o "Binaries/kernel.o" "-I."
& "C:\Program Files\LLVM\bin\clang.exe" -target i686-pc-none-elf -ffreestanding -m32 -mno-sse -fno-stack-protector -O2 -g -c "ata.cpp" -o "Binaries/ata.o" "-I."
& "C:\Program Files\LLVM\bin\clang.exe" -target i686-pc-none-elf -ffreestanding -m32 -mno-sse -fno-stack-protector -O2 -g -c "diskio.cpp" -o "Binaries/diskio.o" "-I."

# Note: ff.c is clean C code, so we compile it using the exact same flags
& "C:\Program Files\LLVM\bin\clang.exe" -target i686-pc-none-elf -ffreestanding -m32 -mno-sse -fno-stack-protector -O2 -g -c "ff.c" -o "Binaries/ff.o" "-I."

# 2. Link EVERYTHING together cleanly
& "C:\Program Files\LLVM\bin\ld.lld.exe" -m elf_i386 --image-base=0 -Ttext 0x1000 "Binaries/entry.o" "Binaries/kernel.o" "Binaries/ata.o" "Binaries/diskio.o" "Binaries/ff.o" -o "Binaries/full_kernel.bin" --oformat=binary

# Build OS Image using our 1.44MB padding script
python build_img.py

# 3. Boot the OS image in QEMU (Emulating a hard drive alongside your floppy boot)
& "C:\msys64\ucrt64\bin\qemu-system-x86_64.exe" -drive format=raw,file="Binaries/OS.bin",index=0,if=floppy -m 128M
