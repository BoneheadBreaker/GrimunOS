# Create the output directory if it doesnt exist
New-Item -ItemType Directory -Force -Path "Binaries"

# use Python to assemble the NASM files
python compile_nasm.py

# Compile the C++ kernel into a clean object file
& "C:\Program Files\LLVM\bin\clang.exe" -target i686-pc-none-elf -ffreestanding -m32 -mno-sse -fno-stack-protector -O2 -g -c "kernel.cpp" -o "Binaries/kernel.o"

# Link everything cleanly (entry.o comes first to align at address 0x1000)
& "C:\Program Files\LLVM\bin\ld.lld.exe" -m elf_i386 --image-base=0 -Ttext 0x1000 "Binaries/entry.o" "Binaries/kernel.o" -o "Binaries/full_kernel.bin" --oformat=binary

# Build OS Image using our 1.44MB padding script
python build_img.py

# Boot the OS image in QEMU as a standard floppy drive target
& "C:\msys64\ucrt64\bin\qemu-system-x86_64.exe" -drive format=raw,file="Binaries/OS.bin",index=0,if=floppy -m 128M
