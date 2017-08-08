%VULKAN_SDK%\Bin32\glslc.exe -fshader-stage=vertex -o triangle.vert.spv triangle.vert
%VULKAN_SDK%\Bin32\glslc.exe -fshader-stage=fragment -o triangle.frag.spv triangle.frag
robocopy . ../../x64/Debug/shaders/ triangle.vert.spv triangle.frag.spv
pause