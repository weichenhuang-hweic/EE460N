make
./assembler.linux assembly/add.asm assembly/add.out
./assembler.linux assembly/data.asm assembly/data.out
./assembler.linux assembly/pagetable.asm assembly/pagetable.out
./assembler.linux assembly/except_page.asm assembly/except_page.out
./assembler.linux assembly/except_prot.asm assembly/except_prot.out
./assembler.linux assembly/except_unaligned.asm assembly/except_unaligned.out
./assembler.linux assembly/except_unknown.asm assembly/except_unknown.out
./assembler.linux assembly/int.asm assembly/int.out
./assembler.linux assembly/vector_table.asm assembly/vector_table.out
clear
./simulate ucode5 assembly/pagetable.out assembly/add.out assembly/data.out assembly/except_page.out assembly/except_prot.out assembly/except_unaligned.out assembly/except_unknown.out assembly/int.out assembly/vector_table.out