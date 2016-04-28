@cl /nologo /Zi /GR- /EHs /MDd /c %1.cc -I ../../ESAT_rev109/include

@cl /nologo /Zi /GR- /EHs /MDd /Fe:%1.exe %1.obj ..\..\ESAT_rev109\bin\ESAT_d.lib ws2_32.lib opengl32.lib user32.lib gdi32.lib shell32.lib