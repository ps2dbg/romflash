
#ifndef _ROMFLASH_INTERNAL_H
#define _ROMFLASH_INTERNAL_H

#include "irx_imports.h"
#include <stdarg.h>

struct rominfo_item
{
	const char *m_device_name;
	void *m_delay_register;
	int m_delay_register_value;
	void *m_base_address;
	int m_chunk_size;
	int m_config_offset_1;
	int m_config_offset_2;
	int m_flash_size;
	int m_manufacturer;
	int m_device_id;
	int m_write_width;
	int m_address_register;
	int m_flash_offset;
	int m_flash_blocksize;
};

struct table_item
{
	const char *m_table;
	struct rominfo_item *m_list;
};

struct type_list_item
{
	u8 m_device_id_8bit;
	u8 m_pad;
	u16 m_device_id_16bit;
	int m_erase_size;
	int m_sector_range_elements;
	struct type_list_inner_item *m_sector_range;
};

struct type_list_inner_item
{
	int m_offset;
	int m_length;
};

struct romflash_info
{
	void *m_option_addr;
	int m_option_size;
	int m_option_offset;
	int m_option_block;
	int m_option_verify;
	int m_option_retry;
	int m_fd;
	unsigned int m_option_flags;
};

struct flash_rom_work
{
	struct type_list_item *m_type_list_element;
	int m_update_offset;
	int m_update_length;
	int m_update_sector_lower;
	int m_update_sector_upper;
};

struct romflash_sigcheck_res
{
	volatile void *m_config_addr_1;
	volatile void *m_config_addr_2;
	volatile void *m_base;
	volatile void *m_base_end;
	int m_manufacturer_res;
	int m_device_id_res;
};

int vprintf(const char *format, va_list va);
void _wait(int count);
void flash_reset(struct rominfo_item *rii);
void flash_write(struct rominfo_item *rii, int offset, void *src_addr, int block_size);
void flash_write_fast(struct rominfo_item *rii, int offset, void *src_addr, int block_size);
int flash_verify(struct rominfo_item *rii, int offset, u8 *addr, int block_size);
struct rominfo_item *flash_probe(struct rominfo_item *rii);
void flash_chip_erace(struct rominfo_item *rii);
void flash_sector_erace_addr(struct rominfo_item *rii, int offset);
struct type_list_item *flash_type_search(const struct rominfo_item *rii);
int flash_sector_search(struct type_list_item *tli, int offset_absolute, int block_size, struct flash_rom_work *frw);

extern int probe_verbose;
extern int non_windup;
extern int percentage;

#endif
