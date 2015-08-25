input = open('glimports', 'r')
output = open('glimports.cpp','w')
output.write("#pragma warning(push)\n#pragma warning(disable:4191)\n")
ProcLoadedContents = '';
import sys
stringtype = ''
if len(sys.argv) > 2 and not sys.argv[2].isspace():
    stringtype = '('+sys.argv[2]+')'
for line in input:
    if line.isspace():
        pass
    else:
        output.write('static PFN'+line.strip().upper()+'PROC '+line.strip()+' = 0;\n')
        ProcLoadedContents = ProcLoadedContents+ line.strip()+'= (PFN'+line.strip().upper()+'PROC)'+sys.argv[1]+'('+stringtype+'"'+line.strip()+'");\n'

output.write('\ninternal void LoadGLProcs()\n{\n'+ ProcLoadedContents + '\n}\n')
output.write("#pragma warning(pop)\n")
