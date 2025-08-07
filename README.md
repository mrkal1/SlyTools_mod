## Tools

### Unpacker
TODO: Write a description

### Packer 
TODO: Write a description

### Compressor
TODO: Write a description

### Compressor
Compresses files to the LZSS-derived format that the official game expects for Sly level files.

## Instructions
### Extracting assets
1) Download the tools from the Releases panel on the right (or build them)
2) Download the [psarc](https://mega.nz/file/u5c1zRCb#-D3vhHr5PdPTYhGKTYXhedgDkmvmxx75l9AayYe_Ksw) tool ([psarc wiki](https://www.psdevwiki.com/ps3/PlayStation_archive_(PSARC)#PSARC))
3) Go to your installed game's USRDIR directory (example: `C:/rpcs3/dev_hdd0/game/NPUA80663/USRDIR`).
   We'll now refer to this as `$USRDIR`, replace it where appropriate.
4) Copy `psarc.exe` there (or use the full path to it in the next command)
5) `psarc extract --input=Sly1.psarc --to=.`

TODO: Finish this section

## Useful knowledge
1) Replace Sly1.psarc with almost empty Sly1.psarc (as an example with Finnish dub mod, there's kok.txt only inside the Sly1.psarc)
2) Game proceeds to skip the file and makes your life easier as you don't need to repack psarc everytime (this is slow process)
3) So now you can place SLY1.WAC and SLY1.WAL into Sly1 folder (inside USRDIR\Sly1)

## Special Thanks
[The Original SlyTools](https://github.com/VelocityRa/SlyTools)
