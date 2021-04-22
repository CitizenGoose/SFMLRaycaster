# SFMLRaycaster
Raycaster C++ SFML 
 
Descargar SFML --> https://www.sfml-dev.org/files/SFML-2.5.1-windows-gcc-7.3.0-mingw-32-bit.zip

Hagan unzip (a veces hay que hacerlo 2 veces) y metansen en la carpeta de 32 o 64 bits, copien el include y lib, y peguenlo en la carpeta principal con el main.cpp
Tambien tienen que pegar elos dll que vienen en bin... esos dll ya los pegue en este repositorio, pero en caso que no sean mas validos(ie. hay unos mas nuevos y los 
viejos no funcionan), borrar los dll y pegar los nuevos que vienen en la carpeta bin en la carpeta de 64/32 bits de SFML

leer el comentario en el cpp


===============22/4/2021====================

Para los que busquen modulos para hacer un prototipo rapido: Dejo enlace a un raycaster que hace casi todo mejor(Tiene ya objetos, suelo y techo con texturas bien, etc), aunque va bastante mas lento en areas amplias con textura en suelo/techo y cae hasta los 20 FPS...

https://github.com/lxndrdagreat/sfray
