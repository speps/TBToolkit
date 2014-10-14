/*
    OpenGEX Import Template Software License
    ==========================================

    OpenGEX Import Template, version 1.0
    Copyright 2014, Eric Lengyel
    All rights reserved.

    The OpenGEX Import Template is free software published on the following website:

        http://opengex.org/

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the entire text of this license,
    comprising the above copyright notice, this list of conditions, and the following
    disclaimer.
    
    2. Redistributions of any modified source code files must contain a prominent
    notice immediately following this license stating that the contents have been
    modified from their original form.

    3. Redistributions in binary form must include attribution to the author in any
    listing of credits provided with the distribution. If there is no listing of
    credits, then attribution must be included in the documentation and/or other
    materials provided with the distribution. The attribution must be exactly the
    statement "This software contains an OpenGEX import module based on work by
    Eric Lengyel" (without quotes).

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef OpenGEX_h
#define OpenGEX_h


#include "OpenDDL.h"


using namespace ODDL;


namespace OGEX
{
    enum
    {
        kStructureMetric						= 'mtrc',
        kStructureVertexArray					= 'vert',
        kStructureIndexArray					= 'indx',
        kStructureMesh							= 'mesh',
        kStructureNode							= 'node',
        kStructureBoneNode						= 'bnnd',
        kStructureGeometryNode					= 'gmnd',
        kStructureLightNode						= 'ltnd',
        kStructureCameraNode					= 'cmnd',
        kStructureObject						= 'objc',
        kStructureGeometryObject				= 'gmob',
        kStructureLightObject					= 'ltob',
        kStructureCameraObject					= 'cmob',
        kStructureMatrix						= 'mtrx',
        kStructureTransform						= 'xfrm',
        kStructureTranslation					= 'xslt',
        kStructureRotation						= 'rota',
        kStructureScale							= 'scal',
        kStructureName							= 'name',
        kStructureObjectRef						= 'obrf',
        kStructureMaterialRef					= 'mtrf',
        kStructureMorph							= 'mrph',
        kStructureBoneRefArray					= 'bref',
        kStructureBoneCountArray				= 'bcnt',
        kStructureBoneIndexArray				= 'bidx',
        kStructureBoneWeightArray				= 'bwgt',
        kStructureSkeleton						= 'skel',
        kStructureSkin							= 'skin',
        kStructureMaterial						= 'matl',
        kStructureAttrib						= 'attr',
        kStructureParam							= 'parm',
        kStructureColor							= 'colr',
        kStructureTexture						= 'txtr',
        kStructureAtten							= 'attn',
        kStructureKey							= 'key ',
        kStructureCurve							= 'curv',
        kStructureTime							= 'time',
        kStructureValue							= 'valu',
        kStructureTrack							= 'trac',
        kStructureAnimation						= 'anim'
    };


    enum
    {
        kDataOpenGexInvalidUpDirection			= 'ivud',
        kDataOpenGexInvalidTranslationKind		= 'ivtk',
        kDataOpenGexInvalidRotationKind			= 'ivrk',
        kDataOpenGexInvalidScaleKind			= 'ivsk',
        kDataOpenGexDuplicateLod				= 'dlod',
        kDataOpenGexMissingLodSkin				= 'mlsk',
        kDataOpenGexUndefinedLightType			= 'ivlt',
        kDataOpenGexUndefinedAttrib				= 'udab',
        kDataOpenGexUndefinedCurve				= 'udcv',
        kDataOpenGexUndefinedAtten				= 'udan',
        kDataOpenGexDuplicateVertexArray		= 'dpva',
        kDataOpenGexPositionArrayRequired		= 'parq',
        kDataOpenGexVertexCountUnsupported		= 'vcus',
        kDataOpenGexIndexValueUnsupported		= 'ivus',
        kDataOpenGexIndexArrayRequired			= 'iarq',
        kDataOpenGexVertexCountMismatch			= 'vcmm',
        kDataOpenGexBoneCountMismatch			= 'bcmm',
        kDataOpenGexBoneWeightCountMismatch		= 'bwcm',
        kDataOpenGexInvalidBoneRef				= 'ivbr',
        kDataOpenGexInvalidObjectRef			= 'ivor',
        kDataOpenGexInvalidMaterialRef			= 'ivmr',
        kDataOpenGexMaterialIndexUnsupported	= 'mius',
        kDataOpenGexDuplicateMaterialRef		= 'dprf',
        kDataOpenGexMissingMaterialRef			= 'msrf',
        kDataOpenGexTargetRefNotLocal			= 'trnl',
        kDataOpenGexInvalidTargetStruct			= 'ivts',
        kDataOpenGexInvalidKeyKind				= 'ivkk',
        kDataOpenGexInvalidCurveType			= 'ivct',
        kDataOpenGexKeyCountMismatch			= 'kycm',
        kDataOpenGexEmptyKeyStructure			= 'emky'
    };


    class ObjectStructure;
    class GeometryObjectStructure;
    class LightObjectStructure;
    class CameraObjectStructure;
    class SkinStructure;
    class MaterialStructure;
    class OpenGexDataDescription;


    class MetricStructure : public Structure
    {
        private:

            String		metricKey;

        public:

            MetricStructure();
            ~MetricStructure();

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class VertexArrayStructure : public Structure
    {
        private:

            String				arrayAttrib;
            unsigned_int32		morphIndex;

        public:

            VertexArrayStructure();
            ~VertexArrayStructure();

            const String& GetArrayAttrib() const
            {
                return arrayAttrib;
            }

            unsigned_int32 GetMorphIndex(void) const
            {
                return (morphIndex);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class IndexArrayStructure : public Structure
    {
        private:

            unsigned_int32			materialIndex;
            unsigned_int64			restartIndex;
            String					frontFace;

        public:

            IndexArrayStructure();
            ~IndexArrayStructure();

            unsigned_int32 GetMaterialIndex(void) const
            {
                return (materialIndex);
            }

            unsigned_int64 GetRestartIndex(void) const
            {
                return (restartIndex);
            }

            const String& GetFrontFace(void) const
            {
                return (frontFace);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class MeshStructure : public Structure, public MapElement<MeshStructure>
    {
        private:

            unsigned_int32					meshLevel;
            String							meshPrimitive;

            SkinStructure					*skinStructure;

        public:

            typedef unsigned_int32 KeyType;

            MeshStructure();
            ~MeshStructure();

            using MapElement<MeshStructure>::Previous;
            using MapElement<MeshStructure>::Next;

            KeyType GetKey(void) const
            {
                return (meshLevel);
            }

            unsigned_int32 GetMeshLevel() const
            {
                return meshLevel;
            }

            const String& GetMeshPrimitive() const
            {
                return meshPrimitive;
            }

            SkinStructure *GetSkinStructure(void) const
            {
                return (skinStructure);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class NodeStructure : public Structure
    {
        private:

            const char			*nodeName;

            virtual const ObjectStructure *GetObjectStructure(void) const;

            void CalculateTransforms(const OpenGexDataDescription *dataDescription);

        protected:

            NodeStructure(StructureType type);

        public:

            NodeStructure();
            ~NodeStructure();

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class BoneNodeStructure : public NodeStructure
    {
        public:

            BoneNodeStructure();
            ~BoneNodeStructure();
    };


    class GeometryNodeStructure : public NodeStructure
    {
        private:

            bool		visibleFlag[2];
            bool		shadowFlag[2];
            bool		motionBlurFlag[2];

            GeometryObjectStructure					*geometryObjectStructure;
            Array<const MaterialStructure *, 4>		materialStructureArray;

            const ObjectStructure *GetObjectStructure(void) const override;

        public:

            GeometryNodeStructure();
            ~GeometryNodeStructure();

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class LightNodeStructure : public NodeStructure
    {
        private:

            bool		shadowFlag[2];

            const LightObjectStructure		*lightObjectStructure;

            const ObjectStructure *GetObjectStructure(void) const override;

        public:

            LightNodeStructure();
            ~LightNodeStructure();

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class CameraNodeStructure : public NodeStructure
    {
        private:

            const CameraObjectStructure		*cameraObjectStructure;

            const ObjectStructure *GetObjectStructure(void) const override;

        public:

            CameraNodeStructure();
            ~CameraNodeStructure();

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class ObjectStructure : public Structure
    {
        protected:

            ObjectStructure(StructureType type);

        public:

            ~ObjectStructure();
    };


    class GeometryObjectStructure : public ObjectStructure
    {
        private:

            bool					visibleFlag;
            bool					shadowFlag;
            bool					motionBlurFlag;

            Map<MeshStructure>		meshMap;

        public:

            GeometryObjectStructure();
            ~GeometryObjectStructure();

            const Map<MeshStructure> *GetMeshMap(void) const
            {
                return (&meshMap);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class LightObjectStructure : public ObjectStructure
    {
        private:

            String		typeString;
            bool		shadowFlag;

        public:

            LightObjectStructure();
            ~LightObjectStructure();

            bool GetShadowFlag(void) const
            {
                return (shadowFlag);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class CameraObjectStructure : public ObjectStructure
    {
        private:

            float		focalLength;
            float		nearDepth;
            float		farDepth;

        public:

            CameraObjectStructure();
            ~CameraObjectStructure();

            float GetFocalLength(void) const
            {
                return (focalLength);
            }

            float GetNearDepth(void) const
            {
                return (nearDepth);
            }

            float GetFarDepth(void) const
            {
                return (farDepth);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class AnimatableStructure : public Structure
    {
        protected:

            AnimatableStructure(StructureType type);
            ~AnimatableStructure();
    };


    class MatrixStructure : public AnimatableStructure
    {
        private:

            bool		objectFlag;

        protected:

            MatrixStructure(StructureType type);

        public:

            ~MatrixStructure();

            bool GetObjectFlag(void) const
            {
                return (objectFlag);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
    };


    class TransformStructure final : public MatrixStructure
    {
        private:

            int32			transformCount;
            const float		*transformArray;
            bool            objectFlag;

        public:

            TransformStructure();
            ~TransformStructure();

            int32 GetTransformCount(void) const
            {
                return (transformCount);
            }

            const float *GetTransform(int32 index = 0) const
            {
                return (&transformArray[index * 16]);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class TranslationStructure final : public MatrixStructure
    {
        private:

            String		translationKind;

        public:

            TranslationStructure();
            ~TranslationStructure();

            const String& GetTranslationKind(void) const
            {
                return (translationKind);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class RotationStructure final : public MatrixStructure
    {
        private:

            String		rotationKind;

        public:

            RotationStructure();
            ~RotationStructure();

            const String& GetRotationKind(void) const
            {
                return (rotationKind);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class ScaleStructure final : public MatrixStructure
    {
        private:

            String		scaleKind;

        public:

            ScaleStructure();
            ~ScaleStructure();

            const String& GetScaleKind(void) const
            {
                return (scaleKind);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class NameStructure : public Structure
    {
        private:

            const char		*name;

        public:

            NameStructure();
            ~NameStructure();

            const char *GetName(void) const
            {
                return (name);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class ObjectRefStructure : public Structure
    {
        private:

            Structure		*targetStructure;

        public:

            ObjectRefStructure();
            ~ObjectRefStructure();

            Structure *GetTargetStructure(void) const
            {
                return (targetStructure);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class MaterialRefStructure : public Structure
    {
        private:

            unsigned_int32				materialIndex;
            const MaterialStructure		*targetStructure;

        public:

            MaterialRefStructure();
            ~MaterialRefStructure();

            unsigned_int32 GetMaterialIndex(void) const
            {
                return (materialIndex);
            }

            const MaterialStructure *GetTargetStructure(void) const
            {
                return (targetStructure);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class MorphStructure : public AnimatableStructure
    {
        private:

            int32		morphWeightCount;
            float		*morphWeightArray;

        public:

            MorphStructure();
            ~MorphStructure();

            int32 GetMorphWeightCount(void) const
            {
                return (morphWeightCount);
            }

            const float *GetMorphWeightArray(void) const
            {
                return (morphWeightArray);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class BoneRefArrayStructure : public Structure
    {
        private:

            int32						boneCount;
            const BoneNodeStructure		**boneNodeArray;

        public:

            BoneRefArrayStructure();
            ~BoneRefArrayStructure();

            int32 GetBoneCount(void) const
            {
                return (boneCount);
            }

            const BoneNodeStructure *const *GetBoneNodeArray(void) const
            {
                return (boneNodeArray);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class BoneCountArrayStructure : public Structure
    {
        private:

            int32					vertexCount;
            const unsigned_int16	*boneCountArray;
            unsigned_int16			*arrayStorage;

        public:

            BoneCountArrayStructure();
            ~BoneCountArrayStructure();

            int32 GetVertexCount(void) const
            {
                return (vertexCount);
            }

            const unsigned_int16 *GetBoneCountArray(void) const
            {
                return (boneCountArray);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class BoneIndexArrayStructure : public Structure
    {
        private:

            int32					boneIndexCount;
            const unsigned_int16	*boneIndexArray;
            unsigned_int16			*arrayStorage;

        public:

            BoneIndexArrayStructure();
            ~BoneIndexArrayStructure();

            int32 GetBoneIndexCount(void) const
            {
                return (boneIndexCount);
            }

            const unsigned_int16 *GetBoneIndexArray(void) const
            {
                return (boneIndexArray);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class BoneWeightArrayStructure : public Structure
    {
        private:

            int32			boneWeightCount;
            const float		*boneWeightArray;

        public:

            BoneWeightArrayStructure();
            ~BoneWeightArrayStructure();

            int32 GetBoneWeightCount(void) const
            {
                return (boneWeightCount);
            }

            const float *GetBoneWeightArray(void) const
            {
                return (boneWeightArray);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class SkeletonStructure : public Structure
    {
        private:

            const BoneRefArrayStructure		*boneRefArrayStructure;
            const TransformStructure		*transformStructure;

        public:

            SkeletonStructure();
            ~SkeletonStructure();

            const BoneRefArrayStructure *GetBoneRefArrayStructure(void) const
            {
                return (boneRefArrayStructure);
            }

            const TransformStructure *GetTransformStructure(void) const
            {
                return (transformStructure);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class SkinStructure : public Structure
    {
        private:

            const SkeletonStructure				*skeletonStructure;
            const BoneCountArrayStructure		*boneCountArrayStructure;
            const BoneIndexArrayStructure		*boneIndexArrayStructure;
            const BoneWeightArrayStructure		*boneWeightArrayStructure;

        public:

            SkinStructure();
            ~SkinStructure();

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class MaterialStructure : public Structure
    {
        private:

            bool				twoSidedFlag;
            const char			*materialName;

        public:

            MaterialStructure();
            ~MaterialStructure();

            bool GetTwoSidedFlag(void) const
            {
                return (twoSidedFlag);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class AttribStructure : public Structure
    {
        private:

            String		attribString;

        protected:

            AttribStructure(StructureType type);

        public:

            ~AttribStructure();

            const String& GetAttribString(void) const
            {
                return (attribString);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
    };


    class ParamStructure : public AttribStructure
    {
        private:

            float		param;

        public:

            ParamStructure();
            ~ParamStructure();

            float GetParam(void) const
            {
                return (param);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class ColorStructure : public AttribStructure
    {
        private:

            float		color[4];

        public:

            ColorStructure();
            ~ColorStructure();

            const float *GetColor(void) const
            {
                return (color);
            }

            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class TextureStructure : public AttribStructure
    {
        private:

            String				textureName;
            unsigned_int32		texcoordIndex;

        public:

            TextureStructure();
            ~TextureStructure();

            const char *GetTextureName(void) const
            {
                return (textureName);
            }

            unsigned_int32 GetTexcoordIndex(void) const
            {
                return (texcoordIndex);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class AttenStructure : public Structure
    {
        private:

            String			attenKind;
            String			curveType;

            float			beginParam;
            float			endParam;

            float			scaleParam;
            float			offsetParam;

            float			constantParam;
            float			linearParam;
            float			quadraticParam;

            float			powerParam;

        public:

            AttenStructure();
            ~AttenStructure();

            const String& GetAttenKind(void) const
            {
                return (attenKind);
            }

            const String& GetCurveType(void) const
            {
                return (curveType);
            }

            float GetBeginParam(void) const
            {
                return (beginParam);
            }

            float GetEndParam(void) const
            {
                return (endParam);
            }

            float GetScaleParam(void) const
            {
                return (scaleParam);
            }

            float GetOffsetParam(void) const
            {
                return (offsetParam);
            }

            float GetConstantParam(void) const
            {
                return (constantParam);
            }

            float GetLinearParam(void) const
            {
                return (linearParam);
            }

            float GetQuadraticParam(void) const
            {
                return (quadraticParam);
            }

            float GetPowerParam(void) const
            {
                return (powerParam);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class KeyStructure : public Structure
    {
        private:

            String			keyKind;

            bool			scalarFlag;

        public:

            KeyStructure();
            ~KeyStructure();

            const String& GetKeyKind(void) const
            {
                return (keyKind);
            }

            bool GetScalarFlag(void) const
            {
                return (scalarFlag);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class CurveStructure : public Structure
    {
        private:

            String					curveType;

            const KeyStructure		*keyValueStructure;
            const KeyStructure		*keyControlStructure[2];
            const KeyStructure		*keyTensionStructure;
            const KeyStructure		*keyContinuityStructure;
            const KeyStructure		*keyBiasStructure;

        protected:

            int32					keyDataElementCount;

            CurveStructure(StructureType type);

        public:

            ~CurveStructure();

            const String& GetCurveType(void) const
            {
                return (curveType);
            }

            const KeyStructure *GetKeyValueStructure(void) const
            {
                return (keyValueStructure);
            }

            const KeyStructure *GetKeyControlStructure(int32 index) const
            {
                return (keyControlStructure[index]);
            }

            const KeyStructure *GetKeyTensionStructure(void) const
            {
                return (keyTensionStructure);
            }

            const KeyStructure *GetKeyContinuityStructure(void) const
            {
                return (keyContinuityStructure);
            }

            const KeyStructure *GetKeyBiasStructure(void) const
            {
                return (keyBiasStructure);
            }

            int32 GetKeyDataElementCount(void) const
            {
                return (keyDataElementCount);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class TimeStructure : public CurveStructure
    {
        public:

            TimeStructure();
            ~TimeStructure();

            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class ValueStructure : public CurveStructure
    {
        public:

            ValueStructure();
            ~ValueStructure();

            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class TrackStructure : public Structure
    {
        private:

            StructureRef			targetRef;

            AnimatableStructure		*targetStructure;
            const TimeStructure		*timeStructure;
            const ValueStructure	*valueStructure;

        public:

            TrackStructure();
            ~TrackStructure();

            const StructureRef& GetTargetRef(void) const
            {
                return (targetRef);
            }

            AnimatableStructure *GetTargetStructure(void) const
            {
                return (targetStructure);
            }

            const TimeStructure *GetTimeStructure(void) const
            {
                return (timeStructure);
            }

            const ValueStructure *GetValueStructure(void) const
            {
                return (valueStructure);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class AnimationStructure : public Structure
    {
        private:

            int32		clipIndex;

            bool		beginFlag;
            bool		endFlag;
            float		beginTime;
            float		endTime;

        public:

            AnimationStructure();
            ~AnimationStructure();

            int32 GetClipIndex(void) const
            {
                return (clipIndex);
            }

            bool ValidateProperty(const DataDescription *dataDescription, const String& identifier, DataType *type, void **value) override;
            bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
            DataResult ProcessData(DataDescription *dataDescription) override;
    };


    class OpenGexDataDescription : public DataDescription
    {
        private:

            float		distanceScale;
            float		angleScale;
            float		timeScale;
            int32		upDirection;

        public:

            OpenGexDataDescription();
            ~OpenGexDataDescription();

            float GetDistanceScale(void) const
            {
                return (distanceScale);
            }

            void SetDistanceScale(float scale)
            {
                distanceScale = scale;
            }

            float GetAngleScale(void) const
            {
                return (angleScale);
            }

            void SetAngleScale(float scale)
            {
                angleScale = scale;
            }

            float GetTimeScale(void) const
            {
                return (timeScale);
            }

            void SetTimeScale(float scale)
            {
                timeScale = scale;
            }

            int32 GetUpDirection(void) const
            {
                return (upDirection);
            }

            void SetUpDirection(int32 direction)
            {
                upDirection = direction;
            }

            Structure *ConstructStructure(const String& identifier) const override;
            bool ValidateTopLevelStructure(const Structure *structure) const override;
            DataResult ProcessData(void) override;
    };
}


#endif
