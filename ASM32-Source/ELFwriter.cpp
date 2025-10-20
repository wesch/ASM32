#include "constants.hpp"
#include "ASM32.hpp"

/// @file
/// \brief ELF output module for the ASM32 assembler.
/// \details
/// This file contains routines that generate ELF output files
/// for the assembled program. It defines the ELF writer, sections,
/// and segments, and provides helper functions to build the `.text`
/// and `.data` sections, insert machine code and data, and write
/// the final ELF executable.  

using namespace ELFIO;

elfio writer;                  ///< ELF writer instance.
section* text_sec = nullptr;   ///< Pointer to the .text section.
segment* text_seg = nullptr;   ///< Pointer to the program segment containing .text.
section* data_sec = nullptr;   ///< Pointer to the .data section.
segment* data_seg = nullptr;   ///< Pointer to the program segment containing .data.
section* note_sec = nullptr;   ///< Pointer to the .note section.


// --------------------------------------------------------------------------------
//  ELF Initialization
// --------------------------------------------------------------------------------

/// \brief Create and initialize the ELF header.
/// \details
/// Configures the ELF writer for a 32-bit, big-endian, Linux executable
/// targeting the x86_64 machine.
/// \return 0 on success.
int createELF() {
    writer.create(ELFCLASS32, ELFDATA2MSB);
    writer.set_os_abi(ELFOSABI_LINUX);
    writer.set_type(ET_EXEC);
    writer.set_machine(EM_X86_64);
    return 0;
}


// --------------------------------------------------------------------------------
//  .text Section and Segment
// --------------------------------------------------------------------------------

/// \brief Create the `.text` section.
/// \details
/// The `.text` section holds machine instructions and is marked
/// as executable and allocated in memory.
/// \return 0 on success.
int createTextSection(char* name) {
    text_sec = writer.sections.add(name);
    text_sec->set_type(SHT_PROGBITS);
    text_sec->set_flags(SHF_ALLOC | SHF_EXECINSTR);
    text_sec->set_addr_align(0x10);
    return 0;
}

/// \brief Add instruction data to the `.text` section.
/// \details
/// If this is the first addition, the section data is set directly.  
/// On subsequent calls, new machine code is appended.
/// \return 0 on success.
int addTextSectionData() {
    if (elfCodeSectionStatus == FALSE) {
        text_sec->set_data(elfCode, 4);
        elfCodeSectionStatus = TRUE;
    }
    else {
        text_sec->append_data(elfCode, 4);
    }
    return 0;
}

/// \brief Create the `.text` segment.
/// \details
/// The `.text` segment is loadable and contains the `.text` section.  
/// It is marked as readable and executable.
/// \return 0 on success.
int createTextSegment() {
    text_seg = writer.segments.add();
    text_seg->set_type(PT_LOAD);
    text_seg->set_virtual_address(elfCodeAddr);
    text_seg->set_physical_address(elfCodeAddr);
    text_seg->set_flags(PF_X | PF_R);
    text_seg->set_align(elfCodeAlign);
    return 0;
}

/// \brief Attach the `.text` section to the `.text` segment.
/// \return 0 on success.
int addTextSectionToSegment() {
    text_seg->add_section(text_sec, text_sec->get_addr_align());
    return 0;
}


// --------------------------------------------------------------------------------
//  .data Section and Segment
// --------------------------------------------------------------------------------

/// \brief Create the `.data` section.
/// \details
/// The `.data` section holds initialized global and static data.
/// It is marked as writable and allocated in memory.
/// \return 0 on success.
int createDataSection(char* name) {
    data_sec = writer.sections.add(name);
    data_sec->set_type(SHT_PROGBITS);
    data_sec->set_flags(SHF_ALLOC | SHF_WRITE);
    data_sec->set_addr_align(0x4);
    return 0;
}

/// \brief Add data to the `.data` section.
/// \param data Pointer to the data buffer.
/// \param len  Length of the data in bytes.
/// \return 0 on success.
int addDataSectionData(char* data, int len) {
    if (elfDataSectionStatus == FALSE) {
        data_sec->set_data(data, len);
        elfDataSectionStatus = TRUE;
    }
    else {
        data_sec->append_data(data, len);
    }
    return 0;
}

/// \brief Create the `.data` segment.
/// \details
/// The `.data` segment is loadable and contains the `.data` section.  
/// It is marked as readable and writable.
/// \return 0 on success.
int createDataSegment() {
    data_seg = writer.segments.add();
    data_seg->set_type(PT_LOAD);
    data_seg->set_virtual_address(elfDataAddr);
    data_seg->set_physical_address(elfDataAddr);
    data_seg->set_flags(PF_W | PF_R);
    data_seg->set_align(elfDataAlign);
    return 0;
}

/// \brief Attach the `.data` section to the `.data` segment.
/// \return 0 on success.
int addDataSectionToSegment() {
    data_seg->add_section(data_sec, data_sec->get_addr_align());
    return 0;
}


// --------------------------------------------------------------------------------
//  .note Section
// --------------------------------------------------------------------------------

/// \brief Add a `.note` section to the ELF file.
/// \details
/// The `.note` section includes metadata, such as a producer string
/// ("Created by ASM32") and the assembler version.
/// \return 0 on success.
int addNote() {
    note_sec = writer.sections.add(".note");
    note_sec->set_type(SHT_NOTE);
    note_sec->set_addr_align(1);
    note_section_accessor note_writer(writer, note_sec);
    note_writer.add_note(0x01, "Created by ASM32", 0, 0);
    note_writer.add_note(0x01, VERSION, 0, 0);
    return 0;
}


// --------------------------------------------------------------------------------
//  Finalization
// --------------------------------------------------------------------------------

/// \brief Write the ELF file to disk.
/// \details
/// Sets the ELF entry point to the global entry point and writes
/// the final ELF binary (`ASM32.out`) to disk.
/// \return 0 on success.
int writeElfFile(char* file) {
    writer.set_entry(elfEntryPoint);
    writer.save(file);
    return 0;
}
