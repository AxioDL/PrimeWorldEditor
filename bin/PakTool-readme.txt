PakTool v1.02

PakTool is a command line tool developed by Parax made to handle Retro Studios .pak files. It can extract and repack files from any Retro game, as well as dump a list of files. It will also automatically decompress files and can even handle compression embedded within file formats!

Check out the Retro Modding Wiki for information on the structure of Retro's pak formats:
http://www.metroid2002.com/retromodding/wiki/PAK_(File_Format)

=================
Table of Contents
=================

1. Changelog
2. Games
3. Usage
3.1. Extract
3.2. Repack
3.3. List Dump
4. Note on Tropical Freeze
5. Future updates

============
1. Changelog
============

v1.02 1/24/16
- Fixed incorrect compression type for MP2 demo paks.

v1.01 5/30/15
- Fixed incorrect DKCR pak names.
- Added pak list for Trilogy.

v1.0 2/19/15
- Duplicate files are now ignored in unpack mode, significantly improving performance.
- File formats with embedded compression can now be decompressed in Extract Mode and recompressed in Repack Mode, allowing for the decompression of MREA files as well as Tropical Freeze models, textures, and shaders at unpack time.
- You can now place PakTool in a game's root directory and specify that game's title in Extract and List Dump modes to automatically unpack every one of that game's paks. (thanks DJGrenola)
- Repacking is now supported for Metroid Prime 3 and Donkey Kong Country Returns.
- Repacking now requires you to enter the game you're repacking for. This sets both the output pak format and the compression used, and replaces the old compression type parameter.
- A new "auto" mode has been added for Repack Mode; now you can just specify the pak name and PakTool will automatically set the files folder, the output pak name, and the list file. This only works if all those things use the default names, though!
- Tropical Freeze files with pak metadata now have it appended to the end of the file.

v0.86 11/11/14
- Donkey Kong Country: Tropical Freeze support added. No auto-decompression for now.

v0.85 9/09/14
- Fixed a bug with Metroid Prime 3 decompression where uncompressed data clusters weren't being copied to the output buffer correctly. This primarily affected textures, preventing the headers from being dumped correctly.
- List files are now required in repack mode; paks created with a list of every file in the provided folder are not only inoptimal, but they wouldn't have even worked ingame at all, so it didn't make sense to keep the functionality.
- Dump mode will now use the name of the input pak as the name of the file list if no name is explicitly provided.
- Donkey Kong Country Returns support added.

v0.8 7/25/14
- Added support for LZO compression for Metroid Prime 2.
- Added support for Metroid Prime 3 paks. Extraction, decompression, and file list dumping are all fully functional; however, there's no support for repacking yet.
- Compression now defaults to being off.
- Fixed crashing issue that occurred occasionally in repack mode.

v0.6 7/21/14
- File names are now dumped into the file list text files and can be reused to repack.
- A single MLVL is no longer required to repack.

v0.5 6/21/14
- Initial release

========
2. Games
========

Some modes can take a parameter specifying a certain game. Extract Mode and List Dump Mode can use this parameter to automatically dump every pak belonging to the specified game. Repack Mode needs this to choose the correct output pak format and compression algorithm. The following arguments, and some variations on them, are accepted:

MP1 - Metroid Prime
MP2 - Metroid Prime 2: Echoes
MP3 - Metroid Prime 3: Corruption
DKCR - Donkey Kong Country Returns
DKCTF - Donkey Kong Country: Tropical Freeze
MP1Demo - Metroid Prime Kiosk Demo
MP2Demo - Metroid Prime 2 Demo Disc
MP3Proto - Metroid Prime 3 E3 Prototype
Trilogy - Metroid Prime: Trilogy

========
3. Usage
========

Usage: PakTool [mode] [options]

The tool has three main functions.

------------
3.1. Extract
------------

PakTool will read an input pak and extract all files contained within it to a folder. Any compressed files will be automatically decompressed. This can either be used with a single pak (in which case the -x can be omitted, and unpack mode will be assumed), or a game can be specified with the Extract Game mode, -xg, which will automatically unpack every pak belonging to that game. For Extract Game to work, PakTool should be placed in the game's root directory.

Usage: PakTool -x [input pak]
  Alt: PakTool -xg [game]

Optional: -o [output directory] -e

  -o [output directory]: Specify the output directory. By default, PakTool will create a new directory in the same directory as the input pak called "[name]-pak". With the -o option, you can specify a different output directory. If this option is used with a single pak, then the files will be extracted directly to the chosen output directory. Otherwise, a separate subdirectory will be created for each pak; the game's original directory structure will be preserved within the specified output directory.

  -e: Some files contain compressed data as part of their actual format, rather than being part of the pak format. If -e is specifed, PakTool will decompress any files that contain embedded compression. This means MREA files in Metroid Prime 2/Metroid Prime 3/DKCR, and CMDL/SMDL/WMDL/TXTR/MTRL files in Tropical Freeze.

-----------
3.2. Repack
-----------

PakTool will repack the files in a directory into a new custom pak that can be loaded ingame. The "game" parameter will set both the output pak format, as well as the type of compression used in the pak. Files are repacked in the order specified by the list file. You can use List Dump Mode to generate a list file first, modify it if necessary, then use it to repack.

You can also use the Repack Auto mode, using -ra, to automatically set the input folder, the output pak, and the list file all at the same time. For example, if you run "PakTool -ra MP1 Metroid1", then PakTool will look for a folder called "Metroid1-pak/", a list file named "Metroid1-pak.txt", and will save to "Metroid1.pak". This will only work properly if everything is extracted with the default names.

Usage: PakTool -r [GAME] [INPUT FOLDER]	[OUTPUT.pak] [LIST.txt]
  Alt: PakTool -ra [GAME] [FOLDER/PAK/LIST NAME]

Optional: -e / -n

  -e: This is the reverse of the -e setting from Extract Mode; it will recompress files with embedded compression as part of their format, if they were previously decompressed. Since Tropical Freeze isn't supported in repack mode, this only affects MREA files.

  -n: Disable compression entirely. Your paks will be much larger with this flag set, but it may be more convenient if you want to hex edit the pak directly. Paks without any compressed files are still compatible with the game.

Keep in mind that -e and -n can't be used simultaneously.

--------------
3.3. List Dump
--------------

PakTool will simply dump a list of files and filenames contained within the pak. This list can then be reused in Repack Mode to create new paks. This can either be used with a single pak, or a game can be specified with the Dump Game mode, -dg, which will automatically dump a list for every pak belonging to that game. For Dump Game to work, PakTool should be placed in the game's root directory.

Usage: PakTool -d [input pak]
  Alt: PakTool -dg [game]

Optional: -o [output]

  -o [output directory]: Specify the output directory. By default, PakTool will create a text file in the same directory as the input pak. With the -o option, you can specify a different output directory. If this parameter is specified with Dump Game mode, then the game's original directory structure will be preserved within the specified output directory.

==========================
4. Note on Tropical Freeze
==========================

Tropical Freeze's pak format allows the pak to hold extra data for certain files in a section labeled "META". The main area where this comes into play is compression. Compression in Tropical Freeze is handled much differently than with the previous pak formats. A few formats are compressed:

- Models (CMDL, SMDL, WMDL), as well as textures (TXTR), have a "GPU " section which is compressed with a custom LZSS implementation.
- Shaders (MTRL) are compressed with zlib.

However, the metadata required to decompress those files - the compressed and decompressed sizes, the number and sizes of compressed buffers, etc. - are located in the pak, in the META section, rather than the file itself. This means if the files are unpacked directly and left as-is, then they're completely useless, because they come without some extremely important metadata.

To remedy this, any file that contains metadata will have an extra META section appended to the end of the file data. This extra META section uses the standard Tropical Freeze section header format, followed by a dump of that file's metadata. This extra data will not be added to the size value in that file's RFRM header, which makes it easy to locate and seek to.

PakTool also has the ability to directly decompress these files at unpack time. For textures and shaders, this is simple; compressed data is simply replaced with the decompressed equivalent. The RFRM and section header sizes are both updated with the new sizes. The extra META section will not be appended to the end of the file in this case.

However, on models, this introduces another problem: the GPU section contains multiple compressed buffers, and the pak metadata is the only way to distinguish between them! This means some modification of the format is necessary; otherwise the file data is still useless.

Model GPU sections contain a number of vertex buffers and index buffers. When PakTool decompresses these, it will insert a u32 indicating the number of vertex buffers; following that, each vertex buffer will be written to the file, each preceded with a 32-bit size value (in bytes). Then the same process is repeated for index buffers.

This process isn't completely ideal, but it does enable all the files to be fully decompressed and completely usable on their own, without any external metadata. Just keep in mind that any modifications to the format will not work ingame; the data needs to be recompressed and the extra data needs to be removed before the files can be properly repacked.

If you'd like to work with the original files directly, simply don't specify the -e parameter, and these files will be extracted with their compression intact and their metadata appended to the end of the file, like with other formats.

You can find information on how the Tropical Freeze LZSS implementation works on the Retro Modding Wiki.
http://www.metroid2002.com/retromodding/wiki/LZSS_Compression

=================
5. Future updates
=================

The following features are being considered for future releases:

- Parsing files for dependencies. This would allow for repack mode file lists to be built automatically. Unfortunately, this is a fairly complicated process and requires a thorough understanding of every format used in each game.
- Repacking for Tropical Freeze. This is unlikely to happen until there's a public means of modding Wii U games. Handling embedded compression would also be difficult since it would require a custom LZSS compressor.