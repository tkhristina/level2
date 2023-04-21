/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

This is the main include file.
----------------------------------------------------------------------------------
This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

   Jarek Francik
   jarek@kingston.ac.uk
*********************************************************************************/
#include <GL/glew.h>
#include "Tools.h"
#include "Model.h"
#include "Shader.h"
#include "Terrain.h"
#include "SkyBox.h"
#include "Bitmap.h"

// link with AssImp and DevIL libraries
#pragma comment (lib, "assimp-vc143-mt.lib") 
#pragma comment (lib, "DevIL.lib") 

