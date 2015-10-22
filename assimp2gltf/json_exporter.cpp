/*
assimp2gltf
Copyright (c) 2011, Alexander C. Gessler
Copyright (c) 2015, Vinjn Zhang

Licensed under a 3-clause BSD license. See the LICENSE file for more information.

*/

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>

#include <assimp/scene.h>

#include <sstream>
#include <limits>
#include <cassert>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filewritestream.h"

#define CURRENT_FORMAT_VERSION 100

// grab scoped_ptr from assimp to avoid a dependency on boost. 
#include <assimp/../../code/BoostWorkaround/boost/scoped_ptr.hpp>

#include "mesh_splitter.h"


extern "C" {
#include "cencode.h"
}

namespace {
void assimp2gltf(const char*, Assimp::IOSystem*, const aiScene*, const Assimp::ExportProperties*);
}

Assimp::Exporter::ExportFormatEntry assimp2gltf_desc = Assimp::Exporter::ExportFormatEntry(
	"assimp.gltf",
	"glTF representation of the Assimp scene data structure",
	"assimp.gltf",
	assimp2gltf,
	0u);

namespace {

using namespace rapidjson;

void Write(Writer<StringBuffer>& out, const aiVector3D& ai) 
{
	out.StartArray();
    out.Double(ai.x);
    out.Double(ai.y);
    out.Double(ai.z);
	out.EndArray();
}

void Write(Writer<StringBuffer>& out, const aiQuaternion& ai) 
{
	out.StartArray();
    out.Double(ai.w);
    out.Double(ai.x);
    out.Double(ai.y);
    out.Double(ai.z);
	out.EndArray();
}

void Write(Writer<StringBuffer>& out, const aiColor3D& ai) 
{
	out.StartArray();
    out.Double(ai.r);
    out.Double(ai.g);
    out.Double(ai.b);
	out.EndArray();
}

void Write(Writer<StringBuffer>& out, const aiMatrix4x4& ai) 
{
	out.StartArray();
	for(unsigned int x = 0; x < 4; ++x) {
		for(unsigned int y = 0; y < 4; ++y) {
			out.Double(ai[x][y]);
		}
	}
	out.EndArray();
}

void Write(Writer<StringBuffer>& out, const aiBone& ai)
{
	out.StartObject();

	out.Key("name");
	out.String(ai.mName.C_Str());

	out.Key("offsetmatrix");
	Write(out,ai.mOffsetMatrix);

	out.Key("weights");
	out.StartArray();
	for(unsigned int i = 0; i < ai.mNumWeights; ++i) {
		out.StartArray();
		out.Uint(ai.mWeights[i].mVertexId);
		out.Double(ai.mWeights[i].mWeight);
		out.EndArray();
	}
	out.EndArray();
	out.EndObject();
}


void Write(Writer<StringBuffer>& out, const aiFace& ai)
{
	out.StartArray();
	for(unsigned int i = 0; i < ai.mNumIndices; ++i) {
		out.Uint(ai.mIndices[i]);
	}
	out.EndArray();
}


void Write(Writer<StringBuffer>& out, const aiMesh& ai)
{
	out.StartObject(); 

	out.Key("name");
	out.String(ai.mName.C_Str());

	out.Key("materialindex");
	out.Uint(ai.mMaterialIndex);

	out.Key("primitivetypes");
    out.Uint(ai.mPrimitiveTypes);

	out.Key("vertices");
	out.StartArray();
	for(unsigned int i = 0; i < ai.mNumVertices; ++i) {
		out.Double(ai.mVertices[i].x);
        out.Double(ai.mVertices[i].y);
        out.Double(ai.mVertices[i].z);
	}
	out.EndArray();

	if(ai.HasNormals()) {
		out.Key("normals");
		out.StartArray();
		for(unsigned int i = 0; i < ai.mNumVertices; ++i) {
            out.Double(ai.mNormals[i].x);
            out.Double(ai.mNormals[i].y);
            out.Double(ai.mNormals[i].z);
		}
		out.EndArray();
	}

	if(ai.HasTangentsAndBitangents()) {
		out.Key("tangents");
		out.StartArray();
		for(unsigned int i = 0; i < ai.mNumVertices; ++i) {
            out.Double(ai.mTangents[i].x);
            out.Double(ai.mTangents[i].y);
            out.Double(ai.mTangents[i].z);
		}
		out.EndArray();

		out.Key("bitangents");
		out.StartArray();
		for(unsigned int i = 0; i < ai.mNumVertices; ++i) {
            out.Double(ai.mBitangents[i].x);
            out.Double(ai.mBitangents[i].y);
            out.Double(ai.mBitangents[i].z);
		}
		out.EndArray();
	}

	if(ai.GetNumUVChannels()) {
		out.Key("numuvcomponents");
		out.StartArray();
		for(unsigned int n = 0; n < ai.GetNumUVChannels(); ++n) {
			out.Uint(ai.mNumUVComponents[n]);
		}
		out.EndArray();

		out.Key("texturecoords");
		out.StartArray();
		for(unsigned int n = 0; n < ai.GetNumUVChannels(); ++n) {

			const unsigned int numc = ai.mNumUVComponents[n] ? ai.mNumUVComponents[n] : 2;
			
			out.StartArray();
			for(unsigned int i = 0; i < ai.mNumVertices; ++i) {
				for(unsigned int c = 0; c < numc; ++c) {
					out.Double(ai.mTextureCoords[n][i][c]);
				}
			}
			out.EndArray();
		}
		out.EndArray();
	}

	if(ai.GetNumColorChannels()) {
		out.Key("colors");
		out.StartArray();
		for(unsigned int n = 0; n < ai.GetNumColorChannels(); ++n) {

			out.StartArray();
			for(unsigned int i = 0; i < ai.mNumVertices; ++i) {
				out.Double(ai.mColors[n][i].r);
                out.Double(ai.mColors[n][i].g);
                out.Double(ai.mColors[n][i].b);
                out.Double(ai.mColors[n][i].a);
			}
			out.EndArray();
		}
		out.EndArray();
	}

	if(ai.mNumBones) {
		out.Key("bones");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumBones; ++n) {
			Write(out, *ai.mBones[n]);
		}
		out.EndArray();
	}


	out.Key("faces");
	out.StartArray();
	for(unsigned int n = 0; n < ai.mNumFaces; ++n) {
		Write(out, ai.mFaces[n]);
	}
	out.EndArray();

	out.EndObject();
}


void Write(Writer<StringBuffer>& out, const aiNode& ai)
{
	out.StartObject();

	out.Key("name");
	out.String(ai.mName.C_Str());

	out.Key("transformation");
	Write(out,ai.mTransformation);

	if(ai.mNumMeshes) {
		out.Key("meshes");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumMeshes; ++n) {
			out.Uint(ai.mMeshes[n]);
		}
		out.EndArray();
	}

	if(ai.mNumChildren) {
		out.Key("children");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumChildren; ++n) {
			Write(out,*ai.mChildren[n]);
		}
		out.EndArray();
	}

	out.EndObject();
}

void Write(Writer<StringBuffer>& out, const aiMaterial& ai)
{
	out.StartObject();

	out.Key("properties");
	out.StartArray();
	for(unsigned int i = 0; i < ai.mNumProperties; ++i) {
		const aiMaterialProperty* const prop = ai.mProperties[i];
		out.StartObject();
		out.Key("key");
		out.String(prop->mKey.C_Str());
		out.Key("semantic");
        out.Uint(prop->mSemantic);
		out.Key("index");
		out.Uint(prop->mIndex);

		out.Key("type");
		out.Int(prop->mType);

		out.Key("value");
		switch(prop->mType)
		{
		case aiPTI_Float:
			if(prop->mDataLength/sizeof(float) > 1) {
				out.StartArray();
				for(unsigned int i = 0; i < prop->mDataLength/sizeof(float); ++i) {
					out.Double(reinterpret_cast<float*>(prop->mData)[i]);
				}
				out.EndArray();
			}
			else {
                out.Double(*reinterpret_cast<float*>(prop->mData));
			}
			break;

		case aiPTI_Integer:
			if(prop->mDataLength/sizeof(int) > 1) {
				out.StartArray();
				for(unsigned int i = 0; i < prop->mDataLength/sizeof(int); ++i) {
					out.Int(reinterpret_cast<int*>(prop->mData)[i]);
				}
				out.EndArray();
			}
			else {
				out.Int(*reinterpret_cast<int*>(prop->mData));
			}
			break;
		case aiPTI_String: 
			{
				aiString s;
				aiGetMaterialString(&ai,prop->mKey.data,prop->mSemantic,prop->mIndex,&s);
				out.String(s.C_Str());
			}
			break;
		case aiPTI_Buffer:
			{
				// binary data is written as series of hex-encoded octets
				out.String(prop->mData,prop->mDataLength);
			}
			break;
		default:
			assert(false);
		}

		out.EndObject();
	}

	out.EndArray();
	out.EndObject();
}

void Write(Writer<StringBuffer>& out, const aiTexture& ai)
{
	out.StartObject();

	out.Key("width");
	out.Uint(ai.mWidth);

	out.Key("height");
	out.Uint(ai.mHeight);

	out.Key("formathint");
	out.String(aiString(ai.achFormatHint).C_Str());

	out.Key("data");
	if(!ai.mHeight) {
		out.String((char*)ai.pcData,ai.mWidth);
	}
	else {
		out.StartArray();
		for(unsigned int y = 0; y < ai.mHeight; ++y) {
			out.StartArray();
			for(unsigned int x = 0; x < ai.mWidth; ++x) {
				const aiTexel& tx = ai.pcData[y*ai.mWidth+x];
				out.StartArray();
				out.Uint(static_cast<unsigned int>(tx.r));
                out.Uint(static_cast<unsigned int>(tx.g));
                out.Uint(static_cast<unsigned int>(tx.b));
                out.Uint(static_cast<unsigned int>(tx.a));
				out.EndArray();
			}
			out.EndArray();
		}
		out.EndArray();
	}

	out.EndObject();
}

void Write(Writer<StringBuffer>& out, const aiLight& ai)
{
	out.StartObject();

	out.Key("name");
	out.String(ai.mName.C_Str());

	out.Key("type");
	out.Int(ai.mType);

	if(ai.mType == aiLightSource_SPOT || ai.mType == aiLightSource_UNDEFINED) {
		out.Key("angleinnercone");
		out.Double(ai.mAngleInnerCone);

		out.Key("angleoutercone");
        out.Double(ai.mAngleOuterCone);
	}

	out.Key("attenuationconstant");
    out.Double(ai.mAttenuationConstant);

	out.Key("attenuationlinear");
    out.Double(ai.mAttenuationLinear);

	out.Key("attenuationquadratic");
    out.Double(ai.mAttenuationQuadratic);

	out.Key("diffusecolor");
	Write(out,ai.mColorDiffuse);

	out.Key("specularcolor");
	Write(out,ai.mColorSpecular);

	out.Key("ambientcolor");
	Write(out,ai.mColorAmbient);

	if(ai.mType != aiLightSource_POINT) {
		out.Key("direction");
		Write(out,ai.mDirection);

	}

	if(ai.mType != aiLightSource_DIRECTIONAL) {
		out.Key("position");
		Write(out,ai.mPosition);
	}

	out.EndObject();
}

void Write(Writer<StringBuffer>& out, const aiNodeAnim& ai)
{
	out.StartObject();

	out.Key("name");
	out.String(ai.mNodeName.C_Str());

	out.Key("prestate");
	out.Int(ai.mPreState);

	out.Key("poststate");
    out.Int(ai.mPostState);

	if(ai.mNumPositionKeys) {
		out.Key("positionkeys");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumPositionKeys; ++n) {
			const aiVectorKey& pos = ai.mPositionKeys[n];
			out.StartArray();
            out.Double(pos.mTime);
			Write(out,pos.mValue);
			out.EndArray();
		}
		out.EndArray();
	}

	if(ai.mNumRotationKeys) {
		out.Key("rotationkeys");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumRotationKeys; ++n) {
			const aiQuatKey& rot = ai.mRotationKeys[n];
			out.StartArray();
            out.Double(rot.mTime);
			Write(out,rot.mValue);
			out.EndArray();
		}
		out.EndArray();
	}

	if(ai.mNumScalingKeys) {
		out.Key("scalingkeys");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumScalingKeys; ++n) {
			const aiVectorKey& scl = ai.mScalingKeys[n];
			out.StartArray();
            out.Double(scl.mTime);
			Write(out,scl.mValue);
			out.EndArray();
		}
		out.EndArray();
	}
	out.EndObject();
}

void Write(Writer<StringBuffer>& out, const aiAnimation& ai)
{
	out.StartObject();

	out.Key("name");
	out.String(ai.mName.C_Str());

	out.Key("tickspersecond");
    out.Double(ai.mTicksPerSecond);

	out.Key("duration");
    out.Double(ai.mDuration);

	out.Key("channels");
	out.StartArray();
	for(unsigned int n = 0; n < ai.mNumChannels; ++n) {
		Write(out,*ai.mChannels[n]);
	}
	out.EndArray();
	out.EndObject();
}

void Write(Writer<StringBuffer>& out, const aiCamera& ai)
{
    out.Key(ai.mName.C_Str());

    {
        out.StartObject();

        out.Key("name");
        out.String(ai.mName.C_Str());

        const char* kCamType = "perspective";
        out.Key(kCamType);
        {
            out.StartObject();

            out.Key("aspect_ratio");
            out.Double(ai.mAspect);

            out.Key("zfar");
            out.Double(ai.mClipPlaneFar);

            out.Key("znear");
            out.Double(ai.mClipPlaneNear);

            out.Key("yfov");
            out.Double(ai.mHorizontalFOV);

            out.Key("up");
            Write(out, ai.mUp);

            out.Key("lookat");
            Write(out, ai.mLookAt);

            out.EndObject();
        }
        out.Key("type");
        out.String(kCamType);

        out.EndObject();
    }
}

void WriteFormatInfo(Writer<StringBuffer>& out)
{
	out.StartObject();
	out.Key("format");
	out.String("\"assimp2gltf\"");
	out.Key("version");
    out.Int(CURRENT_FORMAT_VERSION);
	out.EndObject();
}

void Write(Writer<StringBuffer>& out, const aiScene& ai)
{
	out.StartObject();

	out.Key("__metadata__");
	WriteFormatInfo(out);

	out.Key("rootnode");
	Write(out,*ai.mRootNode);

	out.Key("flags");
	out.Uint(ai.mFlags);

	if(ai.HasMeshes()) {
		out.Key("meshes");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumMeshes; ++n) {
			Write(out,*ai.mMeshes[n]);
		}
		out.EndArray();
	}

	if(ai.HasMaterials()) {
		out.Key("materials");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumMaterials; ++n) {
			Write(out,*ai.mMaterials[n]);
		}
		out.EndArray();
	}

	if(ai.HasAnimations()) {
		out.Key("animations");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumAnimations; ++n) {
			Write(out,*ai.mAnimations[n]);
		}
		out.EndArray();
	}

	if(ai.HasLights()) {
		out.Key("lights");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumLights; ++n) {
			Write(out,*ai.mLights[n]);
		}
		out.EndArray();
	}

	if(ai.HasCameras()) {
		out.Key("cameras");
		out.StartObject();
		for(unsigned int n = 0; n < ai.mNumCameras; ++n) {
			Write(out,*ai.mCameras[n]);
		}
		out.EndObject();
	}

	if(ai.HasTextures()) {
		out.Key("textures");
		out.StartArray();
		for(unsigned int n = 0; n < ai.mNumTextures; ++n) {
			Write(out,*ai.mTextures[n]);
		}
		out.EndArray();
	}
	out.EndObject();
}


void assimp2gltf(const char* file, Assimp::IOSystem* io, const aiScene* scene, const Assimp::ExportProperties*) 
{
	boost::scoped_ptr<Assimp::IOStream> outStream(io->Open(file,"wt"));
    if (!outStream) {
		throw std::exception("could not open output file");
	}

	// get a copy of the scene so we can modify it
	aiScene* scenecopy_tmp;
	aiCopyScene(scene, &scenecopy_tmp);

	try {
		// split meshes so they fit into a 16 bit index buffer
		MeshSplitter splitter;
		splitter.SetLimit(1 << 16);
		splitter.Execute(scenecopy_tmp);

        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);

        Write(writer, *scenecopy_tmp);
        outStream->Write(sb.GetString(), sb.GetSize(), 1);
	}
	catch(...) {
		aiFreeScene(scenecopy_tmp);
		throw;
	}
	aiFreeScene(scenecopy_tmp);
}

} // 
