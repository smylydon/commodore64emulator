
struct T64
{
	dword	mem_start;
	dword	mem_end;
	dword	file_start;
	dword	file_end;
	dword	file_length;
	byte	*file_name;
	byte	buffer[0xff];
};

extern void c64load(byte *name);
extern void p00load(byte *name);
extern void s64load(byte *name);
extern void s64save(byte *name);
extern void t64load(byte *name);
