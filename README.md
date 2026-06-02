# Andy Tracer
Humilde motor de renderizado por trazado de rayos acelerado por hardware con la plataforma de computación en paralelo CUDA.

## Índice
1. [Instrucciones](#instrucciones)
2. [Uso](#uso)
3. [Librerías](#librerías)
4. [Andy](#andy)

## Instrucciones
1. Instalar el toolkit de CUDA
2. Descargar el repositorio
3. Abrir la solución
4. Si la versión de CUDA difiere de 13.3
    - Click derecho en el proyecto
    - Dependencias de compilacion
    - Cambia la versión de CUDA a la tuya
5. Compialar y ejecutar

## Uso
Para usar la aplicación basta con modificar a tu gusto la configuración de ``config.h``
```c++
namespace config
{
	constexpr int MAX_REFLEXES = 10;

	constexpr int SAMPLES = 10;

    constexpr float RAY_EPSILON = 0.01f;
}
```
Y crear la escena que quieras en la constructora de ``Application`` al igual que en el ejemplo ya montado

Para moverte por la escena se usan las teclas ``W A S D`` y para girar la cámara las flechas ``Left`` y ``Right``

## Librerías
- [CUDA Toolkit](https://developer.nvidia.com/cuda/toolkit)
- [PixelToaster](https://github.com/thedmd/pixeltoaster)
- [GLM](https://github.com/g-truc/glm)

## Andy
Andy

![Andy](andy.jpeg)

