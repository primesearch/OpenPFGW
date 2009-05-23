@echo off
: WinClean1.bat

for %%f in (*.scc) do del *.scc
for %%f in (deps*.d) do del deps.d
for %%f in (config*.status) do del config.status
for %%f in (config*.cache) do del config.cache
for %%f in (config*.log) do del config.log
for %%f in (nasmrul?.) do del nasmrule
for %%f in (makefil?.) do del makefile
for %%f in (*.stackdump) do del *.stackdump
for %%f in (*.plg) do del *.plg
for %%f in (aps*.log) do del aps*.log
for %%f in (aps*.ini) do del aps*.ini
for %%f in (*.o) do del *.o
for %%f in (*.ncb) do del *.ncb
for %%f in (*.pos) do del *.pos
for %%f in (*.aps) do del *.aps
for %%f in (*.rej) do del *.rej
for %%f in (*.a) do del *.a
for %%f in (.libs\*.a) do del .libs\*.a
for %%f in (debug\*.o*) do del debug\*.o*
for %%f in (debug\*.r*) do del debug\*.r*
for %%f in (debug\*.i*) do del debug\*.i*
for %%f in (debug\*.l*) do del debug\*.l*
for %%f in (debug\*.p*) do del debug\*.p*
for %%f in (debug\*.m*) do del debug\*.m*
for %%f in (debug\*.e*) do del debug\*.e*
for %%f in (debug\*.htm) do del debug\*.htm
for %%f in (release\*.o*) do del release\*.o*
for %%f in (release\*.r*) do del release\*.r*
for %%f in (release\*.p*) do del release\*.p*
for %%f in (release\*.i*) do del release\*.i*
for %%f in (release\*.l*) do del release\*.l*
for %%f in (release\*.m*) do del release\*.m*
for %%f in (release\*.e*) do del release\*.e*
for %%f in (release\*.htm) do del release\*.htm
if exist release\nul rd release
if exist debug\nul   rd debug
if exist .libs\nul   rd .libs

