set runs=%1
cd x64
Release\RayTracerAss2.exe -runs %runs% -threads 8 -blockSize 64
Release\RayTracerAss2.exe -runs %runs% -threads 8 -blockSize 64 -samples 4
Release\RayTracerAss2.exe -runs %runs% -threads 8 -blockSize 64 -size 500 300
Release\RayTracerAss2.exe -runs %runs% -threads 8 -blockSize 64 -input ../Scenes/cornell-256lights.txt -size 512 512
Release\RayTracerAss2.exe -runs %runs% -threads 8 -blockSize 64 -input ../Scenes/allmaterials.txt
Release\RayTracerAss2.exe -runs %runs% -threads 8 -blockSize 64 -input ../Scenes/5000spheres.txt -size 960 540
Release\RayTracerAss2.exe -runs %runs% -threads 8 -blockSize 64 -input ../Scenes/bunny500.txt
Release\RayTracerAss2.exe -runs %runs% -threads 8 -blockSize 64 -input ../Scenes/bunny10k.txt -size 256 256
cd ..
