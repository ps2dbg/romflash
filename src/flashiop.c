
#include "romflash_internal.h"

static unsigned int pp_14 = -1;

static void probe_message(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	if ( probe_verbose )
	{
		CpuEnableIntr();
		vprintf(fmt, va);
		CpuDisableIntr();
	}
	va_end(va);
}

void _wait(int count)
{
	int wait_tmp;

	for ( wait_tmp = 0; wait_tmp < count; wait_tmp += 1 )
	{
		__asm__ __volatile__("" : "+g"(wait_tmp) : :);
	}
}

#if 0
static int delay_reset()
{
	return 0;
}
#endif

void flash_reset(struct rominfo_item *rii)
{
	CpuDisableIntr();
	switch ( rii->m_write_width )
	{
		case 8:
			*((vu8 *)rii->m_base_address + rii->m_config_offset_1) = 0xF0;
			break;
		case 16:
			*((vu16 *)rii->m_base_address + rii->m_config_offset_1) = 0xF0;
			break;
		default:
			break;
	}
	CpuEnableIntr();
}

static void print_percentage(unsigned int in_percentage, unsigned int in_bar_size)
{
	unsigned int bar_size_x2;

	bar_size_x2 = 2 * in_bar_size;
	if ( percentage )
	{
		if ( !in_percentage )
		{
			char *barstr;

			barstr = __builtin_alloca(bar_size_x2 + 1);
			memset(barstr, '.', in_bar_size);
			memset(barstr + in_bar_size, '\b', in_bar_size);
			barstr[bar_size_x2] = 0;
			CpuEnableIntr();
			printf(barstr);
			CpuDisableIntr();
			pp_14 = 0;
		}
		if ( !in_bar_size || !(0x64 / in_bar_size) )
			__builtin_trap();
		if ( pp_14 != in_percentage / (0x64 / in_bar_size) )
		{
			CpuEnableIntr();
			printf("o");
			CpuDisableIntr();
			pp_14 = in_percentage / (0x64 / in_bar_size);
		}
	}
}

static void _byte_write(vu8 *cfg_addr_1, vu8 *cfg_addr_2, vu8 *dst_addr, u8 src_item)
{
	*cfg_addr_1 = 0xF0;
	*cfg_addr_1 = 0xAA;
	*cfg_addr_2 = 0x55;
	*cfg_addr_1 = 0xA0;
	*dst_addr = src_item;
	*((vu32 *)0xBF803100) = ~src_item;
	while ( *dst_addr != src_item )
		_wait(2);
	_wait(2);
}

static void _word_write(vu16 *cfg_addr_1, vu16 *cfg_addr_2, vu16 *dst_addr, u16 src_item)
{
	*cfg_addr_1 = 0xF0;
	*cfg_addr_1 = 0xAA;
	*cfg_addr_2 = 0x55;
	*cfg_addr_1 = 0xA0;
	*dst_addr = src_item;
	*((vu32 *)0xBF803100) = ~src_item;
	while ( *dst_addr != src_item )
		_wait(2);
	_wait(2);
}

static void flash_byte_write(
	vu8 *cfg_addr_1, vu8 *cfg_addr_2, vu8 *dst_addr, const vu8 *dst_addr_end, u8 *src_addr, unsigned int block_size)
{
	vu8 *dst_addr_cur;

	for ( dst_addr_cur = dst_addr; dst_addr_cur < dst_addr_end; dst_addr_cur += 1, src_addr += 1 )
	{
		if ( !block_size )
			__builtin_trap();
		print_percentage(100 - 100 * ((u8 *)dst_addr_end - (u8 *)dst_addr_cur) / block_size, 0x14u);
		_byte_write(cfg_addr_1, cfg_addr_2, dst_addr_cur, *src_addr);
	}
}

static void flash_word_write(
	vu16 *cfg_addr_1, vu16 *cfg_addr_2, vu16 *dst_addr, const vu16 *dst_addr_end, u16 *src_addr, unsigned int block_size)
{
	vu16 *dst_addr_cur;

	for ( dst_addr_cur = dst_addr; dst_addr_cur < dst_addr_end; dst_addr_cur += 1, src_addr += 1 )
	{
		if ( !block_size )
			__builtin_trap();
		print_percentage(100 - 100 * ((u8 *)dst_addr_end - (u8 *)dst_addr_cur) / block_size, 0x14u);
		_word_write(cfg_addr_1, cfg_addr_2, dst_addr_cur, *src_addr);
	}
}

static void
flash_byte_write_fast(vu8 *cfg_addr_1, vu8 *cfg_addr_2, vu8 *dst_addr, const vu8 *dst_addr_end, u8 *src_addr)
{
	vu8 *dst_addr_cur;

	*cfg_addr_1 = 0xAA;
	*cfg_addr_2 = 0x55;
	*cfg_addr_1 = 32;
	for ( dst_addr_cur = dst_addr; dst_addr_cur < dst_addr_end; dst_addr_cur += 1, src_addr += 1 )
	{
		*dst_addr_cur = 0xA0;
		*dst_addr_cur = *src_addr;
		*((vu32 *)0xBF803100) = ~(u8)*src_addr;
		while ( *dst_addr_cur != *src_addr )
			_wait(2);
	}
	*cfg_addr_1 = 0x90;
	*cfg_addr_1 = 0xF0;
}

static void
flash_word_write_fast(vu16 *cfg_addr_1, vu16 *cfg_addr_2, vu16 *dst_addr, const vu16 *dst_addr_end, u16 *src_addr)
{
	vu16 *dst_addr_cur;

	*cfg_addr_1 = 0xAA;
	*cfg_addr_2 = 0x55;
	*cfg_addr_1 = 32;
	for ( dst_addr_cur = dst_addr; dst_addr_cur < dst_addr_end; dst_addr_cur += 1, src_addr += 1 )
	{
		*dst_addr_cur = 0xA0;
		*dst_addr_cur = *src_addr;
		*((vu32 *)0xBF803100) = ~(u16)*src_addr;
		while ( *dst_addr_cur != *src_addr )
			_wait(2);
	}
	*cfg_addr_1 = 0x90;
	*cfg_addr_1 = 0xF0;
}

void flash_write(struct rominfo_item *rii, int offset, void *src_addr, int block_size)
{
	u8 *effective_address_1;
	u16 *effective_address_2;

	CpuDisableIntr();
	switch ( rii->m_write_width )
	{
		case 8:
			effective_address_1 = (u8 *)((u8 *)rii->m_base_address + offset);
			flash_byte_write(
				&effective_address_1[rii->m_config_offset_1],
				&effective_address_1[rii->m_config_offset_2],
				effective_address_1,
				(u8 *)((u8 *)effective_address_1 + block_size),
				src_addr,
				block_size);
			break;
		case 16:
			effective_address_2 = (u16 *)((u8 *)rii->m_base_address + offset);
			flash_word_write(
				&effective_address_2[rii->m_config_offset_1],
				&effective_address_2[rii->m_config_offset_2],
				effective_address_2,
				(u16 *)((u8 *)effective_address_2 + block_size),
				src_addr,
				block_size);
			break;
		default:
			break;
	}
	CpuEnableIntr();
}

void flash_write_fast(struct rominfo_item *rii, int offset, void *src_addr, int block_size)
{
	u8 *effective_address_1;
	u16 *effective_address_2;

	CpuDisableIntr();
	switch ( rii->m_write_width )
	{
		case 8:
			effective_address_1 = (u8 *)((u8 *)rii->m_base_address + offset);
			flash_byte_write_fast(
				&effective_address_1[rii->m_config_offset_1],
				&effective_address_1[rii->m_config_offset_2],
				effective_address_1,
				(u8 *)((u8 *)effective_address_1 + block_size),
				src_addr);
			break;
		case 16:
			effective_address_2 = (u16 *)((u8 *)rii->m_base_address + offset);
			flash_word_write_fast(
				&effective_address_2[rii->m_config_offset_1],
				&effective_address_2[rii->m_config_offset_2],
				effective_address_2,
				(u16 *)((u8 *)effective_address_2 + block_size),
				src_addr);
			break;
		default:
			break;
	}
}

int flash_verify(struct rominfo_item *rii, int offset, u8 *addr, int block_size)
{
	int diff_count;
	vu8 *start_ptr;
	const u8 *end_ptr;

	diff_count = 0;
	start_ptr = (u8 *)rii->m_base_address + offset;
	end_ptr = (const u8 *)start_ptr + block_size;
	CpuDisableIntr();
	for ( ; start_ptr < end_ptr; start_ptr += 1, addr += 1 )
	{
		if ( !block_size )
			__builtin_trap();
		print_percentage(100 - 100 * (end_ptr - start_ptr) / block_size, 5u);
		if ( *start_ptr != *addr )
		{
			if ( diff_count < 8 )
			{
				CpuEnableIntr();
				printf("  0x%08x: %02x -> %02x\n", (unsigned int)(uiptr)start_ptr, *start_ptr, *addr);
				CpuDisableIntr();
			}
			diff_count += 1;
		}
	}
	CpuEnableIntr();
	return diff_count;
}

static int checksig(struct rominfo_item *rii, struct romflash_sigcheck_res *rsr, int offset)
{
	u8 *effective_address_1;
	u16 *effective_address_2;

	switch ( rii->m_write_width )
	{
		case 8:
			effective_address_1 = (u8 *)((u8 *)rii->m_base_address + offset);
			rsr->m_config_addr_1 = &effective_address_1[rii->m_config_offset_1];
			rsr->m_config_addr_2 = &effective_address_1[rii->m_config_offset_2];
			rsr->m_base = effective_address_1;
			rsr->m_base_end = &effective_address_1[rii->m_chunk_size];
			*(vu8 *)rsr->m_config_addr_1 = 0xAA;
			*(vu8 *)rsr->m_config_addr_2 = 0x55;
			*(vu8 *)rsr->m_config_addr_1 = 0x90;
			*((vu32 *)0xBF803100) = 0xFFFFFF6F;
			rsr->m_manufacturer_res = *((vu8 *)rsr->m_base);
			rsr->m_device_id_res = *((vu8 *)rsr->m_base_end);
			break;
		case 16:
			effective_address_2 = (u16 *)((u8 *)rii->m_base_address + offset);
			rsr->m_config_addr_1 = &effective_address_2[rii->m_config_offset_1];
			rsr->m_config_addr_2 = &effective_address_2[rii->m_config_offset_2];
			rsr->m_base = effective_address_2;
			rsr->m_base_end = &effective_address_2[rii->m_chunk_size];
			*(vu16 *)rsr->m_config_addr_1 = 0xAA;
			*(vu16 *)rsr->m_config_addr_2 = 0x55;
			*(vu16 *)rsr->m_config_addr_1 = 0x90;
			*((vu32 *)0xBF803100) = 0xFFFFFF6F;
			rsr->m_manufacturer_res = *((vu16 *)rsr->m_base);
			rsr->m_device_id_res = *((vu16 *)rsr->m_base_end);
			break;
		default:
			break;
	}
	flash_reset(rii);
	if ( !rii->m_manufacturer && (rsr->m_manufacturer_res == 1 || rsr->m_manufacturer_res == 4) )
		rii->m_manufacturer = rsr->m_manufacturer_res;
	return (rsr->m_manufacturer_res == rii->m_manufacturer) && (rsr->m_device_id_res == rii->m_device_id);
}

static void flash_probe_verbose(struct rominfo_item *rii, struct romflash_sigcheck_res *rsr)
{
	int curoffs_1;
	int curoffs_2;

	probe_message(" cmd0,1  = 0x%08x,0x%08x\n", rsr->m_config_addr_1, rsr->m_config_addr_2);
	probe_message(" id_addr0,1   = 0x%08x,0x%08x\n", rsr->m_base, rsr->m_base_end);
	probe_message(" [0x%08x] = ", rsr->m_base);
	switch ( rii->m_write_width )
	{
		case 8:
			*(vu8 *)rsr->m_config_addr_1 = 0xAA;
			*(vu8 *)rsr->m_config_addr_2 = 0x55;
			*(vu8 *)rsr->m_config_addr_1 = 0x90;
			for ( curoffs_1 = 0; curoffs_1 < 16; curoffs_1 += 1 )
			{
				*((vu32 *)0xBF803100) = 0xFFFFFF6F;
				probe_message("%02x%c", ((vu8 *)rsr->m_base)[curoffs_1], (curoffs_1 < 15) ? ',' : '\n');
			}
			probe_message(" manufacturer : search %02x,   return %02x\n", rii->m_manufacturer, rsr->m_manufacturer_res);
			probe_message(" device id    : search %02x,   return %02x\n", rii->m_device_id, rsr->m_device_id_res);
			break;
		case 16:
			*(vu16 *)rsr->m_config_addr_1 = 0xAA;
			*(vu16 *)rsr->m_config_addr_2 = 0x55;
			*(vu16 *)rsr->m_config_addr_1 = 0x90;
			for ( curoffs_2 = 0; curoffs_2 < 8; curoffs_2 += 1 )
			{
				*((vu32 *)0xBF803100) = 0xFFFFFF6F;
				probe_message("%04x%c", ((vu16 *)rsr->m_base)[curoffs_2], (curoffs_2 < 7) ? ',' : '\n');
			}
			probe_message(" manufacturer : search %02x,   return %02x\n", rii->m_manufacturer, rsr->m_manufacturer_res);
			probe_message(" device id    : search %04x, return %04x\n", rii->m_device_id, rsr->m_device_id_res);
			break;
		default:
			break;
	}
	flash_reset(rii);
}

static int flash_checksig(struct rominfo_item *rii)
{
	int old_address_reg;
	int old_delay_reg;
	int condtmp1;
	int flash_chunks;
	int cur_flash_chunk;
	struct romflash_sigcheck_res rsr;

	memset(&rsr, 0, sizeof(rsr));
	old_address_reg = 0;
	probe_message("\ncheck%d start  \"%s\"\n", rii->m_write_width, rii->m_device_name);
	old_delay_reg = 0;
	if ( rii->m_address_register )
	{
		old_address_reg = *((vu32 *)0xBF801400);
		*((vu32 *)0xBF801400) = rii->m_address_register;
		probe_message("\twrite dev1 address register [0x%08x] => [0x%08x]\n", old_address_reg, rii->m_address_register);
	}
	if ( rii->m_delay_register )
	{
		old_delay_reg = *((vu32 *)rii->m_delay_register);
		*((vu32 *)rii->m_delay_register) = rii->m_delay_register_value;
		*((vu32 *)0xBF803100) = *(vu32 *)rii->m_delay_register;
		probe_message(
			"\twrite %-4s delay   register [0x%08x] => [0x%08x]\n",
			((uiptr)rii->m_delay_register == 0xBF80100C) ? "dev1" : "rom",
			old_delay_reg,
			*((vu32 *)rii->m_delay_register));
	}
	flash_chunks = rii->m_flash_size / rii->m_flash_blocksize;
	for ( cur_flash_chunk = 0;
				(condtmp1 = checksig(rii, &rsr, flash_chunks * cur_flash_chunk)) && (cur_flash_chunk < rii->m_flash_blocksize);
				cur_flash_chunk += 1 )
	{
		if ( !cur_flash_chunk )
			probe_message(" offset       = ");
		probe_message("0c%08x ", flash_chunks * cur_flash_chunk);
	}
	if ( cur_flash_chunk )
		probe_message("\n");
	if ( probe_verbose )
		flash_probe_verbose(rii, &rsr);
	if ( !non_windup && !condtmp1 )
	{
		if ( rii->m_delay_register )
		{
			*((vu32 *)rii->m_delay_register) = old_delay_reg;
			*((vu32 *)0xBF803100) = *(vu32 *)rii->m_delay_register;
		}
		if ( rii->m_address_register )
			*((vu32 *)0xBF801400) = old_address_reg;
	}
	return condtmp1;
}

struct rominfo_item *flash_probe(struct rominfo_item *rii)
{
	CpuDisableIntr();
	for ( ; rii->m_write_width; rii += 1 )
	{
		if ( flash_checksig(rii) )
		{
			CpuEnableIntr();
			return rii;
		}
	}
	CpuEnableIntr();
	return 0;
}

void flash_chip_erace(struct rominfo_item *rii)
{
	vu8 *effective_address_1;
	vu8 *config_address_1_1;
	vu8 *config_address_2_1;
	vu16 *effective_address_2;
	vu16 *config_address_1_2;
	vu16 *config_address_2_2;

	CpuDisableIntr();
	switch ( rii->m_write_width )
	{
		case 8:
			effective_address_1 = (u8 *)((u8 *)rii->m_base_address);
			config_address_1_1 = &effective_address_1[rii->m_config_offset_1];
			config_address_2_1 = &effective_address_1[rii->m_config_offset_2];
			*config_address_1_1 = 0xAA;
			*config_address_2_1 = 0x55;
			*config_address_1_1 = 0x80;
			*config_address_1_1 = 0xAA;
			*config_address_2_1 = 0x55;
			*config_address_1_1 = 0x10;
			_wait(500);
			while ( (u8)*config_address_1_1 != 0xFF )
				_wait(5);
			*config_address_1_1 = 0xF0;
			break;
		case 16:
			effective_address_2 = (u16 *)((u8 *)rii->m_base_address);
			config_address_1_2 = &effective_address_2[rii->m_config_offset_1];
			config_address_2_2 = &effective_address_2[rii->m_config_offset_2];
			*config_address_1_2 = 0xAA;
			*config_address_2_2 = 0x55;
			*config_address_1_2 = 0x80;
			*config_address_1_2 = 0xAA;
			*config_address_2_2 = 0x55;
			*config_address_1_2 = 0x10;
			_wait(500);
			while ( (u16)*config_address_1_2 != 0xFFFF )
				_wait(5);
			*config_address_1_2 = 0xF0;
			break;
		default:
			break;
	}
	CpuEnableIntr();
}
