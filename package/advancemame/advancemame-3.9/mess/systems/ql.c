#include "driver.h"
#include "vidhrdw/generic.h"
#include "inputx.h"

/*

	Sinclair QL

	MESS Driver by Curt Coder


	TODO:

	- everything

*/

/* vidhrdw */

static int mode4_colors[] = { 0, 2, 4, 7 };

PALETTE_INIT( ql )
{
	palette_set_color( 0x00, 0x00, 0x00, 0x00 );
	palette_set_color( 0x01, 0x00, 0x00, 0xff );
	palette_set_color( 0x02, 0x00, 0xff, 0x00 );
	palette_set_color( 0x03, 0x00, 0xff, 0xff );
	palette_set_color( 0x04, 0xff, 0x00, 0x00 );
	palette_set_color( 0x05, 0xff, 0x00, 0xff );
	palette_set_color( 0x06, 0xff, 0xff, 0x00 );
	palette_set_color( 0x07, 0xff, 0xff, 0xff );
}

WRITE8_HANDLER( ql_videoram_w )
{
	int i, x, y, r, g, color, byte0, byte1;
	int offs;

	videoram[offset] = data;

	offs = offset / 2;

	byte0 = videoram[offs];
	byte1 = videoram[offs + 1];

	x = (offs % 64) << 3;
	y = offs / 64;

//	logerror("ofs %u data %u x %u y %u\n", offset, data, x, y);

		/*

		# Note: QL video is encoded as 2-byte chunks
		# msb->lsb (Green Red)
		# mode 4: GGGGGGGG RRRRRRRR
		# R+G=White

		*/

		for (i = 0; i < 8; i++)
		{
			r = (byte1 & 0x80) >> 6;
			g = (byte0 & 0x80) >> 7;

			color = r | g;
			
//			logerror("x %u y %u color %u\n", x, y, color);

			plot_pixel(tmpbitmap, x++, y, Machine->pens[mode4_colors[color]]);

			byte0 <<= 1;
			byte1 <<= 1;

		}

		/*

		# Note: QL video is encoded as 2-byte chunks
		# msb->lsb (Green Flash Red Blue)
		# mode 8: GFGFGFGF RBRBRBRB

		*/

/*
		for (i = 0; i < 8; i++)
		{
			r = (data & 0x0080) >> 5;
			g = (data & 0x8000) >> 14;
			b = (data & 0x0040) >> 6;
			f = (data & 0x4000) >> 14;

			color = r | g | b;

			plot_pixel(tmpbitmap, x++, y, Machine->pens[color]);
			plot_pixel(tmpbitmap, x++, y, Machine->pens[color]);

			data <<= 2;
		}
*/
}

/* Read/Write Handlers */

WRITE8_HANDLER( video_ctrl_w )
{
	/*
	18063 is write only (Quasar p.618)
	bit 1: 0: screen on, 1: screen off
	bit 3: 0: mode 512, 1: mode 256
	bit 7: 0: base=0x20000, 1: base=0x28000
	*/
}

static int ZXmode, ZXbps;

WRITE8_HANDLER( zx8302_w )
{
	ZXmode = data & 0x18;

	switch(ZXmode)
	{
	case 0x00: logerror("ZXSER1\n"); break;
	case 0x08: logerror("ZXSER2\n"); break;
	case 0x10: logerror("ZXMDV\n"); break;
	case 0x18: logerror("ZXNET\n"); break;
	}

	switch(data & 0x07)
	{
	case 0: ZXbps = 19200; break;
	case 1: ZXbps = 9600; break;
	case 2: ZXbps = 4800; break;
	case 3: ZXbps = 2400; break;
	case 4: ZXbps = 1200; break;
	case 5: ZXbps = 600; break;
	case 6: ZXbps = 300; break;
	case 7: ZXbps = 75; break;
	}
}

WRITE8_HANDLER( i8049_w )
{
	/*
	process 8049 commands (p92)
	WR 18003: 8049  commands
	0:
	1: get interrupt status
	2: open ser1
	3: open ser2
	4: close ser1/ser2
	5:
	6: serial1 receive
	7: serial2 receive
	8: read keyboard
	9: keyrow
	a: set sound
	b: kill sound
	c:
	d: set serial baudrate
	e: get response
	f: test

	write nibble to 8049:
	'dbca' in 4 writes to 18003: 000011d0, 000011c0, 000011b0, 000011a0.
	each write is acknowledged by 8049 with a '0' in bit 6 of 18020.

	response from 8049:
	repeat for any number of bits to be received (MSB first)
	970718
	{ write 00001101 to 18003; wait for '0' in bit6 18020; read bit7 of 18020 }

	*/
}

WRITE8_HANDLER( mdv_ctrl_w )
{
	/*
	MDV control
	at reset; and after data transfer of any mdv
		(02 00) 8 times			stop all motors
	start an mdvN:
		(03 01) (02 00) N-1 times	start motorN
	format (erase):
		0a
		(0e 0e) 0a per header and per sector
	format (verify):
		02
		(02 02) after finding pulses per header/sector
	format (write directory):
		0a (0e 0e) 02 write sector
	format (find sector 0):
		(02 02) after pulses
	format (write directory):
		0a (0e 0e) 02 write sector
		(02 00) 8 times 		stop motor

	read sector:
		(02 02) after pulses
		get 4 bytes
		(02 02) indication to skip PLL sequence: 6*00,2*ff
	*/
}

WRITE8_HANDLER( mdv_data_w )
{
}

READ8_HANDLER( mdv_status_r )
{
	return 0x0;
}

WRITE8_HANDLER( clock_w )
{
}

/* Memory Maps */

static ADDRESS_MAP_START( ql_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000000, 0x00bfff) AM_ROM	// System ROM
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM	// Cartridge ROM
	AM_RANGE(0x018000, 0x018001) AM_WRITE(clock_w)
	AM_RANGE(0x018002, 0x018002) AM_WRITE(zx8302_w)
	AM_RANGE(0x018003, 0x018003) AM_WRITE(i8049_w)
	AM_RANGE(0x018020, 0x018020) AM_READWRITE(mdv_status_r, mdv_ctrl_w)
	AM_RANGE(0x018021, 0x018021) AM_WRITENOP // ???
	AM_RANGE(0x018022, 0x018022) AM_WRITE(mdv_data_w)
	AM_RANGE(0x018023, 0x018023) AM_WRITENOP // ???
	AM_RANGE(0x018063, 0x018063) AM_WRITE(video_ctrl_w)
	AM_RANGE(0x020000, 0x027fff) AM_RAM AM_WRITE(ql_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x028000, 0x02ffff) AM_RAM // videoram 2
	AM_RANGE(0x030000, 0x03ffff) AM_RAM // onboard RAM
	AM_RANGE(0x040000, 0x0bffff) AM_RAM // 512KB add-on RAM
	AM_RANGE(0x0c0000, 0x0dffff) AM_NOP // 8x16KB device slots
	AM_RANGE(0x0e0000, 0x0fffff) AM_ROM	// add-on ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ipc_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

/* Input Ports */

INPUT_PORTS_START( ql )
	PORT_START_TAG("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("F1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("F2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_NAME("F3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_NAME("F4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_NAME("F5")

	PORT_START_TAG("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')

	PORT_START_TAG("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') PORT_NAME("TABULATE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')

	PORT_START_TAG("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START_TAG("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("CAPS LOCK")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')

	PORT_START_TAG("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('?') PORT_CHAR('~')

	PORT_START_TAG("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START_TAG("ROW7") // where is UP?
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("\x1b") 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_NAME("ESC ?") 
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("\x1a") 
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_NAME("SPACE") 
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("\x19") 
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("ENTER")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START_TAG("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("SHIFT")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_NAME("CTRL")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_NAME("ALT")

	PORT_START_TAG("JOY0")

	PORT_START_TAG("JOY1")

INPUT_PORTS_END

/* Machine Drivers */

static MACHINE_DRIVER_START( ql )
	// basic machine hardware
	MDRV_CPU_ADD(M68008, 15000000/2)	// 7.5 MHz
	MDRV_CPU_PROGRAM_MAP(ql_map, 0)

	MDRV_CPU_ADD(I8039, 11000000)		// 11 MHz (i8049 IPC)
	MDRV_CPU_PROGRAM_MAP(ipc_map, 0)

	MDRV_FRAMES_PER_SECOND(50)
	MDRV_VBLANK_DURATION(DEFAULT_REAL_60HZ_VBLANK_DURATION)

    // video hardware
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(8)

	MDRV_PALETTE_INIT(ql)
	MDRV_VIDEO_START(generic_bitmapped)
	MDRV_VIDEO_UPDATE(generic_bitmapped)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(BEEP, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( ql_ntsc )
	MDRV_IMPORT_FROM(ql)
	MDRV_FRAMES_PER_SECOND(60)
MACHINE_DRIVER_END

/* ROMs */

ROM_START( ql )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "ah.ic33.1", 0x000000, 0x004000, CRC(a9b4d2df) SHA1(142d6f01a9621aff5e0ad678bd3cbf5cde0db801) )
    ROM_LOAD16_WORD_SWAP( "ah.ic33.2", 0x004000, 0x004000, CRC(36488e4e) SHA1(ff6f597b30ea03ce480a3d6728fd1d858da34d6a) )
    ROM_LOAD16_WORD_SWAP( "ah.ic34",   0x008000, 0x004000, CRC(61259d4c) SHA1(bdd10d111e7ba488551a27c8d3b2743917ff1307) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_jm )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "jm.ic33", 0x000000, 0x008000, CRC(1f8e840a) SHA1(7929e716dfe88318bbe99e34f47d039957fe3cc0) )
    ROM_LOAD16_WORD_SWAP( "jm.ic34", 0x008000, 0x004000, CRC(9168a2e9) SHA1(1e7c47a59fc40bd96dfefc2f4d86827c15f0199e) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_tb )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "tb.ic33", 0x000000, 0x008000, CRC(1c86d688) SHA1(7df8028e6671afc4ebd5f65bf6c2d6019181f239) )
    ROM_LOAD16_WORD_SWAP( "tb.ic34", 0x008000, 0x004000, CRC(de7f9669) SHA1(9d6bc0b794541a4cec2203256ae92c7e68d1011d) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_js )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "js.ic33", 0x000000, 0x008000, CRC(1bbad3b8) SHA1(59fd4372771a630967ee102760f4652904d7d5fa) )
    ROM_LOAD16_WORD_SWAP( "js.ic34", 0x008000, 0x004000, CRC(c970800e) SHA1(b8c9203026a7de6a44bd0942ec9343e8b222cb41) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_jsu )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "jsu.ic33", 0x000000, 0x008000, CRC(e397f49f) SHA1(c06f92eabaf3e6dd298c51cb7f7535d8ef0ef9c5) )
    ROM_LOAD16_WORD_SWAP( "jsu.ic34", 0x008000, 0x004000, CRC(3debbacc) SHA1(9fbc3e42ec463fa42f9c535d63780ff53a9313ec) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_mge )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "mge.ic33", 0x000000, 0x008000, CRC(d5293bde) SHA1(bf5af7e53a472d4e9871f182210787d601db0634) )
    ROM_LOAD16_WORD_SWAP( "mge.ic34", 0x008000, 0x004000, CRC(a694f8d7) SHA1(bd2868656008de85d7c191598588017ae8aa3339) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_mgf )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "mgf.ic33", 0x000000, 0x008000, NO_DUMP )
    ROM_LOAD16_WORD_SWAP( "mgf.ic34", 0x008000, 0x004000, NO_DUMP )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_mgg )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "mgg.ic33", 0x000000, 0x008000, CRC(b4e468fd) SHA1(cd02a3cd79af90d48b65077d0571efc2f12f146e) )
    ROM_LOAD16_WORD_SWAP( "mgg.ic34", 0x008000, 0x004000, CRC(54959d40) SHA1(ffc0be9649f26019d7be82925c18dc699259877f) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_mgi )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "mgi.ic33", 0x000000, 0x008000, CRC(d5293bde) SHA1(bf5af7e53a472d4e9871f182210787d601db0634) )
    ROM_LOAD16_WORD_SWAP( "mgi.ic34", 0x008000, 0x004000, CRC(a2fdfb83) SHA1(162b1052737500f3c13497cdf0f813ba006bdae9) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_mgs )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "mgs.ic33", 0x000000, 0x008000, NO_DUMP )
    ROM_LOAD16_WORD_SWAP( "mgs.ic34", 0x008000, 0x004000, NO_DUMP )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

ROM_START( ql_efp )
    ROM_REGION( 0x400000, REGION_CPU1, 0 )
    ROM_LOAD16_WORD_SWAP( "efp.ic33", 0x000000, 0x008000, NO_DUMP )
    ROM_LOAD16_WORD_SWAP( "efp.ic34", 0x008000, 0x004000, NO_DUMP )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x0000, 0x0800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )
ROM_END

/* System Configuration */

SYSTEM_CONFIG_START( ql )
	CONFIG_RAM_DEFAULT	(128 * 1024)
	CONFIG_RAM			(640 * 1024)
SYSTEM_CONFIG_END

/* Computer Drivers */

/*     YEAR  NAME      PARENT	COMPAT	MACHINE   INPUT     INIT  CONFIG	COMPANY	FULLNAME */
//COMP( 1984, ql_fb,  0,  0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.00, FB)", GAME_NOT_WORKING )
//COMP( 1984, ql_pm,  0,  0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.01, PM)", GAME_NOT_WORKING )
COMP( 1984, ql,     0,  0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.02, AH)", GAME_NOT_WORKING )
COMP( 1984, ql_jm,  ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.03, JM)", GAME_NOT_WORKING )
COMP( 1984, ql_tb,  ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.0?, TB)", GAME_NOT_WORKING )
COMP( 1985, ql_js,  ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.10, JS)", GAME_NOT_WORKING )
COMP( 1985, ql_jsu, ql, 0, ql_ntsc, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.10, JSU) (NTSC)", GAME_NOT_WORKING )
COMP( 1985, ql_mge, ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.13, MGE) (Spain)", GAME_NOT_WORKING )
COMP( 1985, ql_mgf, ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.13, MGF) (France)", GAME_NOT_WORKING )
COMP( 1985, ql_mgg, ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.13, MGG) (Germany)", GAME_NOT_WORKING )
COMP( 1985, ql_mgi, ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.13, MGI) (Italy)", GAME_NOT_WORKING )
COMP( 1985, ql_mgs, ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.13, MGS) (Sweden)", GAME_NOT_WORKING )
COMP( 1985, ql_efp, ql, 0, ql, ql, 0, ql, "Sinclair Research Ltd", "QL (v1.13, EFP) (Mexico)", GAME_NOT_WORKING )
