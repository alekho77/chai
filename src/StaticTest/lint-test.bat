set "cpplint=C:\Google\cpplint.py"
for /D %%p in (%1*) do (
	echo - Are testing %%~np
	pushd "%%p"
	for %%f in (*.cpp *.h) do (
		rem echo %%f
		python %cpplint% --linelength=150 --filter=-build/include_what_you_use %%f
		rem echo "%%f ERR: %errorlevel%"
	)
	popd
)
