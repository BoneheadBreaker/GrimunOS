import os

# Only combine the bootloader and the actual compiled kernel code
files_to_combine = [
    "Binaries/boot.bin", 
    "Binaries/full_kernel.bin"
]
output_file = "Binaries/OS.bin"

# Exact byte size of a standard 1.44MB floppy disk
FLOPPY_SIZE = 1440 * 1024 

with open(output_file, "wb") as f_out:
    # 1. Write the active code blocks
    for filename in files_to_combine:
        with open(filename, "rb") as f_in:
            f_out.write(f_in.read())
            
    # Calculate remaining bytes needed to reach 1.44MB exactly
    current_size = f_out.tell()
    padding_needed = FLOPPY_SIZE - current_size
    
    if padding_needed > 0:
        f_out.write(b'\x00' * padding_needed)
        print(f"[Success] Generated a valid 1.44MB OS Image with {padding_needed} bytes of safety padding.")
    else:
        print("[Error] Your kernel is too massive to fit inside a standard disk sector allocation size!")
