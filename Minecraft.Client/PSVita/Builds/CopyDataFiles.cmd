
:: Copies all files for PS Vita Minecraft

:: PARAM
:: %1 - Target Dir
:: %2 - Territory

::Set back level
pushd .
cd ..\..\

::Remove Directories
if exist %1\%2 rmdir /S /Q %1\%2

:: Create directories
if not exist %1\%2 mkdir %1\%2

::Copying app files [images etc.]
xcopy /I /Y /S "PSVita\app" "%1\%2\app"

::Correct Param.sfo depending on territory
copy "PSVita\app\Region\%2\param.sfo" "%1\%2\app\sce_sys\param.sfo"

::Copy correct livearea xml depending on territory
::Trial
copy "PSVita\app\Region\%2\template-trial.xml" "%1\%2\app\sce_sys\livearea\contents\template.xml"
::Full
copy "PSVita\app\Region\%2\template.xml" "%1\%2\app\sce_sys\retail\livearea\contents\template.xml"

::Remove Region folder
rmdir /S /Q %1\%2\app\Region

::Copying Game Files
::Common - Media
if not exist %1\%2\app\Common\Media mkdir %1\%2\app\Common\Media
xcopy /I /Y /S Common\Media\font %1\%2\app\Common\Media\font
xcopy /I /Y Common\Media\MediaPSVita.arc %1\%2\app\Common\Media\
xcopy /I /Y /S Common\res %1\%2\app\Common\res
::Music
xcopy /I /Y /S music %1\%2\app\music
::Sounds
xcopy /I /Y /S PSVita\Sound %1\%2\app\PSVita\Sound
::Product codes
copy ..\PsVitaProductCodes\%2\PSVitaProductCodes.bin %1\%2\app\PSVita\PSVitaProductCodes.bin
::Invite image
copy PSVita\session_image.png %1\%2\app\PSVita\session_image.png
::DLC
xcopy /I /Y /S PSVita\DLC %1\%2\app\PSVita\DLC
::Trophy file
mkdir %1\%2\app\sce_sys\trophy\NPWR06859_00\
copy  PSVita\GameConfig\Minecraft_signed.trp %1\%2\app\sce_sys\trophy\NPWR06859_00\TROPHY.TRP

:: Manual
if exist PSVita\app\Region\%2\manual xcopy /I /Y /S PSVita\app\Region\%2\manual %1\%2\app\sce_sys\manual


::Removing files
rmdir /S /Q %1\%2\app\Common\res\TitleUpdate\GameRules\BuildOnly
rmdir /S /Q %1\%2\app\Common\res\achievement
rmdir /S /Q %1\%2\app\Common\res\1_2_2\achievement

::Remove Microsoft fonts
del /F /S /Q %1\%2\app\Common\Media\font\KOR\BOKMSD.ttf
del /F /S /Q %1\%2\app\Common\Media\font\RU\SpaceMace.ttf
del /F /S /Q %1\%2\app\Common\Media\font\JPN\DFGMaruGothic-Md.ttf
del /F /S /Q %1\%2\app\Common\Media\font\CHT\DFHeiMedium-B5.ttf

:: Creating EBOOT.bin
"%SCE_PSP2_SDK_DIR%\host_tools\build\bin\psp2bin" -i ..\PSVita_ContentPackage\Minecraft.Client.self --strip-all -o %1\%2\app\eboot.bin
copy ..\PSVita_ContentPackage\Minecraft.Client.self %1\%2\%2.self

::Move the gp4p file
move /Y %1\%2\app\Minecraft.Client_%2.gp4p %1\%2\Minecraft.Client_%2.gp4p

::End dir
popd

echo Copied Files for %1 Build
