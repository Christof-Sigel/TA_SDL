input = open('glimports', 'r')
output = open('glimports.cpp','w')
output.write("#pragma warning(push)\n#pragma warning(disable:4191)\n")
ProcLoadedContents = '';
for line in input:
    if line.isspace():
        pass
    else:
        output.write('PFN'+line.strip().upper()+'PROC '+line.strip()+' = 0;\n')
        ProcLoadedContents = ProcLoadedContents+ line.strip()+'= (PFN'+line.strip().upper()+'PROC)wglGetProcAddress("'+line.strip()+'");\n'

output.write('\nvoid LoadGLProcs()\n{\n'+ ProcLoadedContents + '\n}\n')
output.write("#pragma warning(pop)\n")
