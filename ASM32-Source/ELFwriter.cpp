#include "constants.hpp"
#include "ASM32.hpp"


// ELF Format output

using namespace ELFIO;
elfio writer;
section* text_sec = nullptr;
segment* text_seg = nullptr;
section* data_sec = nullptr;
segment* data_seg = nullptr;
section* note_sec = nullptr;

int createELF() {
    writer.create(ELFCLASS32, ELFDATA2MSB);
    writer.set_os_abi(ELFOSABI_LINUX);
    writer.set_type(ET_EXEC);
    writer.set_machine(EM_X86_64);
    return 0;
}

int createTextSection() {
    text_sec = writer.sections.add(".text");
    text_sec->set_type(SHT_PROGBITS);
    text_sec->set_flags(SHF_ALLOC | SHF_EXECINSTR);
    text_sec->set_addr_align(0x10);
    return 0;
}

int addTextSectionData() {
    if (O_TextSectionData == FALSE) {
        text_sec->set_data(O_TEXT, 4);
        O_TextSectionData = TRUE;
    }
    else
    {
        text_sec->append_data(O_TEXT, 4);
    }
    return 0;
}

int createTextSegment() {
    text_seg = writer.segments.add();
    text_seg->set_type(PT_LOAD);
    text_seg->set_virtual_address(O_CODE_ADDR);
    text_seg->set_physical_address(O_CODE_ADDR);
    text_seg->set_flags(PF_X | PF_R);
    text_seg->set_align(O_CODE_ALIGN);
    return 0;
}
int addTextSectionToSegment() {
    text_seg->add_section(text_sec, text_sec->get_addr_align());
    return 0;
}

int createDataSection() {
    data_sec = writer.sections.add(".data");
    data_sec->set_type(SHT_PROGBITS);
    data_sec->set_flags(SHF_ALLOC | SHF_WRITE);
    data_sec->set_addr_align(0x4);
    return 0;
}

int addDataSectionData() {
    if (O_DataSectionData == FALSE) {
        data_sec->set_data(O_DATA, O_dataLen);
        O_DataSectionData = TRUE;
    }
    else
    {
        data_sec->append_data(O_DATA, O_dataLen);
    }
    return 0;
}

int createDataSegment() {
    data_seg = writer.segments.add();
    data_seg->set_type(PT_LOAD);
    data_seg->set_virtual_address(O_DATA_ADDR);
    data_seg->set_physical_address(O_DATA_ADDR);
    data_seg->set_flags(PF_W | PF_R);
    data_seg->set_align(O_DATA_ALIGN);
    return 0;
}
int addDataSectionToSegment() {
    data_seg->add_section(data_sec, data_sec->get_addr_align());
    return 0;
}

int addNote() {
    note_sec = writer.sections.add(".note");
    note_sec->set_type(SHT_NOTE);
    note_sec->set_addr_align(1);
    note_section_accessor note_writer(writer, note_sec);
    note_writer.add_note(0x01, "Created by ASM32", 0, 0);
    note_writer.add_note(0x01, VERSION, 0, 0);
    return 0;
}

int writeElfFile() {
    writer.set_entry(O_ENTRY);
    writer.save("ASM32.out");
    return 0;
}

/*
int WriteELF() {


    // You can't proceed without this function call!
    writer.create(ELFCLASS32, ELFDATA2MSB);

    writer.set_os_abi(ELFOSABI_LINUX);
    writer.set_type(ET_EXEC);
    writer.set_machine(EM_X86_64);

    // Create text section
    section* text_sec = writer.sections.add(".text");
    text_sec->set_type(SHT_PROGBITS);
    text_sec->set_flags(SHF_ALLOC | SHF_EXECINSTR);
    text_sec->set_addr_align(0x10);

    // Add data into it
    char text[] = {
        '\xB8', '\x04', '\x00', '\x00', '\x00', // mov eax, 4
        '\xBB', '\x01', '\x00', '\x00', '\x00', // mov ebx, 1
        '\xB9', '\x00', '\x00', '\x00', '\x00', // mov ecx, msg
        '\xBA', '\x0E', '\x00', '\x00', '\x00', // mov edx, 14
        '\xCD', '\x80',                         // int 0x80
        '\xB8', '\x01', '\x00', '\x00', '\x00', // mov eax, 1
        '\xCD', '\x80'                          // int 0x80
    };
    // Adjust data address for 'msg'
    //*(uint32_t*)(text + 11) = O_DATA_ADDR;

    text_sec->set_data(text, sizeof(text));

    // Create a loadable segment
    segment* text_seg = writer.segments.add();
    text_seg->set_type(PT_LOAD);
    text_seg->set_virtual_address(O_CODE_ADDR);
    text_seg->set_physical_address(O_CODE_ADDR);
    text_seg->set_flags(PF_X | PF_R);
    text_seg->set_align(O_CODE_ALIGN);

    // Add text section into program segment
    text_seg->add_section(text_sec, text_sec->get_addr_align());

    // Create data section
    section* data_sec = writer.sections.add(".data");
    data_sec->set_type(SHT_PROGBITS);
    data_sec->set_flags(SHF_ALLOC | SHF_WRITE);
    data_sec->set_addr_align(0x4);

    data_sec->set_data(p_data, O_dataOfs);

    // Create a read/write segment
    segment* data_seg = writer.segments.add();
    data_seg->set_type(PT_LOAD);
    data_seg->set_virtual_address(O_DATA_ADDR);
    data_seg->set_physical_address(O_DATA_ADDR);
    data_seg->set_flags(PF_W | PF_R);
    data_seg->set_align(O_DATA_ALIGN);

    // Add data section into program segment
    data_seg->add_section(data_sec, data_sec->get_addr_align());

    // Add optional signature for the file producer
    section* note_sec = writer.sections.add(".note");
    note_sec->set_type(SHT_NOTE);
    note_sec->set_addr_align(1);
    note_section_accessor note_writer(writer, note_sec);
    note_writer.add_note(0x01, "Created by ASM32", 0, 0);
    note_writer.add_note(0x01, VERSION, 0, 0);

    // Setup entry point. Usually, a linker sets this address on base of
    // ‘_start’ label.
    // In this example, the code starts at the first address of the
    // 'text_seg' segment. Therefore, the start address is set
    // to be equal to the segment location
    writer.set_entry(O_ENTRY);

    // Create ELF file
    writer.save("ASM32.out");

    return 0;

}
*/