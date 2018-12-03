/****************************************************************************
 Copyright (c) 2013-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "renderer/CCTrianglesCommand.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCGLProgram.h"
#include "renderer/CCGLProgramState.h"
#include "external/xxhash/xxhash.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCTexture2D.h"

NS_CC_BEGIN

TrianglesCommand::TrianglesCommand()
:_materialID(0)
,_textureID(0)
,_glProgramState(nullptr)
,_blendType(BlendFunc::DISABLE)
,_alphaTextureID(0)
{
    _type = RenderCommand::Type::TRIANGLES_COMMAND;
}

void TrianglesCommand::init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles,const Mat4& mv, uint32_t flags)
{
    CCASSERT(glProgramState, "Invalid GLProgramState");
    CCASSERT(glProgramState->getVertexAttribsFlags() == 0, "No custom attributes are supported in QuadCommand");

    RenderCommand::init(globalOrder, mv, flags);

    _triangles = triangles;
    if(_triangles.indexCount % 3 != 0)
    {
        static_assert(sizeof(int) == 4, "We're not a 32-bit system!");
        //static_assert(sizeof(int) == 8, "We're not a 64-bit system!");
        int count = _triangles.indexCount;
        _triangles.indexCount = count / 3 * 3;
        CCLOGERROR("Resize indexCount from %d to %d, size must be multiple times of 3", count, _triangles.indexCount);
    }
    _mv = mv;
    
    if( _textureID != textureID || _blendType.src != blendType.src || _blendType.dst != blendType.dst ||
       _glProgramState != glProgramState)
    {
        _textureID = textureID;
        _blendType = blendType;
        _glProgramState = glProgramState;

        generateMaterialID();
    }
}

void TrianglesCommand::init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles,const Mat4& mv)
{
    init(globalOrder, textureID, glProgramState, blendType, triangles, mv, 0);
}

void TrianglesCommand::init(float globalOrder, Texture2D* texture, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles, const Mat4& mv, uint32_t flags)
{
    init(globalOrder, texture->getName(), glProgramState, blendType, triangles, mv, flags);
    _alphaTextureID = texture->getAlphaTextureName();
}

TrianglesCommand::~TrianglesCommand()
{
}

void TrianglesCommand::generateMaterialID()
{
    // glProgramState is hashed because it contains:
    //  *  uniforms/values
    //  *  glProgram
    //
    // we safely can when the same glProgramState is being used then they share those states
    // if they don't have the same glProgramState, they might still have the same
    // uniforms/values and glProgram, but it would be too expensive to check the uniforms.
    struct {
        void* glProgramState;
        GLuint textureId;
        GLenum blendSrc;
        GLenum blendDst;
    } hashMe;

    // NOTE: Initialize hashMe struct to make the value of padding bytes be filled with zero.
    // It's important since XXH32 below will also consider the padding bytes which probably 
    // are set to random values by different compilers.
    memset(&hashMe, 0, sizeof(hashMe)); 

    hashMe.textureId = _textureID;
    hashMe.blendSrc = _blendType.src;
    hashMe.blendDst = _blendType.dst;
    hashMe.glProgramState = _glProgramState;
    _materialID = XXH32((const void*)&hashMe, sizeof(hashMe), 0);
}

void TrianglesCommand::useMaterial() const
{
    //Set texture
    GL::bindTexture2D(_textureID);
    
    if (_alphaTextureID > 0)
    { // ANDROID ETC1 ALPHA supports.
        GL::bindTexture2DN(1, _alphaTextureID);
    }
    //set blend mode
    GL::blendFunc(_blendType.src, _blendType.dst);
    
    _glProgramState->apply(_mv);
}






///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// MARK: - PolygonInfo -
// NOTE: from CCAutoPoly.cpp

static unsigned short quadIndices9[]={
    0+4*0,1+4*0,2+4*0, 3+4*0,2+4*0,1+4*0,
    0+4*1,1+4*1,2+4*1, 3+4*1,2+4*1,1+4*1,
    0+4*2,1+4*2,2+4*2, 3+4*2,2+4*2,1+4*2,
    0+4*3,1+4*3,2+4*3, 3+4*3,2+4*3,1+4*3,
    0+4*4,1+4*4,2+4*4, 3+4*4,2+4*4,1+4*4,
    0+4*5,1+4*5,2+4*5, 3+4*5,2+4*5,1+4*5,
    0+4*6,1+4*6,2+4*6, 3+4*6,2+4*6,1+4*6,
    0+4*7,1+4*7,2+4*7, 3+4*7,2+4*7,1+4*7,
    0+4*8,1+4*8,2+4*8, 3+4*8,2+4*8,1+4*8,
};

PolygonInfo::PolygonInfo()
: _isVertsOwner(true)
, _rect(Rect::ZERO)
//, _filename("")
{
    triangles.verts = nullptr;
    triangles.indices = nullptr;
    triangles.vertCount = 0;
    triangles.indexCount = 0;
};

PolygonInfo::PolygonInfo(const PolygonInfo& other)
: triangles()
, _isVertsOwner(true)
, _rect()
{
//    _filename = other._filename;
    _isVertsOwner = true;
    _rect = other._rect;
    triangles.verts = new (std::nothrow) V3F_C4B_T2F[other.triangles.vertCount];
    triangles.indices = new (std::nothrow) unsigned short[other.triangles.indexCount];
    CCASSERT(triangles.verts && triangles.indices, "not enough memory");
    triangles.vertCount = other.triangles.vertCount;
    triangles.indexCount = other.triangles.indexCount;
    memcpy(triangles.verts, other.triangles.verts, other.triangles.vertCount * sizeof(other.triangles.verts[0]));
    memcpy(triangles.indices, other.triangles.indices, other.triangles.indexCount * sizeof(other.triangles.indices[0]));
};

PolygonInfo& PolygonInfo::operator= (const PolygonInfo& other)
{
    if(this != &other)
    {
        releaseVertsAndIndices();
//        _filename = other._filename;
        _isVertsOwner = true;
        _rect = other._rect;
        triangles.verts = new (std::nothrow) V3F_C4B_T2F[other.triangles.vertCount];
        triangles.indices = new (std::nothrow) unsigned short[other.triangles.indexCount];
        CCASSERT(triangles.verts && triangles.indices, "not enough memory");
        triangles.vertCount = other.triangles.vertCount;
        triangles.indexCount = other.triangles.indexCount;
        memcpy(triangles.verts, other.triangles.verts, other.triangles.vertCount * sizeof(other.triangles.verts[0]));
        memcpy(triangles.indices, other.triangles.indices, other.triangles.indexCount * sizeof(other.triangles.indices[0]));
    }
    return *this;
}

PolygonInfo::~PolygonInfo()
{
    releaseVertsAndIndices();
}

void PolygonInfo::setQuad(V3F_C4B_T2F_Quad *quad)
{
    releaseVertsAndIndices();
    _isVertsOwner = false;
    triangles.indices = quadIndices9;
    triangles.vertCount = 4;
    triangles.indexCount = 6;
    triangles.verts = (V3F_C4B_T2F*)quad;
}

void PolygonInfo::setQuads(V3F_C4B_T2F_Quad *quad, int numberOfQuads)
{
    CCASSERT(numberOfQuads >= 1 && numberOfQuads <= 9, "Invalid number of Quads");

    releaseVertsAndIndices();
    _isVertsOwner = false;
    triangles.indices = quadIndices9;
    triangles.vertCount = 4 * numberOfQuads;
    triangles.indexCount = 6 * numberOfQuads;
    triangles.verts = (V3F_C4B_T2F*)quad;
}

void PolygonInfo::setTriangles(const TrianglesCommand::Triangles& other)
{
    this->releaseVertsAndIndices();
    _isVertsOwner = false;

    this->triangles.vertCount = other.vertCount;
    this->triangles.indexCount = other.indexCount;
    this->triangles.verts = other.verts;
    this->triangles.indices = other.indices;
}

void PolygonInfo::releaseVertsAndIndices()
{
    if(_isVertsOwner)
    {
        if(nullptr != triangles.verts)
        {
            CC_SAFE_DELETE_ARRAY(triangles.verts);
        }

        if(nullptr != triangles.indices)
        {
            CC_SAFE_DELETE_ARRAY(triangles.indices);
        }
    }
}

unsigned int PolygonInfo::getVertCount() const
{
    return (unsigned int)triangles.vertCount;
}

unsigned int PolygonInfo::getTrianglesCount() const
{
    return (unsigned int)triangles.indexCount/3;
}

unsigned int PolygonInfo::getTriaglesCount() const
{
    return getTrianglesCount();
}

float PolygonInfo::getArea() const
{
    float area = 0;
    V3F_C4B_T2F *verts = triangles.verts;
    unsigned short *indices = triangles.indices;
    for(int i = 0; i < triangles.indexCount; i+=3)
    {
        auto A = verts[indices[i]].vertices;
        auto B = verts[indices[i+1]].vertices;
        auto C = verts[indices[i+2]].vertices;
        area += (A.x*(B.y-C.y) + B.x*(C.y-A.y) + C.x*(A.y - B.y))/2;
    }
    return area;
}


NS_CC_END
