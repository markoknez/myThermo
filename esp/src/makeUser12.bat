make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=6 || goto :error
make COMPILE=gcc BOOT=new APP=2 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=6 || goto :error
curl http://mrostudios.duckdns.org:23231/start_upgrade
goto :EOF

:error
echo Error #%errorlevel%
exit /b %errorlevel%