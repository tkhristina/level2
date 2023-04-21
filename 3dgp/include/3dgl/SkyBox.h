/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

A very simple Sky Box class.
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
#ifndef _3dglSkyBox_H
#define _3dglSkyBox_H

// Include GLM core features
#include "../glm/glm.hpp"

#include "VAO.h"

namespace _3dgl
{
    class C3dglProgram;

    class MY3DGL_API C3dglSkyBox : public C3dglVertexAttrObject
    {
        unsigned int  m_idTex[6];

    public:
        C3dglSkyBox();

	    bool load(const char* pFd, const char* pRt, const char* pBk, const char* pLt, const char* pUp, const char* pDn, C3dglProgram *pProgram = NULL);
	    
        void render(glm::mat4 matrix, C3dglProgram* pProgram = NULL) const;
        virtual void render(GLsizei instances = 1) const;

        std::string getName() const { return "SkyBox"; }
    };
}
#endif