for %%i in (Core\Inc\*.h) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Core\Src\*.c) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\CLI\Config\*.h) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\CLI\*.h) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\CLI\*.c) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\Console\Config\*.h) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\Console\UART\*.h) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\Console\UART\*.c) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\Tests\*.h) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\Tests\*.c) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\*.h) do uncrustify -c uncrustify.cfg --no-backup %%i
for %%i in (Application\*.c) do uncrustify -c uncrustify.cfg --no-backup %%i