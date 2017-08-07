%VULKAN_SDK%\Bin32\glslc.exe -o triangle.vert.spv triangle.vert
%VULKAN_SDK%\Bin32\glslc.exe -o triangle.frag.spv triangle.frag
robocopy . ../../x64/Debug/shaders/ triangle.vert.spv triangle.frag.spv
pause