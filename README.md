# sk
School Kombat: A MS-DOS videogame for the msdos.club game context

![alt text](https://raw.githubusercontent.com/jm-fernandez/sk/refs/heads/main/assets/intro/sk_logo.bmp)

## Como compilar
El codigo fuente se compila uando compilacion cruzada con OpenWatcom. Se proporciona un fichero makefile para la utilidad wmake proporcionada justo con Openwatcom. Para generar el fichero ejecutable solo es necesario ejecutar el siguiente comando:

```
wmake
``` 

Se generará un fichero sk.exe. Este fichero, juto con la carpeta assets es lo unico que es necesario para poder ejecutar el juego. El fichero makefile tambien proporciona una opcion para eliminar los ficheros generadores durante la compilacion:

```
wmake clean
``` 

## Como jugar
El programa ha sido testeado en DOSBox-X, usando la configuracion por defecto, y llevando a cabo los siguientes ajustes:
   - Ajustar el tipo de CPU a "486 with prefetch"
   - Ajustar la velocidad de emulación (Opcion "Emulated CPU Speed) a "486DX2 66MHZ"

En caso de que se detected problemas con la reproducción MIDI, se recomienda usar el dispositivo Midi fluidsynth.

Las teclas para jugar son las siguientes
   - Cursores para el movimiento 
   - A: Defensa
   - S: Patada Alta
   - D: Punetazo alto
   - X: Patada baja
   - C: Punetazo bajo
   - P: Pausa
   - Esc: Pasar la intro o abandonar programa (Sin aviso previo!)

## Creditos
El codigo para la reproduccion de ficheros MIDI esta basada en el codigo fuente de DOSMid, por Mateusz Viste

El codigo para la reproduccion de ficheros WAV esta basado en el codigo fuente de auplay, por John Tsiombikas

El fichero MIDI que se reproduce durante el juego es de dominio publico

Algunos de los contenidos del juego, como algunas de las imagenes y dibujos han sido generados usando inteligencia artificial

El resto del contenido constituye una obra original
