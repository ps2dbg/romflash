
#include "romflash_internal.h"

// clang-format off
static const char *usage_str =
	"  Usage:\n"
	"   romflash [option] [datafile]\n"
	"     option: ( n:dec, x:hex )\n"
	"      -f        : fast mode ( no verify, short message )\n"
	"      -v        : verbose message\n"
	"      -s        : silence ( except error message )\n"
	"      -probe    : probe information only\n"
	"\n"
	"      search type\n"
	"      -flash    : T10000/EB* flash ( default search )\n"
	"      -gmain    : Gmain1-5 flash\n"
	"      -dev1     : 2Mbyte dev1 flash ( addr:0x1fa00000, size:0x00200000 )\n"
	"      -dev1e0   : 4Mbyte dev1 flash ( addr:0x1e000000, size:0x00400000 )\n"
	"                             set base address to dev1 address register\n"
	"      -if3flash : interface3 board flash\n"
	"      -if3dev1  : interface3 board dev1 flash (4MB * n)\n"
	"      -if3_12m  : interface3 board dev1 flash (12MB)\n"
	"      -if3_4m   : interface3 board dev1 flash (4MB)\n"
	"      search type ( mpu4 )\n"
	"      -mpu4boot     : mpu4 boot flash search ( caution! )\n"
	"      -mpu4shadow   : mpu4 shadow flash search\n"
	"      -mpu4dev1     : mpu4 dev1 except b64k flash search\n"
	"      -mpu4dev1b64k : mpu4 dev1 b64k flash search\n"
	"\n"
	"      search type list\n"
	"      -list     : print search type list\n"
	"      -listall  : print search type list all\n"
	"\n"
	"      write option\n"
	"      -retry n  : retry\n"
	"      -verify n : verify\n"
	"      -block x  : read block size\n"
	"      -offset x : write offset\n"
	"\n"
	"      write memory image\n"
	"      -mem      : write memory image ( addr:0x00080000, size:0x00100000 )\n"
	"             -addr x  : memory image address\n"
	"             -size x  : memory image size\n"
	"\n";
static struct rominfo_item default_list[] =
{
	{
		"T10000, boot flash, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		2097152,
		0,
		196,
		8,
		0,
		0,
		0
	},
	{
		"T10000, boot flash, 2M flash (MBM29LV160B/AM29LV160B)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		2097152,
		0,
		73,
		8,
		0,
		0,
		0
	},
	{
		"Gmain1-3, flash mode, 512K flash (MBM29F040C)",
		(void *)0xBF801010,
		1256783,
		(void *)0xBFC00000,
		1,
		1365,
		682,
		524288,
		4,
		164,
		8,
		0,
		0,
		0
	},
	{
		"Gmain4, rom mode, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFE00000,
		2,
		2730,
		1365,
		2097152,
		0,
		196,
		8,
		0,
		0,
		0
	},
	{
		"Gmain1-3, rom mode, 512K flash (MBM29F040C)",
		(void *)0xBF80100C,
		1256783,
		(void *)0xBFA00000,
		1,
		1365,
		682,
		524288,
		4,
		164,
		8,
		0,
		0,
		0
	},
	{
		"G4 special, flash mode, 4M flash (MBM29DL324BD)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		4194304,
		4,
		95,
		8,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item gmain_list[] =
{
	{
		"Gmain1-3, flash mode, 512K flash (MBM29F040C)",
		(void *)0xBF801010,
		1256783,
		(void *)0xBFC00000,
		1,
		1365,
		682,
		524288,
		4,
		164,
		8,
		0,
		0,
		0
	},
	{
		"Gmain5..., flash mode, 2M flash*2 (MBM29LV160T/AM29LV160T)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		4194304,
		0,
		196,
		8,
		0,
		0,
		2
	},
	{
		"Gmain5..., flash mode, 2M flash*2 (MBM29LV160B/AM29LV160B)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		4194304,
		0,
		73,
		8,
		0,
		0,
		2
	},
	{
		"Gmain5..., rom mode, 2M flash*2 (MBM29LV160T/AM29LV160T)",
		(void *)0xBF80100C,
		1453391,
		(void *)0xBE000000,
		2,
		2730,
		1365,
		4194304,
		0,
		196,
		8,
		503316480,
		0,
		2
	},
	{
		"Gmain5..., rom mode, 2M flash*2 (MBM29LV160B/AM29LV160B)",
		(void *)0xBF80100C,
		1453391,
		(void *)0xBE000000,
		2,
		2730,
		1365,
		4194304,
		0,
		73,
		8,
		503316480,
		0,
		2
	},
	{
		"Gmain4, flash mode, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF801010,
		1387855,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		2097152,
		0,
		196,
		8,
		0,
		0,
		0
	},
	{
		"Gmain4, rom mode, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFE00000,
		2,
		2730,
		1365,
		2097152,
		0,
		196,
		8,
		0,
		0,
		0
	},
	{
		"Gmain1-3, rom mode, 512K flash (MBM29F040C)",
		(void *)0xBF80100C,
		1256783,
		(void *)0xBFA00000,
		1,
		1365,
		682,
		524288,
		4,
		164,
		8,
		0,
		0,
		0
	},
	{
		"G4 special, flash mode, 4M flash (MBM29DL324BD)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		4194304,
		4,
		95,
		8,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item dev1_list[] =
{
	{
		"Gmain4, dev1, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF80100C,
		1392463,
		(void *)0xBFA00000,
		1,
		1365,
		682,
		2097152,
		0,
		8900,
		16,
		0,
		0,
		0
	},
	{
		"TrialBoard, dev1, only 2M access 4M flash (MBM29DL324BD)",
		(void *)0xBF80100C,
		1392463,
		(void *)0xBFA00000,
		1,
		1365,
		682,
		2097152,
		4,
		8799,
		16,
		0,
		2097152,
		0
	},
	{
		"G4 special, dev1, only 2M access 4M flash (MBM29DL324BD)",
		(void *)0xBF80100C,
		1387855,
		(void *)0xBFA00000,
		2,
		2730,
		1365,
		2097152,
		4,
		95,
		8,
		0,
		2097152,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item dev1e0_list[] =
{
	{
		"TrialBoard, dev1 on 1e00, 4M flash (MBM29DL324BD)",
		(void *)0xBF80100C,
		1457999,
		(void *)0xBE000000,
		1,
		1365,
		682,
		4194304,
		4,
		8799,
		16,
		503316480,
		0,
		0
	},
	{
		"G4 special, dev1 on 1e00, 4M flash (MBM29DL324BD)",
		(void *)0xBF80100C,
		1453391,
		(void *)0xBE000000,
		2,
		2730,
		1365,
		4194304,
		4,
		95,
		8,
		503316480,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item if3flash_list[] =
{
	{
		"interface3, flash mode, 4M flash (MBM29DL324TD)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		4194304,
		4,
		92,
		8,
		0,
		0,
		0
	},
	{
		"interface3, flash mode, 4M flash (MBM29DL324BD)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		4194304,
		4,
		95,
		8,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item if3flash16_list[] =
{
	{
		"interface3, flash mode, 4M flash (MBM29DL324TD)",
		(void *)0xBF801010,
		1457999,
		(void *)0xBFC00000,
		1,
		1365,
		682,
		4194304,
		4,
		8796,
		16,
		0,
		0,
		0
	},
	{
		"interface3, flash mode, 4M flash (MBM29DL324BD)",
		(void *)0xBF801010,
		1457999,
		(void *)0xBFC00000,
		1,
		1365,
		682,
		4194304,
		4,
		8799,
		16,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item if3dev1_list[] =
{
	{
		"interface3, dev1 on 1e00, 4M flash *? (MBM29DL324TD)",
		(void *)0xBF80100C,
		1589071,
		(void *)0xBE000000,
		1,
		1365,
		682,
		16777216,
		4,
		8796,
		16,
		503316480,
		0,
		0
	},
	{
		"interface3, dev1 on 1e00, 4M flash *? (MBM29DL324BD)",
		(void *)0xBF80100C,
		1589071,
		(void *)0xBE000000,
		1,
		1365,
		682,
		16777216,
		4,
		8799,
		16,
		503316480,
		0,
		0
	},
	{
		"interface3, dev1 on 1e00, 2M flash *? (MBM29LV160B/AM29LV160B)",
		(void *)0xBF80100C,
		1589071,
		(void *)0xBE000000,
		1,
		1365,
		682,
		16777216,
		0,
		8777,
		16,
		503316480,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item if3dev1_12m_list[] =
{
	{
		"interface3, dev1 on 1e00, 4M flash *3 (MBM29DL324TD)",
		(void *)0xBF80100C,
		1589071,
		(void *)0xBE000000,
		1,
		1365,
		682,
		12582912,
		4,
		8796,
		16,
		503316480,
		0,
		3
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item if3dev1_4m_list[] =
{
	{
		"interface3, dev1 on 1e00, 4M flash (MBM29DL324TD)",
		(void *)0xBF80100C,
		1589071,
		(void *)0xBE000000,
		1,
		1365,
		682,
		4194304,
		4,
		8796,
		16,
		503316480,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item njig16_4m_list[] =
{
	{
		"njig 16bit, dev9 on 1000, 4M flash (MBM29DL324TD)",
		(void *)0xBF801418,
		1720319,
		(void *)0xB0000000,
		1,
		1365,
		682,
		4194304,
		4,
		8796,
		16,
		0,
		0,
		0
	},
	{
		"njig 16bit, dev9 on 1000, 2M flash*2 (MBM29LV160B/AM29LV160B)",
		(void *)0xBF801418,
		1720319,
		(void *)0xB0000000,
		1,
		1365,
		682,
		4194304,
		0,
		8777,
		16,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item njig8_4m_list[] =
{
	{
		"njig 8bit, dev9 on 1000, 4M flash (MBM29DL324TD)",
		(void *)0xBF801418,
		1716223,
		(void *)0xB0000000,
		2,
		2730,
		1365,
		4194304,
		4,
		92,
		8,
		0,
		0,
		0
	},
	{
		"njig 8bit, dev9 on 1000, 2M flash*2 (MBM29LV160B/AM29LV160B)",
		(void *)0xBF801418,
		1716223,
		(void *)0xB0000000,
		2,
		2730,
		1365,
		4194304,
		0,
		73,
		8,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item mpu4boot_list[] =
{
	{
		"mpu4, boot flash, 512K flash (AM29LV040B)",
		(void *)0xBF801010,
		1256783,
		(void *)0xBFC00000,
		1,
		1365,
		682,
		524288,
		1,
		79,
		8,
		0,
		0,
		0
	},
	{
		"mpu4, boot flash, 512K flash (AM29F040B)",
		(void *)0xBF801010,
		1256783,
		(void *)0xBFC00000,
		1,
		1365,
		682,
		524288,
		1,
		164,
		8,
		0,
		0,
		0
	},
	{
		"mpu4, boot flash, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		2097152,
		0,
		196,
		8,
		0,
		0,
		0
	},
	{
		"mpu4, boot flash, 2M flash (MBM29LV160B/AM29LV160B)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFC00000,
		2,
		2730,
		1365,
		2097152,
		0,
		73,
		8,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item mpu4shadow_list[] =
{
	{
		"mpu4, shadow flash, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFE00000,
		2,
		2730,
		1365,
		2097152,
		0,
		196,
		8,
		0,
		0,
		0
	},
	{
		"mpu4, shadow flash, 2M flash (MBM29LV160B/AM29LV160B)",
		(void *)0xBF801010,
		1453391,
		(void *)0xBFE00000,
		2,
		2730,
		1365,
		2097152,
		0,
		73,
		8,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item mpu4dev1_list[] =
{
	{
		"mpu4, dev1 flash, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF80100C,
		1392463,
		(void *)0xBFA00000,
		1,
		1365,
		682,
		2031616,
		0,
		8900,
		16,
		0,
		0,
		0
	},
	{
		"mpu4, dev1 flash, 2M flash (MBM29LV160B/AM29LV160B)",
		(void *)0xBF80100C,
		1392463,
		(void *)0xBFA00000,
		1,
		1365,
		682,
		2031616,
		0,
		8777,
		16,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct rominfo_item mpu4b64k_list[] =
{
	{
		"mpu4, dev1 b64k, 2M flash (MBM29LV160T/AM29LV160T)",
		(void *)0xBF80100C,
		1392463,
		(void *)0xBFBF0000,
		1,
		1365,
		682,
		65536,
		0,
		8900,
		16,
		0,
		2031616,
		0
	},
	{
		"mpu4, dev1 b64k, 2M flash (MBM29LV160B/AM29LV160B)",
		(void *)0xBF80100C,
		1392463,
		(void *)0xBFBF0000,
		1,
		1365,
		682,
		65536,
		0,
		8777,
		16,
		0,
		2031616,
		0
	},
	{ NULL, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static struct table_item table[] =
{
	{ "-flash", &default_list[0] },
	{ "-gmain", &gmain_list[0] },
	{ "-dev1", &dev1_list[0] },
	{ "-dev1e0", &dev1e0_list[0] },
	{ "-mpu4boot", &mpu4boot_list[0] },
	{ "-mpu4shadow", &mpu4shadow_list[0] },
	{ "-mpu4dev1", &mpu4dev1_list[0] },
	{ "-mpu4dev1b64k", &mpu4b64k_list[0] },
	{ "-if3flash", &if3flash_list[0] },
	{ "-if3flash16", &if3flash16_list[0] },
	{ "-if3dev1", &if3dev1_list[0] },
	{ "-if3_12m", &if3dev1_12m_list[0] },
	{ "-if3_4m", &if3dev1_4m_list[0] },
	{ "-njig8", &njig8_4m_list[0] },
	{ "-njig16", &njig16_4m_list[0] }
};
int probe_verbose = 0;
int non_windup = 0;
int percentage = 0;
// clang-format on

typedef struct my_stdio_prnt_context_
{
	s16 fd;
	s16 current_length;
	char tmp_buf[64];
} my_stdio_prnt_context_t;

static void my_stdio_prnt_callback(void *userdata, int c)
{
	size_t current_length;
	char *tmp_buf;
	my_stdio_prnt_context_t *context = (my_stdio_prnt_context_t *)userdata;

	current_length = context->current_length;
	tmp_buf = context->tmp_buf;

	switch ( c )
	{
		case 512:
			context->current_length = 0;
			return;
		case 513:
			if ( current_length == 0 )
			{
				return;
			}
			write(context->fd, tmp_buf, current_length);
			return;
		case 10:
			my_stdio_prnt_callback(context, '\r');
			break;
	}
	tmp_buf[current_length] = c;
	current_length += 1;
	context->current_length = current_length;
	if ( current_length == 64 )
	{
		context->current_length = 0;
		write(context->fd, tmp_buf, current_length);
	}
}

int vprintf(const char *format, va_list va)
{
	my_stdio_prnt_context_t context;

	context.fd = 1;
	context.current_length = 0;
	return prnt(&my_stdio_prnt_callback, &context, format, va);
}

static void message(const struct romflash_info *rfi, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	if ( !((rfi->m_option_flags >> 3) & 1) )
		vprintf(fmt, va);
	va_end(va);
}

static void v_message(const struct romflash_info *rfi, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	if ( !!((rfi->m_option_flags >> 2) & 1) )
		vprintf(fmt, va);
	va_end(va);
}

static int fileopen(const char *filename, int fileflags)
{
	int fd;
	char pathbuf[256];

	strcpy(pathbuf, "host1:");
	strcat(pathbuf, filename);
	fd = open(pathbuf, fileflags);
	if ( fd < 0 )
	{
		strcpy(pathbuf, "sim:");
		strcat(pathbuf, filename);
		return open(pathbuf, fileflags);
	}
	return fd;
}

static int filesize(int fd)
{
	int lseek_res;

	lseek_res = lseek(fd, 0, 2);
	lseek(fd, 0, 0);
	return lseek_res;
}

static int FileOpenAndCheck(struct rominfo_item *fii, struct romflash_info *rfw, const char *inputfile)
{
	int fd;
	int filesize_res;

	fd = fileopen(inputfile, 1);
	if ( fd < 0 )
	{
		printf("\t`%s' can't open !!!\n", inputfile);
		return 1;
	}
	rfw->m_fd = fd;
	filesize_res = filesize(fd);
	if ( filesize_res > 0 )
	{
		if ( !rfw->m_option_size )
			rfw->m_option_size = filesize_res;
		if ( (unsigned int)fii->m_flash_size >= (unsigned int)rfw->m_option_size )
		{
			rfw->m_option_addr = AllocSysMemory(0, rfw->m_option_block, 0);
			if ( rfw->m_option_addr )
			{
				message(rfw, " inputfile: %s  size: %x(%dbytes)\n", inputfile, filesize_res, filesize_res);
				return 0;
			}
			printf("\ttoo large block size !!!\n");
		}
		else
		{
			printf("\tout of filesize !!!\n");
			printf("\tflash size [0x%08x] < file size [0x%08x]\n", fii->m_flash_size, filesize_res);
		}
	}
	else
	{
		printf("\tfile empty !!!\n");
	}
	close(fd);
	return 1;
}

static int ReadData(struct romflash_info *rfw, int offset, unsigned int size)
{
	int lseek_res;

	*((vu8 *)0xBF802070) = 2;
	lseek_res = lseek(rfw->m_fd, offset, 0);
	if ( lseek_res != offset )
		return 1;
	message(rfw, "  Reading [offset:0x%x] ...", lseek_res);
	read(rfw->m_fd, rfw->m_option_addr, size);
	message(rfw, " 0x%x(%dbytes)\n", size, size);
	return 0;
}

static int FlashWriteAndVerify(struct rominfo_item *fii, struct romflash_info *rfw, int offset, int block_size)
{
	int flash_verify_result;

	*((vu8 *)0xBF802070) = 4;
	v_message(
		rfw,
		"  Writing flash memory ( source address:0x%08x-0x%08x )\n      ",
		rfw->m_option_addr,
		&((u8 *)rfw->m_option_addr)[block_size - 1]);
	message(
		rfw,
		"  Writing [0x%08x-0x%08x] ",
		(char *)fii->m_base_address + offset,
		(char *)fii->m_base_address + offset + block_size - 1);
	flash_verify_result = 0;
	if ( (rfw->m_option_flags & 0x22) == 34 )
		flash_write_fast(fii, offset, rfw->m_option_addr, block_size);
	else
		flash_write(fii, offset, rfw->m_option_addr, block_size);
	if ( rfw->m_option_verify )
	{
		int flash_verify_trycnt;

		*((vu8 *)0xBF802070) = 8;
		message(rfw, "\n  Verify flash memory (x%d) ", rfw->m_option_verify);
		for ( flash_verify_trycnt = 0; !flash_verify_result && (flash_verify_trycnt < rfw->m_option_verify);
					flash_verify_trycnt += 1 )
		{
			flash_verify_result |= flash_verify(fii, offset, (u8 *)rfw->m_option_addr, block_size);
			message(rfw, " ");
		}
		if ( !flash_verify_result )
			message(rfw, "\n  Wrote flash memory\n");
	}
	message(rfw, "\n");
	return flash_verify_result;
}

static void FlashSectorErace(struct rominfo_item *rii, const struct romflash_info *rfw, struct flash_rom_work *frw)
{
	int update_sector_cur;

	if ( !((rfw->m_option_flags >> 3) & 1) )
	{
		int i;
		int j;

		if ( frw->m_update_sector_lower == frw->m_update_sector_upper )
			printf("  Sector Clear (%d) ", frw->m_update_sector_upper);
		else
			printf("  Sector Clear (%d-%d) ", frw->m_update_sector_lower, frw->m_update_sector_upper);
		for ( i = 0; i < frw->m_update_sector_upper - frw->m_update_sector_lower + 1; i += 1 )
			printf(".");
		for ( j = 0; j < frw->m_update_sector_upper - frw->m_update_sector_lower + 1; j += 1 )
			printf("\b");
	}
	for ( update_sector_cur = frw->m_update_sector_lower; update_sector_cur <= frw->m_update_sector_upper;
				update_sector_cur += 1 )
	{
		message(rfw, "o");
		flash_sector_erace_addr(rii, frw->m_type_list_element->m_sector_range[update_sector_cur].m_offset);
	}
	message(rfw, "\n");
}

static void
UpdateSectorWrite(struct rominfo_item *rii, struct romflash_info *rfw, struct flash_rom_work *frw, int offset_aboslute)
{
	int flash_verify_result;
	int offs;
	int update_sector_cur;
	int flash_verify_trycnt;

	flash_verify_result = 0;
	offs = 0;
	if ( !((rfw->m_option_flags >> 3) & 1) )
	{
		int i;
		int j;

		if ( frw->m_update_sector_lower == frw->m_update_sector_upper )
			printf("  Update Sector (%d) ", frw->m_update_sector_upper);
		else
			printf("  Update Sector (%d-%d) ", frw->m_update_sector_lower, frw->m_update_sector_upper);
		for ( i = 0; i < frw->m_update_sector_upper - frw->m_update_sector_lower + 1; i += 1 )
			printf(".");
		for ( j = 0; j < frw->m_update_sector_upper - frw->m_update_sector_lower + 1; j += 1 )
			printf("\b");
	}
	percentage = 0;
	for ( update_sector_cur = frw->m_update_sector_lower; update_sector_cur <= frw->m_update_sector_upper;
				update_sector_cur += 1 )
	{
		char *addr_effective;
		struct type_list_inner_item *range_param;
		int range_length;

		addr_effective = (char *)rfw->m_option_addr + offs;
		range_param = &frw->m_type_list_element->m_sector_range[update_sector_cur];
		range_length = range_param->m_length;
		if ( !memcmp((char *)rii->m_base_address + range_param->m_offset, addr_effective, range_length) )
		{
			message(rfw, "=");
			offs += range_length;
		}
		else
		{
			message(rfw, "o");
			flash_sector_erace_addr(rii, range_param->m_offset);
			flash_write_fast(rii, offset_aboslute + offs, addr_effective, range_length);
			for ( flash_verify_trycnt = 1; !flash_verify_result && (rfw->m_option_verify >= flash_verify_trycnt);
						flash_verify_trycnt += 1 )
			{
				flash_verify_result |= flash_verify(rii, offset_aboslute + offs, (u8 *)addr_effective, range_length);
				message(rfw, "\b%x", flash_verify_trycnt);
			}
			offs += range_length;
		}
	}
	message(rfw, "\n");
}

static struct type_list_item *SectorWriteCheck(const struct rominfo_item *rii, struct romflash_info *rfw)
{
	struct type_list_item *result;

	result = flash_type_search(rii);
	if ( !result )
	{
		message(rfw, " unknown flash type!!!\n");
		return 0;
	}
	if ( !!(rfw->m_option_offset & (result->m_erase_size - 1)) )
	{
		message(rfw, "\terror !!! write offset [0x%08x]\n", rfw->m_option_offset);
		return 0;
	}
	if ( !!(rfw->m_option_block & (result->m_erase_size - 1)) && !rfw->m_option_block )
	{
		message(rfw, "\terror !!! block size [0x%08x]\n", 0);
		return 0;
	}
	return result;
}

static int WriteWithRetry(struct rominfo_item *rii, struct romflash_info *rfw, struct type_list_item *tli)
{
	int cur_size;
	int retry_count;
	struct flash_rom_work frw;

	retry_count = 0;
	for ( cur_size = rfw->m_option_size; cur_size; )
	{
		int cur_block;
		int offset;

		cur_block = (rfw->m_option_block < cur_size) ? rfw->m_option_block : cur_size;
		offset = rfw->m_option_size - cur_size;
		flash_reset(rii);
		if ( !((rfw->m_option_flags >> 1) & 1) && !offset && !((rfw->m_option_flags >> 4) & 1) )
		{
			message(rfw, "  Clear flash memory\n");
			flash_chip_erace(rii);
		}
		if ( (rfw->m_option_flags & 0x12) == 2 )
		{
			if ( flash_sector_search(tli, rfw->m_option_offset + offset, cur_block, &frw) )
			{
				printf("sector search error!!!\n");
				return cur_size;
			}
			FlashSectorErace(rii, rfw, &frw);
		}
		if ( !(rfw->m_option_flags & 1) && ReadData(rfw, offset, cur_block) )
		{
			printf(" file read error!!!\n");
			return cur_size;
		}
		if ( FlashWriteAndVerify(rii, rfw, rfw->m_option_offset + offset, cur_block) )
		{
			retry_count += 1;
			printf("Verify error !\n");
			if ( rfw->m_option_retry < retry_count )
			{
				printf(" abort !!!\n");
				return cur_size;
			}
			printf(" retry %d\n", retry_count);
			cur_size = rfw->m_option_size;
		}
		else
		{
			cur_size -= cur_block;
		}
	}
	return cur_size;
}

static int UpdateWithRetry(struct rominfo_item *rii, struct romflash_info *rfw, struct type_list_item *tli)
{
	int cur_size;
	int cur_block;
	struct flash_rom_work frw;

	for ( cur_size = rfw->m_option_size; cur_size; cur_size -= cur_block )
	{
		int offset;

		cur_block = (rfw->m_option_block < cur_size) ? rfw->m_option_block : cur_size;
		offset = rfw->m_option_size - cur_size;
		flash_reset(rii);
		if ( !(rfw->m_option_flags & 1) && ReadData(rfw, offset, cur_block) )
		{
			printf(" file read error!!!\n");
			return cur_size;
		}
		if ( (rfw->m_option_flags & 0x12) == 2 )
		{
			if ( flash_sector_search(tli, rfw->m_option_offset + offset, cur_block, &frw) )
			{
				printf("sector search error!!!\n");
				return cur_size;
			}
			UpdateSectorWrite(rii, rfw, &frw, rfw->m_option_offset + offset);
		}
	}
	return cur_size;
}

static int FlashWriteWithRetry(struct rominfo_item *rii, struct romflash_info *rfw)
{
	struct type_list_item *tli;
	unsigned int write_result;

	tli = 0;
	if ( rii->m_flash_size < rfw->m_option_offset + rfw->m_option_size )
	{
		printf(" write offset, size error!!!\n\n");
		*((vu8 *)0xBF802070) = 0xFF;
		return 1;
	}
	if ( rii->m_flash_size < rfw->m_option_size )
	{
		v_message(rfw, "\twritesize change [0x%08x] => [0x%08x]\n", rfw->m_option_size, rii->m_flash_size);
		rfw->m_option_size = rii->m_flash_size;
	}
	if ( !!(rfw->m_option_flags & 1) && rfw->m_option_block != rfw->m_option_size )
	{
		v_message(rfw, "\tblocksize change [0x%08x] => [0x%08x]\n", rfw->m_option_block, rfw->m_option_size);
		rfw->m_option_block = rfw->m_option_size;
	}
	if ( !!((rfw->m_option_flags >> 1) & 1) && (tli = SectorWriteCheck(rii, rfw), !tli) )
		return 1;
	message(
		rfw,
		" %s: 0x%08x-0x%08x",
		(!!(rfw->m_option_flags & 1)) ? "memory image" : "data area",
		rfw->m_option_addr,
		(char *)rfw->m_option_addr + rfw->m_option_block - 1);
	message(rfw, "  blocksize: %x(%dbytes)\n\n", rfw->m_option_block, rfw->m_option_block);
	*((vu8 *)0xBF802070) = 1;
	write_result = ((rfw->m_option_flags & 0x62) == 98) ? UpdateWithRetry(rii, rfw, tli) : WriteWithRetry(rii, rfw, tli);
	if ( write_result )
	{
		printf("Error !!! ( remain %x )\n\n", write_result);
		*((vu8 *)0xBF802070) = 0xFF;
		return 1;
	}
	message(rfw, "Complete !\n\n");
	*((vu8 *)0xBF802070) = 0;
	return 0;
}

static int usage(int flag)
{
	if ( !flag )
		return 0;
	printf("\n%s %s\n", "romflash", "$Revision: 1.33 $");
	printf(usage_str);
	return 1;
}

static int ishex(int chr)
{
	return ((unsigned int)(chr - 48) < 0xA || (unsigned int)(chr - 65) < 6 || (unsigned int)(chr - 97) < 6) ? 1 : 0;
}

static int xtoi(const char *str)
{
	const char *strpos;
	int retrestmp;

	strpos = str;
	retrestmp = 0;
	if ( *str == '0' && str[1] == 'x' )
		strpos = str + 2;
	for ( ; *strpos && ishex(*strpos); strpos += 1 )
	{
		int curchr;
		int chrbase;
		int chrasint;

		curchr = *(u8 *)strpos;
		retrestmp *= 16;
		if ( curchr - (unsigned int)'0' < 0xA )
		{
			chrbase = retrestmp - '0';
			chrasint = (char)curchr;
			retrestmp = chrbase + chrasint;
		}
		else if ( curchr - (unsigned int)'A' < 6 || curchr - (unsigned int)'a' < 6 )
		{
			chrbase = retrestmp + 9;
			chrasint = curchr & 7;
			retrestmp = chrbase + chrasint;
		}
	}
	return retrestmp;
}

static void printdesc(struct rominfo_item *rii)
{
	printf("\t\"%s\"\n", rii->m_device_name);
	printf("\t0x%08x,\t0x%08x\n", (unsigned int)(uiptr)rii->m_delay_register, rii->m_delay_register_value);
	printf(
		"\t0x%08x,\t0x%04x,\t0x%04x,\t0x%04x,\n",
		(unsigned int)(uiptr)rii->m_base_address,
		rii->m_chunk_size,
		rii->m_config_offset_1,
		rii->m_config_offset_2);
	printf(
		"\t0x%08x,\t0x%02x,\t0x%02x,\t%d\n", rii->m_flash_size, rii->m_manufacturer, rii->m_device_id, rii->m_write_width);
	if ( rii->m_address_register )
		printf("\t0x%08x\n", rii->m_address_register);
	printf("\n");
}

static void printlist(struct table_item *ti)
{
	struct rominfo_item *cur_list;

	printf("[%s]\n", ti->m_table);
	for ( cur_list = ti->m_list; cur_list->m_device_name; cur_list += 1 )
	{
		printdesc(cur_list);
	}
}

int _start(int argc, char **argv)
{
	int probe_flag;
	int list_flag;
	const char *flash_file_filename;
	struct rominfo_item *cur_list;
	int i;
	struct table_item *ti0;
	struct table_item *ti1;
	struct rominfo_item *rii;
	struct romflash_info rfi;

	probe_flag = 0;
	list_flag = 0;
	flash_file_filename = 0;
	cur_list = default_list;
	rfi.m_option_block = 0x80000;
	rfi.m_option_addr = 0;
	rfi.m_option_size = 0;
	rfi.m_option_offset = 0;
	rfi.m_option_retry = 1;
	rfi.m_option_verify = 1;
	rfi.m_option_flags = 0;
	rfi.m_option_flags &= ~0xff;
	rfi.m_option_flags |= 2;
	for ( i = 1; i < argc && argv[i][0] == '-'; i += 1 )
	{
		for ( ti0 = table; ti0 < &table[sizeof(table) / sizeof(table[0])]; ti0 += 1 )
		{
			if ( !strcmp(ti0->m_table, argv[i]) )
			{
				cur_list = ti0->m_list;
				break;
			}
		}
		if ( ti0 >= &table[sizeof(table) / sizeof(table[0])] )
		{
			if ( !strcmp("-probe", argv[i]) )
			{
				probe_flag += 1;
			}
			else if ( !strcmp("-v", argv[i]) || !strcmp("-verbose", argv[i]) )
			{
				rfi.m_option_flags ^= 4;
			}
			else if ( !strcmp("-s", argv[i]) || !strcmp("-silence", argv[i]) )
			{
				rfi.m_option_flags ^= 8;
			}
			else if ( !strcmp("-ne", argv[i]) || !strcmp("-noerace", argv[i]) )
			{
				rfi.m_option_flags ^= 0x10;
			}
			else if ( !strcmp("-eo", argv[i]) || !strcmp("-eraceonly", argv[i]) )
			{
				rfi.m_option_flags ^= 0x80;
			}
			else if ( !strcmp("-nw", argv[i]) || !strcmp("-nowindup", argv[i]) )
			{
				non_windup = !non_windup;
			}
			else if ( !strcmp("-ce", argv[i]) || !strcmp("-chiperace", argv[i]) )
			{
				rfi.m_option_flags &= ~2u;
			}
			else if ( !strcmp("-sec", argv[i]) || !strcmp("-sector", argv[i]) )
			{
				rfi.m_option_flags ^= 2;
			}
			else if ( !strcmp("-f", argv[i]) || !strcmp("-fast", argv[i]) )
			{
				rfi.m_option_flags ^= 0x20;
			}
			else if ( !strcmp("-up", argv[i]) || !strcmp("-update", argv[i]) )
			{
				rfi.m_option_verify = 0;
				rfi.m_option_flags ^= 0x40;
			}
			else if ( !strcmp("-retry", argv[i]) )
			{
				i += 1;
				if ( i >= argc )
					return usage(1);
				rfi.m_option_retry = strtol(argv[i], 0, 10);
			}
			else if ( !strcmp("-verify", argv[i]) )
			{
				i += 1;
				if ( i >= argc )
					return usage(1);
				rfi.m_option_verify = strtol(argv[i], 0, 10);
			}
			else if ( !strcmp("-block", argv[i]) )
			{
				i += 1;
				if ( i >= argc )
					return usage(1);
				rfi.m_option_block = xtoi(argv[i]);
			}
			else if ( !strcmp("-mem", argv[i]) || !strcmp("-", argv[i]) )
			{
				rfi.m_option_flags ^= 1;
			}
			else if ( !strcmp("-addr", argv[i]) )
			{
				i += 1;
				if ( i >= argc )
					return usage(1);
				rfi.m_option_addr = (void *)xtoi(argv[i]);
			}
			else if ( !strcmp("-size", argv[i]) )
			{
				i += 1;
				if ( i >= argc )
					return usage(1);
				rfi.m_option_size = xtoi(argv[i]);
			}
			else if ( !strcmp("-offset", argv[i]) )
			{
				i += 1;
				if ( i >= argc )
					return usage(1);
				rfi.m_option_offset = xtoi(argv[i]);
			}
			else if ( !strcmp("-listall", argv[i]) )
			{
				for ( ti1 = table; ti1 < &table[sizeof(table) / sizeof(table[0])]; ti1 += 1 )
					printlist(ti1);
				return 1;
			}
			else if ( !strcmp("-list", argv[i]) )
			{
				list_flag = 1;
				if ( !ti0 )
					return usage(1);
			}
			else
			{
				return usage(1);
			}
		}
	}
	if ( list_flag )
	{
		struct table_item *ti2;

		for ( ti2 = table; ti2 < &table[sizeof(table) / sizeof(table[0])]; ti2 += 1 )
		{
			if ( ti2->m_list == cur_list )
				printlist(ti2);
		}
		return 1;
	}
	if ( !!(rfi.m_option_flags & 1) )
	{
		if ( probe_flag )
			rfi.m_option_flags &= ~8u;
	}
	else if ( i < argc )
	{
		flash_file_filename = argv[i];
		if ( probe_flag )
			rfi.m_option_flags &= ~8u;
	}
	else
	{
		if ( probe_flag )
			rfi.m_option_flags &= ~8u;
		if ( !((rfi.m_option_flags >> 7) & 1) )
			return usage(1);
	}
	if ( !!((rfi.m_option_flags >> 3) & 1) )
		rfi.m_option_flags &= ~4u;
	if ( probe_flag && !!((rfi.m_option_flags >> 2) & 1) )
		probe_verbose = 1;
	*((vu8 *)0xBF802070) = 0xFF;
	rii = flash_probe(cur_list);
	if ( !rii )
	{
		*((vu8 *)0xBF802070) = 0xFD;
		printf("\n%s %s\n", "romflash", "$Revision: 1.33 $");
		printf("\n\t!!! flash ram not found !!!\n\n");
		if ( !!((rfi.m_option_flags >> 2) & 1) )
		{
			printf("dev1 address  = 0x%08x\n", (unsigned int)*((vu32 *)0xBF801400));
			printf("dev1 delayreg = 0x%08x\n", (unsigned int)*((vu32 *)0xBF80100C));
			printf("rom delayreg  = 0x%08x\n\n", (unsigned int)*((vu32 *)0xBF801010));
		}
		return 1;
	}
	if ( !((rfi.m_option_flags >> 3) & 1) )
	{
		printf(
			"\n%s %s  %s mode (pos=%x,sig=%x) on %s\n\n",
			"romflash",
			"$Revision: 1.33 $",
			(((uiptr)rii->m_base_address) != 0xBFC00000) ? "safe" : "quick",
			(unsigned int)((((uiptr)rii->m_base_address) >> 20) & 0xF),
			rii->m_device_id,
			"native kernel");
		percentage = 1;
	}
	if ( !!((rfi.m_option_flags >> 2) & 1) || probe_flag )
	{
		struct type_list_item *tli;

		printf("\"%s\"\n", rii->m_device_name);
		printf("manufacturer  = %02x\n", rii->m_manufacturer);
		printf("device id     = %02x\n", rii->m_device_id);
		printf("base address  = 0x%08x\n", (unsigned int)(uiptr)rii->m_base_address);
		printf("flash size    = 0x%08x\n", rii->m_flash_size);
		tli = flash_type_search(rii);
		if ( tli )
			printf("erace size    = 0x%08x\n", tli->m_erase_size);
		else
			printf("unknown flash type !!\n");
		printf("dev1 address  = 0x%08x\n", (unsigned int)*((vu32 *)0xBF801400));
		printf("dev1 delayreg = 0x%08x\n", (unsigned int)*((vu32 *)0xBF80100C));
		printf("rom delayreg  = 0x%08x\n", (unsigned int)*((vu32 *)0xBF801010));
		printf("\n");
		if ( non_windup )
			printf(" no windup\n");
	}
	if ( (!((rfi.m_option_flags >> 2) & 1) && !probe_flag) || !probe_flag )
	{
		if ( !!((rfi.m_option_flags >> 5) & 1) && (rii->m_flash_size <= 0x1FFFFF || rii->m_manufacturer != 4) )
		{
			printf("!non support bypass mode\n");
			rfi.m_option_flags &= ~0x20u;
		}
		if ( !!((rfi.m_option_flags >> 7) & 1) )
		{
			printf("Clear flash memory\n");
			flash_chip_erace(rii);
			return 0;
		}
		else if ( !!(rfi.m_option_flags & 1) )
		{
			if ( !rfi.m_option_addr )
				rfi.m_option_addr = (void *)0x80000;
			if ( !rfi.m_option_size )
				rfi.m_option_size = (rii->m_flash_size < 0x100000) ? rii->m_flash_size : 0x100000;
			return FlashWriteWithRetry(rii, &rfi);
		}
		else if ( flash_file_filename && !FileOpenAndCheck(rii, &rfi, flash_file_filename) )
		{
			int retres;

			retres = FlashWriteWithRetry(rii, &rfi);
			close(rfi.m_fd);
			return retres;
		}
		return 1;
	}
	return 0;
}
