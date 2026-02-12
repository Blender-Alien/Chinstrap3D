#!/usr/bin/bash

${VULKAN_SDK}/bin/glslc Basic.frag -o BasicFragment.spv
${VULKAN_SDK}/bin/glslc Basic.vert -o BasicVertex.spv
