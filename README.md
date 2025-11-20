# GSEA - Gestión Segura y Eficiente de Archivos

Utilidad de línea de comandos en C++17 para comprimir/descomprimir (Huffman) y cifrar/descifrar (AES-128 modo CBC) archivos. Usa llamadas directas al sistema operativo para I/O y procesa varios archivos en paralelo (un hilo por archivo de primer nivel).

## Características
- Compresión sin pérdida con Huffman.
- Cifrado simétrico AES-128 en modo CBC.
- Procesamiento concurrente por archivo (no hay pool de hilos).
- I/O con API nativa (Win32 o POSIX).
- Multiplataforma: Windows y Linux/macOS.

### Limitaciones importantes
- Derivación de clave casera (XOR) y IV pseudoaleatorio (mt19937); sin autenticación de integridad (no hay MAC/AEAD). Adecuado solo para fines académicos, no para datos sensibles.
- Procesamiento de directorios no es recursivo y aplana nombres (solo archivos del primer nivel).
- Se lee cada archivo completo en memoria; archivos muy grandes pueden requerir mucha RAM.
- Un hilo por archivo puede agotar recursos si hay miles de archivos.

## Requisitos
### Windows
- MinGW-w64 o Microsoft Visual C++ (MSVC).
- Make (si usa MinGW).

### Linux/macOS
- GCC o Clang con C++17.
- Make y pthread.

## Compilación
### Windows (MinGW)
```bash
mingw32-make
```

### Windows (MSVC)
En la consola de desarrollador de Visual Studio:
```bash
nmake
```

### Linux/macOS
```bash
make
```
El ejecutable queda en `bin/gsea.exe` (Windows) o `bin/gsea` (POSIX).

## Uso
Sintaxis general:
```bash
gsea [OPCIONES]
```

Opciones:
- `-c` : Comprimir
- `-d` : Descomprimir
- `-e` : Encriptar
- `-u` : Desencriptar
- `-ce` : Comprimir y encriptar (operaciones combinables)
- `-ud` : Desencriptar y descomprimir
- `--comp-alg ALG` : Algoritmo de compresión (huffman)
- `--enc-alg ALG` : Algoritmo de cifrado (aes128)
- `-i PATH` : Archivo o directorio de entrada
- `-o PATH` : Archivo o directorio de salida
- `-k KEY` : Clave para cifrado/descifrado (requerida para -e/-u)
- `-h, --help` : Mostrar ayuda

Ejemplos rápidos:
```bash
bin/gsea -c  -i test/test_data.txt -o output/test1.huf
bin/gsea -d  -i output/test1.huf   -o output/test1_restored.txt
bin/gsea -e  -i test/test_data.txt -o output/test2.aes    -k clave123
bin/gsea -u  -i output/test2.aes   -o output/test2_plain  -k clave123
bin/gsea -ce -i test/test_data.txt -o output/test3.secure -k clave123
bin/gsea -ud -i output/test3.secure -o output/test3_plain -k clave123
```
Nota: Para directorios, se listan solo archivos de primer nivel y se genera un hilo por cada uno; el nombre de salida aplanado elimina sufijos `.aes` y `.huf` en modo `-ud`.

## Arquitectura (src/)
- `main.cpp`: orquesta operaciones y rutas.
- `arg_parser.*`: parseo/validación de argumentos.
- `file_manager.*`: I/O con llamadas del SO.
- `worker.*`: procesamiento concurrente (un hilo por archivo).
- `huffman.*`: compresión/descompresión Huffman.
- `aes.*`: cifrado/descifrado AES-128 CBC.

## Pruebas automatizadas
- Linux/macOS: `bash test_build.sh`
- Windows (MinGW): `.\test_build.bat`

Los scripts limpian, compilan y ejecutan un flujo básico de `-c/-d` y `-e/-u`, verificando integridad contra `test/test_data.txt`.

## Limpieza
```bash
make clean         # Linux/macOS
mingw32-make clean # Windows MinGW
```

## Autores
Juan Pablo Rúa Cartagena, Juan José Restrepo Higuita, Fredy Cadavid Franco.

## Licencia
Abierto para fines educativos.
