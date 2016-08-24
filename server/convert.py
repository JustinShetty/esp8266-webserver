#! python3
import os
import sys

# Set-up the directories
os.chdir('X:\\Dropbox\\Projects\\ESP8266\\server') #  <---Change this
CURDIR = os.getcwd()
OUTFILE = CURDIR + "\\website.h"
WEBSITE = "./website"

# Some predefined strings for the output file
header = '''//
// converted websites to mainly flash variables
//

#include "Flash.h"

'''
arrayPreamble = 'FLASH_ARRAY(uint8_t, file_'

structurePrototype = '''
struct t_websitefiles {
  const char* filename;
  const char* mime;
  const unsigned int len;
  const _FLASH_ARRAY<uint8_t>* content;
} files[] = {
'''

# Mime type - file extension is converted to lower before lookup
mimeDictionary = {'.jpg':'image/jpeg', '.jpeg':'image/jpeg', '.gif':'image/gif', '.css':'text/css', '.js':'application/javascript',
                  '.png':'image/png', '.htm':'text/html', '.html':'text/html', '.txt':'text/plain','.ico':'image/x-icon'}




BytesPerRow = 20  # This determines how wide the file will be
fileIndex = 0

if os.path.isdir(WEBSITE):
    outFile = open(OUTFILE,'w')
    outFile.write(header)
    os.chdir(WEBSITE)
    files = [f for f in os.listdir(os.curdir) if os.path.isfile(f)]  # All the files in the current directory
    print("Processed files :")
    for f in files:
        print(f)
        outFile.write(arrayPreamble + str(fileIndex) + ',\n  ')
        fp = open(f,'rb')
        fileLength = os.fstat(fp.fileno()).st_size
        totalWritten = 0
        bytesWrittenToRow = 0
        while totalWritten < fileLength - 1 :
            b = fp.read(1)
            outFile.write(hex(b[0]) + ',')
            bytesWrittenToRow += 1
            totalWritten +=1
            if bytesWrittenToRow >= BytesPerRow :
                outFile.write('\n  ')
                bytesWrittenToRow = 0

        b = fp.read(1)
        outFile.write(hex(b[0]) + '\n);\n\n')
        fp.close()
        fileIndex += 1

    outFile.write(structurePrototype)
    fileIndex = 0
    for f in files:
        extension = os.path.splitext(f)[1]
        extension = extension.lower()
        fileLength = os.stat(f).st_size
        fileType = mimeDictionary.get(extension,None)
        if fileType is None:
            import ctypes  # An included library with Python install.
            ctypes.windll.user32.MessageBoxA(0, b"Unknown file type", b"File Error", 0)
            outFile.write("-------------Unknown File Type <" + extension + "> Found----------------")
            outFile.close()
            os.chdir('..')  # Change back to the parent directory
            sys.exit(1)

        outFile.write('  {\n')
        outFile.write('    .filename = "' + f + '",\n')
        outFile.write('    .mime = "' + fileType + '",\n')
        outFile.write('    .len = ' + str(fileLength) + ',\n')
        outFile.write('    .content = &file_' + str(fileIndex) + '\n  },\n')
        fileIndex += 1

    outFile.write('};')
    outFile.close()
    os.chdir('..')  # Change back to the parent directory

else:
    print(WEBSITE +" Directory not found")
    sys.exit(2)
