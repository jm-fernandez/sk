
<p align="center" width="100%">
    <img src="https://raw.githubusercontent.com/jm-fernandez/sk/refs/heads/main/assets/intro/sk_logo.bmp"> 
</p>


Monika esta en medio de unas vacaciones de ensueño en Hawai, disfrutando de cócteles en la playa y sintiendo el cálido sol en su piel, cuando de repente una voz la interrumpe. ¡Qué decepción! Era solo un sueño. Esa voz pertenecía a su madre, apurándola para que no llegara tarde al insti. En realidad, Monika se encuentra en su habitación. Gijón, Junio de 1995. Es el último día de clase, y solo le queda recoger sus notas antes de empezar el tan esperado verano. Sin embargo, al llegar al insti, su mundo se desmorona: Las ha suspendido todas. Pero Monika no va a resignarse y pasar el verano encerrada estudiando. Monika se propone convencer a cada uno de sus profesores de que merece ser aprobada. Y los va a convencer por las buenas... o por las malas!

## Como jugar
El programa ha sido probado en DOSBox-X, usando la configuracion por defecto, y llevando a cabo los siguientes ajustes:
   - Ajustar el tipo de CPU a "486 with prefetch"
   - Ajustar la velocidad de emulación (Opcion "Emulated CPU Speed) a "486DX2 66MHZ"
En caso de que se detected problemas con la reproducción MIDI, se recomienda usar el dispositivo Midi fluidsynth.

La version 0.3 añade el soporte para PCem v17. El juego necesita una tarjeta de memoria SVGA con al menos 1Mb de RAM, un driver compatible con VESA 2.0 instalado, y con un modo 640x480x256 con frame buffer lineal disponible. Se pueden consultar las notas del release para mas detalles. Puedes encontrar una maquina virtual para PCem v17 lista para poder jugar [aquí](https://github.com/jm-fernandez/PCEM-486-SB16-TGUI9400)

Las teclas para jugar son las siguientes
   - Cursores para el movimiento 
   - A: Defensa
   - S: Patada Alta
   - D: Punetazo alto
   - X: Patada baja
   - C: Punetazo bajo
   - P: Pausa
   - Esc: Pasar la intro o abandonar programa (Sin aviso previo!)


## Como compilar
El codigo fuente se compila uando compilacion cruzada con OpenWatcom. Se proporciona un fichero makefile para la utilidad wmake proporcionada justo con Openwatcom. Para generar el fichero ejecutable solo es necesario ejecutar el siguiente comando:

```
wmake
``` 

Se generará un fichero sk.exe. Este fichero, juto con la carpeta assets es lo unico que es necesario para poder ejecutar el juego. El fichero makefile tambien proporciona una opcion para eliminar los ficheros generadores durante la compilacion:

```
wmake clean
``` 

En caso de no querer compilar el juego, se puede descargar el juego junto con todos los ficheros necearios en formato zip en la lista de releases. La ultima release es https://github.com/jm-fernandez/sk/releases/tag/0.3

## Creditos
El codigo para la reproduccion de ficheros MIDI esta basada en el codigo fuente de DOSMid, por Mateusz Viste

El codigo para la reproduccion de ficheros WAV esta basado en el codigo fuente de auplay, por John Tsiombikas

El fichero MIDI que se reproduce durante el juego es de dominio publico

Algunos de los contenidos del juego, como algunas de las imagenes y dibujos han sido generados usando inteligencia artificial

El resto del contenido constituye una obra original

## Un juego para MS-DOS en 2025?
School Kombat es un juego para MS-DOS, desarrollado para el concurso C:\DOS\CONTEST organizado por el [msdos.club](https://msdos.club/)

<p align="center" width="100%">
    <img src="https://raw.githubusercontent.com/jm-fernandez/sk/refs/heads/main/assets/intro/logoc.bmp"> 
</p>
