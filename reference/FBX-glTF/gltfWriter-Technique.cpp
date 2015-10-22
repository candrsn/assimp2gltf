//
// Copyright (c) Autodesk, Inc. All rights reserved 
//
// C++ glTF FBX importer/exporter plug-in
// by Cyrille Fauvel - Autodesk Developer Network (ADN)
// January 2015
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
#include "StdAfx.h"
#include "gltfWriter.h"

namespace _IOglTF_NS_ {

// https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePass.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePassDetails.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePassDetailsCommonProfile.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePassDetailsCommonProfileTexcoordBindings.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePassInstanceProgram.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePassInstanceProgramAttribute.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePassInstanceProgramUniform.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePassStates.schema.json

void gltfWriter::AdditionalTechniqueParameters (FbxNode *pNode, web::json::value &techniqueParameters, bool bHasNormals /*=false*/) {
	if ( bHasNormals ) {
		techniqueParameters [U("normalMatrix")] =web::json::value::object ({ // normal matrix
			{ U("semantic"), web::json::value::string (U("MODELVIEWINVERSETRANSPOSE")) },
			{ U("type"), web::json::value::number ((int)IOglTF::FLOAT_MAT3) }
		}) ;
	}
	techniqueParameters [U("modelViewMatrix")] =web::json::value::object ({ // modeliew matrix
		{ U("semantic"), web::json::value::string (U("MODELVIEW")) },
		{ U("type"), web::json::value::number ((int)IOglTF::FLOAT_MAT4) }
	}) ;
	techniqueParameters [U("projectionMatrix")] =web::json::value::object ({ // projection matrix
		{ U("semantic"), web::json::value::string (U("PROJECTION")) },
		{ U("type"), web::json::value::number ((int)IOglTF::FLOAT_MAT4) }
	}) ;

//d:\projects\gltf\converter\collada2gltf\shaders\commonprofileshaders.cpp #905
	//if ( hasSkinning ) {
	//	addSemantic ("vs", "attribute",
	//				 "JOINT", "joint", 1, false);
	//	addSemantic ("vs", "attribute",
	//				 "WEIGHT", "weight", 1, false);

	//	assert (techniqueExtras != nullptr);
	//	addSemantic ("vs", "uniform",
	//				 JOINTMATRIX, "jointMat", jointsCount, false, true /* force as an array */);
	//}

	// We ignore lighting if the only light we have is ambient
	int lightCount =pNode->GetScene ()->RootProperty.GetSrcObjectCount<FbxLight> () ;
	for ( int i =0 ; i < lightCount ; i++ ) {
		FbxLight *pLight =pNode->GetScene ()->RootProperty.GetSrcObject<FbxLight> (i) ;
		if ( lightCount == 1 && pLight->LightType.Get () == FbxLight::EType::ePoint )
			return ;
		//utility::string_t name =nodeId (pLight->GetNode ()) ;
		utility::string_t name =utility::conversions::to_string_t (pLight->GetTypeName ()) ;
		std::transform (name.begin (), name.end (), name.begin (), ::tolower) ;
		if ( pLight->LightType.Get () == FbxLight::EType::ePoint ) {
			techniqueParameters [name + utility::conversions::to_string_t (i) + U("Color")] =web::json::value::object ({ // Color
				{ U("type"), web::json::value::number ((int)IOglTF::FLOAT_VEC3) },
				{ U("value"), web::json::value::array ({{ pLight->Color.Get () [0], pLight->Color.Get () [1], pLight->Color.Get () [2] }}) }
			}) ;
		} else {
			techniqueParameters [name + utility::conversions::to_string_t (i) + U("Color")] =web::json::value::object ({ // Color
				{ U("type"), web::json::value::number ((int)IOglTF::FLOAT_VEC3) },
				{ U("value"), web::json::value::array ({{ pLight->Color.Get () [0], pLight->Color.Get () [1], pLight->Color.Get () [2] }}) }
			}) ;
			techniqueParameters [name + utility::conversions::to_string_t (i) + U("Transform")] =web::json::value::object ({ // Transform
				{ U("semantic"), web::json::value::string (U("MODELVIEW")) },
				{ U("source"), web::json::value::string (utility::conversions::to_string_t (pLight->GetNode ()->GetName ())) },
				{ U("type"), web::json::value::number ((int)IOglTF::FLOAT_MAT4) }
			}) ;
			if ( pLight->LightType.Get () == FbxLight::EType::eDirectional ) {
				web::json::value lightDef =web::json::value::object () ;
				lightAttenuation (pLight, lightDef) ;
				if ( !lightDef [U("constantAttenuation")].is_null () )
					techniqueParameters [name + utility::conversions::to_string_t (i) + U("ConstantAttenuation")] =web::json::value::object ({
						{ U("type"), web::json::value::number ((int)IOglTF::FLOAT) },
						{ U("value"), lightDef [U("constantAttenuation")] }
					}) ;
				if ( !lightDef [U("linearAttenuation")].is_null () )
					techniqueParameters [name + utility::conversions::to_string_t (i) + U("LinearAttenuation")] =web::json::value::object ({
						{ U("type"), web::json::value::number ((int)IOglTF::FLOAT) },
						{ U("value"), lightDef [U("linearAttenuation")] }
					}) ;
				if ( !lightDef [U("quadraticAttenuation")].is_null () )
					techniqueParameters [name + utility::conversions::to_string_t (i) + U("QuadraticAttenuation")] =web::json::value::object ({
						{ U("type"), web::json::value::number ((int)IOglTF::FLOAT) },
						{ U("value"), lightDef [U("quadraticAttenuation")] }
					}) ;
			}
			if ( pLight->LightType.Get () == FbxLight::EType::eSpot ) {
				techniqueParameters [name + utility::conversions::to_string_t (i) + U("InverseTransform")] =web::json::value::object ({
					{ U("semantic"), web::json::value::string (U("MODELVIEWINVERSE")) },
					{ U("source"), web::json::value::string (utility::conversions::to_string_t (pLight->GetNode ()->GetName ())) },
					{ U("type"), web::json::value::number ((int)IOglTF::FLOAT_MAT4) }
				}) ;
				techniqueParameters [name + utility::conversions::to_string_t (i) + U("FallOffAngle")] =web::json::value::object ({
					{ U("type"), web::json::value::number ((int)IOglTF::FLOAT) },
					{ U("value"), web::json::value::number (DEG2RAD (pLight->OuterAngle)) }
				}) ;
				techniqueParameters [name + utility::conversions::to_string_t (i) + U("FallOffExponent")] =web::json::value::object ({
					{ U("type"), web::json::value::number ((int)IOglTF::FLOAT) },
					{ U("value"), web::json::value::number ((double)0.) }
				}) ;
			}
		}
	}
}

void gltfWriter::TechniqueParameters (FbxNode *pNode, web::json::value &techniqueParameters, web::json::value &attributes, web::json::value &accessors) {
	for ( const auto &iter : attributes.as_object () ) {
		utility::string_t name =iter.first ;
		std::transform (name.begin (), name.end (), name.begin (), ::tolower) ;
		//std::replace (name.begin (), name.end (), U('_'), U('x')) ;
		name.erase (std::remove (name.begin (), name.end (), U('_')), name.end ()) ;
		utility::string_t upperName (iter.first) ;
		std::transform (upperName.begin (), upperName.end (), upperName.begin (), ::toupper) ;
		web::json::value accessor =accessors [iter.second.as_string ()] ;
		techniqueParameters [name] =web::json::value::object ({
			{ U("semantic"), web::json::value::string (upperName) },
			{ U("type"), web::json::value::number ((int)IOglTF::techniqueParameters (accessor [U("type")].as_string ().c_str (), accessor [U("componentType")].as_integer ())) }
		}) ;
	}
}

web::json::value gltfWriter::WriteTechnique (FbxNode *pNode, FbxSurfaceMaterial *pMaterial, web::json::value &techniqueParameters) {
	web::json::value commonProfile =web::json::value::object () ;
	// The FBX SDK does not have such attribute. At best, it is an attribute of a Shader FX, CGFX or HLSL.
	commonProfile [U("extras")] =web::json::value::object ({{ U("doubleSided"), web::json::value::boolean (false) }}) ;
	commonProfile [U("lightingModel")] =web::json::value::string (LighthingModel (pMaterial)) ;
	commonProfile [U("parameters")] =web::json::value::array () ;
	if ( _uvSets.size () ) {
		commonProfile [U("texcoordBindings")] =web::json::value::object () ;
		for ( auto iter : _uvSets ) {
			utility::string_t key (iter.first) ;
			std::string::size_type i =key.find (U("Color")) ;
			if ( i != std::string::npos )
				key.erase (i, 5) ;
			std::transform (key.begin (), key.end (), key.begin (), ::tolower) ;
			commonProfile [U("texcoordBindings")] [key] =web::json::value::string (iter.second) ;
		}
	}
	for ( const auto &iter : techniqueParameters.as_object () ) {
		if (   (  utility::details::limitedCompareTo (iter.first, U("position")) != 0
			   && utility::details::limitedCompareTo (iter.first, U("normal")) != 0
			   && utility::details::limitedCompareTo (iter.first, U("texcoord")) != 0)
			|| iter.first == U("normalMatrix")
			)
			commonProfile [U("parameters")] [commonProfile [U("parameters")].size ()] =web::json::value::string (iter.first) ;

		web::json::value param =iter.second ;
		if ( param [U("type")].as_integer () == IOglTF::SAMPLER_2D ) {

		}
	}

	web::json::value details =web::json::value::object () ;
	details [U("commonProfile")] =commonProfile ;
	details [U("type")] =web::json::value::string (FBX_GLTF_COMMONPROFILE) ;

	web::json::value attributes =web::json::value::object () ;
	for ( const auto &iter : techniqueParameters.as_object () ) {
		if (   utility::details::limitedCompareTo (iter.first, U("position")) == 0
			|| (utility::details::limitedCompareTo (iter.first, U("normal")) == 0 && iter.first != U("normalMatrix"))
			|| utility::details::limitedCompareTo (iter.first, U("texcoord")) == 0 )
			attributes [utility::string_t (U("a_")) + iter.first] =web::json::value::string (iter.first) ;
	}

	web::json::value instanceProgram=web::json::value::object () ;
	instanceProgram [U("attributes")] =attributes ;
	instanceProgram [U("program")] =web::json::value::string (createUniqueId (utility::string_t (U("program")), 0)) ;
	instanceProgram [U("uniforms")] =web::json::value::object () ;
	for ( const auto &iter : techniqueParameters.as_object () ) {
		if (   (  utility::details::limitedCompareTo (iter.first, U("position")) != 0
			   && utility::details::limitedCompareTo (iter.first, U("normal")) != 0
			   && utility::details::limitedCompareTo (iter.first, U("texcoord")) != 0)
			|| iter.first == U("normalMatrix") )
			instanceProgram [U("uniforms")] [utility::string_t (U("u_")) + iter.first] =web::json::value::string (iter.first) ;
	}

	web::json::value techStatesEnable =web::json::value::array () ;
	if ( pNode->mCullingType != FbxNode::ECullingType::eCullingOff )
		techStatesEnable [techStatesEnable.size ()] =web::json::value::number ((int)IOglTF::CULL_FACE) ;
	// TODO: should it always be this way?
	techStatesEnable [techStatesEnable.size ()] =web::json::value::number ((int)IOglTF::DEPTH_TEST) ;

	web::json::value techStates =web::json::value::object () ;
	techStates [U("enable")] =techStatesEnable ;
	// TODO: needs to be implemented
	//techStates [U("functions")] =

	web::json::value techniquePass =web::json::value::object () ;
	techniquePass [U("details")] =details ;
	techniquePass [U("instanceProgram")] =instanceProgram ;
	techniquePass [U("states")] =techStates ;

	web::json::value technique =web::json::value::object () ;
	technique [U("parameters")] =techniqueParameters ;
	technique [U("pass")] =web::json::value::string (U("defaultPass")) ;
	technique [U("passes")] =web::json::value::object ({{ U("defaultPass"), techniquePass }}) ;

	return (technique) ;
}

}
