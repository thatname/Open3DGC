/*
Copyright (c) 2013 Khaled Mammou - Advanced Micro Devices, Inc.

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
*/

#pragma once
#ifndef O3DGC_SC3DMC_DECODER_INL
#define O3DGC_SC3DMC_DECODER_INL

#include "o3dgcArithmeticCodec.h"
#include "o3dgcTimer.h"

//#define DEBUG_VERBOSE

namespace o3dgc
{
#ifdef DEBUG_VERBOSE
        FILE * g_fileDebugSC3DMCDec = NULL;
#endif //DEBUG_VERBOSE

    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::DecodeHeader(IndexedFaceSet<T> & ifs, 
                                                  const BinaryStream & bstream)
    {
        unsigned long iterator0 = m_iterator;
        unsigned long start_code = bstream.ReadUInt32(m_iterator, O3DGC_SC3DMC_STREAM_TYPE_BINARY);
        if (start_code != O3DGC_SC3DMC_START_CODE)
        {
            m_iterator = iterator0;
            start_code = bstream.ReadUInt32(m_iterator, O3DGC_SC3DMC_STREAM_TYPE_ASCII);
            if (start_code != O3DGC_SC3DMC_START_CODE)
            {
                return O3DGC_ERROR_CORRUPTED_STREAM;
            }
            else
            {
                m_streamType = O3DGC_SC3DMC_STREAM_TYPE_ASCII;
            }
        }
        else
        {
            m_streamType = O3DGC_SC3DMC_STREAM_TYPE_BINARY;
        }
            
        m_streamSize = bstream.ReadUInt32(m_iterator, m_streamType); // to be filled later
        m_params.SetEncodeMode( (O3DGCSC3DMCEncodingMode) bstream.ReadUChar(m_iterator, m_streamType));

        ifs.SetCreaseAngle((Real) bstream.ReadFloat32(m_iterator, m_streamType));
          
        unsigned char mask = bstream.ReadUChar(m_iterator, m_streamType);

        ifs.SetCCW             ((mask & 1) == 1);
        ifs.SetSolid           ((mask & 2) == 1);
        ifs.SetConvex          ((mask & 4) == 1);
        ifs.SetIsTriangularMesh((mask & 8) == 1);
        //bool markerBit0 = (mask & 16 ) == 1;
        //bool markerBit1 = (mask & 32 ) == 1;
        //bool markerBit2 = (mask & 64 ) == 1;
        //bool markerBit3 = (mask & 128) == 1;
       
        ifs.SetNCoord         (bstream.ReadUInt32(m_iterator, m_streamType));
        ifs.SetNNormal        (bstream.ReadUInt32(m_iterator, m_streamType));
        ifs.SetNColor         (bstream.ReadUInt32(m_iterator, m_streamType));
        ifs.SetNTexCoord      (bstream.ReadUInt32(m_iterator, m_streamType));


        ifs.SetNumFloatAttributes(bstream.ReadUInt32(m_iterator, m_streamType));
        ifs.SetNumIntAttributes  (bstream.ReadUInt32(m_iterator, m_streamType));
                              
        if (ifs.GetNCoord() > 0)
        {
            ifs.SetNCoordIndex(bstream.ReadUInt32(m_iterator, m_streamType));
            for(int j=0 ; j<3 ; ++j)
            {
                ifs.SetCoordMin(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                ifs.SetCoordMax(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
            }
            m_params.SetCoordQuantBits( bstream.ReadUChar(m_iterator, m_streamType) );
        }
        if (ifs.GetNNormal() > 0)
        {
            ifs.SetNNormalIndex(bstream.ReadUInt32(m_iterator, m_streamType));
            for(int j=0 ; j<3 ; ++j)
            {
                ifs.SetNormalMin(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                ifs.SetNormalMax(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
            }
            ifs.SetNormalPerVertex(bstream.ReadUChar(m_iterator, m_streamType) == 1);
            m_params.SetNormalQuantBits(bstream.ReadUChar(m_iterator, m_streamType));
        }
        if (ifs.GetNColor() > 0)
        {
            ifs.SetNColorIndex(bstream.ReadUInt32(m_iterator, m_streamType));
            for(int j=0 ; j<3 ; ++j)
            {
                ifs.SetColorMin(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                ifs.SetColorMax(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
            }
            ifs.SetColorPerVertex(bstream.ReadUChar(m_iterator, m_streamType)==1);
            m_params.SetColorQuantBits(bstream.ReadUChar(m_iterator, m_streamType));
        }
        if (ifs.GetNTexCoord() > 0)
        {
            ifs.SetNTexCoordIndex(bstream.ReadUInt32(m_iterator, m_streamType));
            for(int j=0 ; j<2 ; ++j)
            {
                ifs.SetTexCoordMin(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                ifs.SetTexCoordMax(j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
            }
            m_params.SetTexCoordQuantBits(bstream.ReadUChar(m_iterator, m_streamType));
        }

        for(unsigned long a = 0; a < ifs.GetNumFloatAttributes(); ++a)
        {
            ifs.SetNFloatAttribute(a, bstream.ReadUInt32(m_iterator, m_streamType));    
            if (ifs.GetNFloatAttribute(a) > 0)
            {
                ifs.SetNFloatAttributeIndex(a, bstream.ReadUInt32(m_iterator, m_streamType));
                unsigned char d = bstream.ReadUChar(m_iterator, m_streamType);
                ifs.SetFloatAttributeDim(a, d);
                for(unsigned char j = 0 ; j < d ; ++j)
                {
                    ifs.SetFloatAttributeMin(a, j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                    ifs.SetFloatAttributeMax(a, j, (Real) bstream.ReadFloat32(m_iterator, m_streamType));
                }
                ifs.SetFloatAttributePerVertex(a, bstream.ReadUChar(m_iterator, m_streamType) == 1);
                m_params.SetFloatAttributeQuantBits(a, bstream.ReadUChar(m_iterator, m_streamType));
            }
        }
        for(unsigned long a = 0; a < ifs.GetNumIntAttributes(); ++a)
        {
            ifs.SetNIntAttribute(a, bstream.ReadUInt32(m_iterator, m_streamType));
            if (ifs.GetNIntAttribute(a) > 0)
            {
                ifs.SetNIntAttributeIndex(a, bstream.ReadUInt32(m_iterator, m_streamType));
                ifs.SetIntAttributeDim(a, bstream.ReadUChar(m_iterator, m_streamType));
                ifs.SetIntAttributePerVertex(a, bstream.ReadUChar(m_iterator, m_streamType) == 1);
            }
        }    
        return O3DGC_OK;
    }
    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::DecodePlayload(IndexedFaceSet<T> & ifs,
                                                    const BinaryStream & bstream)
    {
#ifdef DEBUG_VERBOSE
        g_fileDebugSC3DMCDec = fopen("tfans_dec_main.txt", "w");
#endif //DEBUG_VERBOSE

        m_triangleListDecoder.SetStreamType(m_streamType);
        m_stats.m_streamSizeCoordIndex = m_iterator;
        Timer timer;
        timer.Tic();
        m_triangleListDecoder.Decode(ifs.GetCoordIndex(), ifs.GetNCoordIndex(), ifs.GetNCoord(), bstream, m_iterator);
        timer.Toc();
        m_stats.m_timeCoordIndex       = timer.GetElapsedTime();
        m_stats.m_streamSizeCoordIndex = m_iterator - m_stats.m_streamSizeCoordIndex;

        // decode coord
        m_stats.m_streamSizeCoord = m_iterator;
        timer.Tic();
        if (ifs.GetNCoord() > 0)
        {
            DecodeFloatArray(ifs.GetCoord(), ifs.GetNCoord(), 3, ifs.GetCoordMin(), ifs.GetCoordMax(),
                             m_params.GetCoordQuantBits(), ifs, m_params.GetCoordPredMode(), bstream);
        }
        timer.Toc();
        m_stats.m_timeCoord       = timer.GetElapsedTime();
        m_stats.m_streamSizeCoord = m_iterator - m_stats.m_streamSizeCoord;

        // decode Normal
        m_stats.m_streamSizeNormal = m_iterator;
        timer.Tic();
        if (ifs.GetNNormal() > 0)
        {
            if (m_params.GetNormalPredMode() == O3DGC_SC3DMC_SURF_NORMALS_PREDICTION)
            {
                unsigned long nvert = ifs.GetNNormal();
                Real * const normals = ifs.GetNormal();
                ProcessNormals(ifs, bstream);
                DecodeFloatArray(normals, nvert, 3, ifs.GetNormalMin(), ifs.GetNormalMax(),
                                 m_params.GetNormalQuantBits(), ifs, m_params.GetNormalPredMode(), bstream);
                nvert *= 3;
                for (unsigned long long v=0; v < nvert; ++v) 
                {
                    normals[v] += m_normals[v];
                }
            }
            else
            {
                DecodeFloatArray(ifs.GetNormal(), ifs.GetNNormal(), 3, ifs.GetNormalMin(), ifs.GetNormalMax(),
                                 m_params.GetNormalQuantBits(), ifs, m_params.GetNormalPredMode(), bstream);

            }
        }
        timer.Toc();
        m_stats.m_timeNormal       = timer.GetElapsedTime();
        m_stats.m_streamSizeNormal = m_iterator - m_stats.m_streamSizeNormal;

        // decode Color
        m_stats.m_streamSizeColor = m_iterator;
        timer.Tic();
        if (ifs.GetNColor() > 0)
        {
            DecodeFloatArray(ifs.GetColor(), ifs.GetNColor(), 3, ifs.GetColorMin(), ifs.GetColorMax(),
                                m_params.GetColorQuantBits(), ifs, m_params.GetColorPredMode(), bstream);
        }
        timer.Toc();
        m_stats.m_timeColor       = timer.GetElapsedTime();
        m_stats.m_streamSizeColor = m_iterator - m_stats.m_streamSizeColor;

        // decode TexCoord
        m_stats.m_streamSizeFloatAttribute = m_iterator;
        timer.Tic();
        if (ifs.GetNTexCoord() > 0)
        {
            DecodeFloatArray(ifs.GetTexCoord(), ifs.GetNTexCoord(), 2, ifs.GetTexCoordMin(), ifs.GetTexCoordMax(), 
                                m_params.GetTexCoordQuantBits(), ifs, m_params.GetTexCoordPredMode(), bstream);
        }
        timer.Toc();
        m_stats.m_timeFloatAttribute       = timer.GetElapsedTime();
        m_stats.m_streamSizeFloatAttribute = m_iterator - m_stats.m_streamSizeFloatAttribute;

        m_stats.m_streamSizeIntAttribute = m_iterator;
        timer.Tic();
        for(unsigned long a = 0; a < ifs.GetNumFloatAttributes(); ++a)
        {
            DecodeFloatArray(ifs.GetFloatAttribute(a), ifs.GetNFloatAttribute(a), ifs.GetFloatAttributeDim(a), ifs.GetFloatAttributeMin(a), ifs.GetFloatAttributeMax(a), 
                                m_params.GetFloatAttributeQuantBits(a), ifs, m_params.GetFloatAttributePredMode(a), bstream);
        }
        timer.Toc();
        m_stats.m_timeIntAttribute       = timer.GetElapsedTime();
        m_stats.m_streamSizeIntAttribute = m_iterator - m_stats.m_streamSizeIntAttribute;

        m_stats.m_streamSizeIntAttribute = m_iterator;
        timer.Tic();
        for(unsigned long a = 0; a < ifs.GetNumIntAttributes(); ++a)
        {
            DecodeIntArray(ifs.GetIntAttribute(a), ifs.GetNIntAttribute(a), ifs.GetIntAttributeDim(a), bstream);
        }
        timer.Toc();
        m_stats.m_timeIntAttribute       = timer.GetElapsedTime();
        m_stats.m_streamSizeIntAttribute = m_iterator - m_stats.m_streamSizeIntAttribute;

#ifdef DEBUG_VERBOSE
        fclose(g_fileDebugSC3DMCDec);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }
    inline long DecodeIntACEGC(Arithmetic_Codec & acd,
                               Adaptive_Data_Model & mModelValues,
                               Static_Bit_Model & bModel0,
                               Adaptive_Bit_Model & bModel1,
                               const unsigned long exp_k,
                               const unsigned long M)
    {
        unsigned long uiValue = acd.decode(mModelValues);
        if (uiValue == M) 
        {
            uiValue += acd.ExpGolombDecode(exp_k, bModel0, bModel1);
        }

        if (uiValue & 1)
        {
            return - ((long)((uiValue+1) >> 1));
        }
        else
        {
            return  (long)(uiValue >> 1);
        }
    }
    inline unsigned long DecodeUIntACEGC(Arithmetic_Codec & acd,
                                         Adaptive_Data_Model & mModelValues,
                                         Static_Bit_Model & bModel0,
                                         Adaptive_Bit_Model & bModel1,
                                         const unsigned long exp_k,
                                         const unsigned long M)
    {
        unsigned long uiValue = acd.decode(mModelValues);
        if (uiValue == M) 
        {
            uiValue += acd.ExpGolombDecode(exp_k, bModel0, bModel1);
        }
        return uiValue;
    }
    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::DecodeIntArray(long * const intArray, 
                                                 unsigned long numIntArray,
                                                 unsigned long dimIntArray,
                                                 const BinaryStream & bstream)
    {        
        const long nvert = (long) numIntArray;

        unsigned char * buffer = 0;
        Arithmetic_Codec acd;
        Static_Bit_Model bModel0;
        Adaptive_Bit_Model bModel1;
        
        unsigned long start    = m_iterator;
        unsigned long sizeSize = bstream.ReadUInt32(m_iterator, m_streamType);  // bitsream size
        bstream.ReadUChar(m_iterator, m_streamType);                            // unsigned char mask = bstream.ReadUChar(m_iterator, m_streamType);
        unsigned int exp_k;
        unsigned int M = 0;

        long minValue = bstream.ReadUInt32(m_iterator, m_streamType) - O3DGC_MAX_LONG;
        sizeSize -= (m_iterator - start);
        
        if (m_streamType != O3DGC_SC3DMC_STREAM_TYPE_ASCII)
        {
            bstream.GetBuffer(m_iterator, buffer);
            m_iterator += sizeSize;
            acd.set_buffer(sizeSize, buffer);
            acd.start_decoder();
            exp_k = acd.ExpGolombDecode(0, bModel0, bModel1);
            M     = acd.ExpGolombDecode(0, bModel0, bModel1);
        }
        Adaptive_Data_Model mModelValues(M+2);

#ifdef DEBUG_VERBOSE
        printf("IntArray (%i, %i)\n", numIntArray, dimIntArray);
        fprintf(g_fileDebugSC3DMCDec, "IntArray (%i, %i)\n", numIntArray, dimIntArray);
#endif //DEBUG_VERBOSE
        if (m_streamType == O3DGC_SC3DMC_STREAM_TYPE_ASCII)
        {
            for (long v=0; v < nvert; ++v) 
            {
                for (unsigned long i = 0; i < dimIntArray; i++) 
                {
                    intArray[v*dimIntArray+i] = bstream.ReadUIntASCII(m_iterator) + minValue;
#ifdef DEBUG_VERBOSE
                    printf("%i\n", intArray[v*dimIntArray+i]);
                    fprintf(g_fileDebugSC3DMCDec, "%i\n", intArray[v*dimIntArray+i]);
#endif //DEBUG_VERBOSE
                }
            }
        }
        else
        {
            for (long v=0; v < nvert; ++v) 
            {
                for (unsigned long i = 0; i < dimIntArray; i++) 
                {
                    intArray[v*dimIntArray+i] = DecodeUIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M) + minValue;
#ifdef DEBUG_VERBOSE
                    printf("%i\n", intArray[v*dimIntArray+i]);
                    fprintf(g_fileDebugSC3DMCDec, "%i\n", intArray[v*dimIntArray+i]);
#endif //DEBUG_VERBOSE
                }
            }
        }
#ifdef DEBUG_VERBOSE
        fflush(g_fileDebugSC3DMCDec);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }
    template <class T>
    O3DGCErrorCode SC3DMCDecoder<T>::ProcessNormals(const IndexedFaceSet<T> & ifs,
                                                    const BinaryStream & bstream)
    {
        const long nvert  = (long) ifs.GetNNormal();

        // decode normals orientation information 
        const unsigned long size  = (unsigned long) nvert;
        unsigned long streamSize = m_iterator;
        streamSize = bstream.ReadUInt32(m_iterator, m_streamType) - streamSize;
        m_predictors.Allocate(size);
        m_predictors.Clear();
        if (m_streamType == O3DGC_SC3DMC_STREAM_TYPE_ASCII)
        {
            long symbol;
            for(unsigned long i = 0; i < streamSize;)
            {
                symbol = bstream.ReadUCharASCII(m_iterator);
                for(unsigned long h = 0; h < O3DGC_BINARY_STREAM_BITS_PER_SYMBOL0; ++h)
                {
                    m_predictors.PushBack(symbol & 1);
                    symbol >>= 1;
                    ++i;
                }
            }
        }
        else
        {
            if (streamSize == 0)
            {
                return O3DGC_OK;
            }
            unsigned char * buffer = 0;
            bstream.GetBuffer(m_iterator, buffer);
            m_iterator += streamSize;

            Arithmetic_Codec acd;
            acd.set_buffer(streamSize, buffer);
            acd.start_decoder();
            Adaptive_Bit_Model bModel;
            for(unsigned long i = 0; i < size; ++i)
            {
                m_predictors.PushBack(acd.decode(bModel));
            }
        }
        const unsigned long normalsSize = ifs.GetNNormal() * 3;
        if (m_normalsSize < normalsSize)
        {
            delete [] m_normals;
            m_normalsSize = normalsSize;
            m_normals     = new Real [size];
        }                                  
        const AdjacencyInfo & v2T          = m_triangleListDecoder.GetVertexToTriangle();
        const T * const       triangles    = ifs.GetCoordIndex();
        const Real * const originalNormals = ifs.GetNormal();
        Vec3<Real> p1, p2, p3, n0;
        long a, b, c;
        for (long vm=0; vm < nvert; ++vm) 
        {
            n0.X() = 0;
            n0.Y() = 0;
            n0.Z() = 0;
            int u0 = v2T.Begin(vm);
            int u1 = v2T.End(vm);
            for (long u = u0; u < u1; u++) 
            {
                long ta = v2T.GetNeighbor(u);
                a = triangles[ta*3 + 0];
                b = triangles[ta*3 + 1];
                c = triangles[ta*3 + 2];
                p1.X() = (Real) m_quantFloatArray[3*a];
                p1.Y() = (Real) m_quantFloatArray[3*a+1];
                p1.Z() = (Real) m_quantFloatArray[3*a+2];
                p2.X() = (Real) m_quantFloatArray[3*b];
                p2.Y() = (Real) m_quantFloatArray[3*b+1];
                p2.Z() = (Real) m_quantFloatArray[3*b+2];
                p3.X() = (Real) m_quantFloatArray[3*c];
                p3.Y() = (Real) m_quantFloatArray[3*c+1];
                p3.Z() = (Real) m_quantFloatArray[3*c+2];
                n0  += (p2-p1)^(p3-p1);
            }
            n0.Normalize();
            if (m_predictors[vm])
            {
                n0 = -n0;
            }
            m_normals[3*vm]   = n0.X();
            m_normals[3*vm+1] = n0.Y();
            m_normals[3*vm+2] = n0.Z();
        }
        return O3DGC_OK;
    }
    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::DecodeFloatArray(Real * const floatArray, 
                                                   unsigned long numFloatArray,
                                                   unsigned long dimFloatArray,
                                                   const Real * const minFloatArray,
                                                   const Real * const maxFloatArray,
                                                   unsigned long nQBits,
                                                   const IndexedFaceSet<T> & ifs,
                                                   O3DGCSC3DMCPredictionMode predMode,
                                                   const BinaryStream & bstream)
    {
        assert(dimFloatArray <  O3DGC_SC3DMC_MAX_DIM_FLOAT_ATTRIBUTES);
        long predResidual;
        SC3DMCPredictor m_neighbors  [O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS];
        Arithmetic_Codec acd;
        Static_Bit_Model bModel0;
        Adaptive_Bit_Model bModel1;
        Adaptive_Data_Model mModelPreds(O3DGC_SC3DMC_MAX_PREDICTION_NEIGHBORS+1);
        unsigned long nPred;

        const AdjacencyInfo & v2T        = m_triangleListDecoder.GetVertexToTriangle();
        const T * const     triangles    = ifs.GetCoordIndex();       
        const long          nvert        = (long) numFloatArray;
        const unsigned long size         = numFloatArray * dimFloatArray;
        unsigned char *     buffer       = 0;
        unsigned long       start        = m_iterator;
        unsigned long       sizeSize     = bstream.ReadUInt32(m_iterator, m_streamType);        // bitsream size
        bstream.ReadUChar(m_iterator, m_streamType); //        unsigned char mask = bstream.ReadUChar(m_iterator, m_streamType);
        sizeSize -= (m_iterator - start);

        unsigned long       iteratorPred = m_iterator + sizeSize;
        unsigned int        exp_k;
        unsigned int        M = 0;

        if (m_streamType != O3DGC_SC3DMC_STREAM_TYPE_ASCII)
        {
            
            bstream.GetBuffer(m_iterator, buffer);
            m_iterator += sizeSize;
            acd.set_buffer(sizeSize, buffer);
            acd.start_decoder();
            exp_k = acd.ExpGolombDecode(0, bModel0, bModel1);
            M     = acd.ExpGolombDecode(0, bModel0, bModel1);
        }
        else
        {
            bstream.ReadUInt32(iteratorPred, m_streamType);        // predictors bitsream size
        }
        Adaptive_Data_Model mModelValues(M+2);

#ifdef DEBUG_VERBOSE
        printf("FloatArray (%i, %i)\n", numFloatArray, dimFloatArray);
        fprintf(g_fileDebugSC3DMCDec, "FloatArray (%i, %i)\n", numFloatArray, dimFloatArray);
#endif //DEBUG_VERBOSE

        if (m_quantFloatArraySize < size)
        {
            delete [] m_quantFloatArray;
            m_quantFloatArraySize = size;
            m_quantFloatArray     = new long [size];
        }
        for (long v=0; v < nvert; ++v) 
        {
            nPred = 0;
            if ( v2T.GetNumNeighbors(v) > 0 && 
                 predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                int u0 = v2T.Begin(v);
                int u1 = v2T.End(v);
                for (long u = u0; u < u1; u++) 
                {
                    long ta = v2T.GetNeighbor(u);
                    if (ta < 0)
                    {
                        break;
                    }
                    if (predMode == O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION)
                    {
                        long a,b;
                        if ((long) triangles[ta*3] == v)
                        {
                            a = triangles[ta*3 + 1];
                            b = triangles[ta*3 + 2];
                        }
                        else if ((long)triangles[ta*3 + 1] == v)
                        {
                            a = triangles[ta*3 + 0];
                            b = triangles[ta*3 + 2];
                        }
                        else
                        {
                            a = triangles[ta*3 + 0];
                            b = triangles[ta*3 + 1];
                        }
                        if ( a < v && b < v)
                        {
                            int u0 = v2T.Begin(a);
                            int u1 = v2T.End(a);
                            for (long u = u0; u < u1; u++) 
                            {
                                long tb = v2T.GetNeighbor(u);
                                if (tb < 0)
                                {
                                    break;
                                }
                                long c = -1;
                                bool foundB = false;
                                for(long k = 0; k < 3; ++k)
                                {
                                    long x = triangles[tb*3 + k];
                                    if (x == b)
                                    {
                                        foundB = true;
                                    }
                                    if (x < v && x != a && x != b)
                                    {
                                        c = x;
                                    }
                                }
                                if (c != -1 && foundB)
                                {
                                    SC3DMCTriplet id = {min(a, b), max(a, b), -c-1};
                                    unsigned long p = Insert(id, nPred, m_neighbors);
                                    if (p != 0xFFFFFFFF)
                                    {
                                        for (unsigned long i = 0; i < dimFloatArray; i++) 
                                        {
                                            m_neighbors[p].m_pred[i] = m_quantFloatArray[a*dimFloatArray+i] + 
                                                                       m_quantFloatArray[b*dimFloatArray+i] - 
                                                                       m_quantFloatArray[c*dimFloatArray+i];
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if ( predMode == O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION ||
                         predMode == O3DGC_SC3DMC_DIFFERENTIAL_PREDICTION )
                    {                
                        for(long k = 0; k < 3; ++k)
                        {
                            long w = triangles[ta*3 + k];
                            if ( w < v )
                            {
                                SC3DMCTriplet id = {-1, -1, w};
                                unsigned long p = Insert(id, nPred, m_neighbors);
                                if (p != 0xFFFFFFFF)
                                {
                                    for (unsigned long i = 0; i < dimFloatArray; i++) 
                                    {
                                        m_neighbors[p].m_pred[i] = m_quantFloatArray[w*dimFloatArray+i];
                                    } 
                                }
                            }
                        }
                    }
                }
            }
            if (nPred > 1)
            {
#ifdef DEBUG_VERBOSE
                printf("\t\t vm %i\n", v);
                fprintf(g_fileDebugSC3DMCDec, "\t\t vm %i\n", v);
                for (unsigned long p = 0; p < nPred; ++p)
                {
                    printf("\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
                    fprintf(g_fileDebugSC3DMCDec, "\t\t pred a = %i b = %i c = %i \n", m_neighbors[p].m_id.m_a, m_neighbors[p].m_id.m_b, m_neighbors[p].m_id.m_c);
                    for (unsigned long i = 0; i < dimFloatArray; ++i) 
                    {
                        printf("\t\t\t %i\n", m_neighbors[p].m_pred[i]);
                        fprintf(g_fileDebugSC3DMCDec, "\t\t\t %i\n", m_neighbors[p].m_pred[i]);
                    }
                }
#endif //DEBUG_VERBOSE


                unsigned long bestPred;
                if (m_streamType == O3DGC_SC3DMC_STREAM_TYPE_ASCII)
                {
                    bestPred = bstream.ReadUCharASCII(iteratorPred);
                }
                else
                {
                    bestPred = acd.decode(mModelPreds);
                }
#ifdef DEBUG_VERBOSE
                    printf("best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
                    fprintf(g_fileDebugSC3DMCDec, "best (%i, %i, %i) \t pos %i\n", m_neighbors[bestPred].m_id.m_a, m_neighbors[bestPred].m_id.m_b, m_neighbors[bestPred].m_id.m_c, bestPred);
#endif //DEBUG_VERBOSE
                for (unsigned long i = 0; i < dimFloatArray; i++) 
                {
                    if (m_streamType == O3DGC_SC3DMC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    m_quantFloatArray[v*dimFloatArray+i] = predResidual + m_neighbors[bestPred].m_pred[i];
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i \t [%i]\n", v*dimFloatArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i \t [%i]\n", v*dimFloatArray+i, predResidual, m_neighbors[bestPred].m_pred[i]);
#endif //DEBUG_VERBOSE
                }
            }
            else if (v > 0 && predMode != O3DGC_SC3DMC_NO_PREDICTION)
            {
                for (unsigned long i = 0; i < dimFloatArray; i++) 
                {
                    if (m_streamType == O3DGC_SC3DMC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    m_quantFloatArray[v*dimFloatArray+i] = predResidual + m_quantFloatArray[(v-1)*dimFloatArray+i];
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", v*dimFloatArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i\n", v*dimFloatArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
            else
            {
                for (unsigned long i = 0; i < dimFloatArray; i++) 
                {
                    if (m_streamType == O3DGC_SC3DMC_STREAM_TYPE_ASCII)
                    {
                        predResidual = bstream.ReadUIntASCII(m_iterator);
                    }
                    else
                    {
                        predResidual = DecodeUIntACEGC(acd, mModelValues, bModel0, bModel1, exp_k, M);
                    }
                    m_quantFloatArray[v*dimFloatArray+i] = predResidual;
#ifdef DEBUG_VERBOSE
                    printf("%i \t %i\n", v*dimFloatArray+i, predResidual);
                    fprintf(g_fileDebugSC3DMCDec, "%i \t %i\n", v*dimFloatArray+i, predResidual);
#endif //DEBUG_VERBOSE
                }
            }
        }
        m_iterator  = iteratorPred;
        IQuantizeFloatArray(floatArray, numFloatArray, dimFloatArray, minFloatArray, maxFloatArray, nQBits);
#ifdef DEBUG_VERBOSE
        fflush(g_fileDebugSC3DMCDec);
#endif //DEBUG_VERBOSE
        return O3DGC_OK;
    }
    template<class T>
    O3DGCErrorCode SC3DMCDecoder<T>::IQuantizeFloatArray(Real * const floatArray, 
                                                      unsigned long numFloatArray,
                                                      unsigned long dimFloatArray,
                                                      const Real * const minFloatArray,
                                                      const Real * const maxFloatArray,
                                                      unsigned long nQBits)
    {
        
        Real idelta[O3DGC_SC3DMC_MAX_DIM_FLOAT_ATTRIBUTES];
        Real r;
        for(unsigned long d = 0; d < dimFloatArray; d++)
        {
            r = maxFloatArray[d] - minFloatArray[d];
            if (r > 0.0f)
            {
                idelta[d] = r/(float)((1 << nQBits) - 1);
            }
            else 
            {
                idelta[d] = 1.0f;
            }
        }        
        for(unsigned long v = 0; v < numFloatArray; ++v)
        {
            for(unsigned long d = 0; d < dimFloatArray; ++d)
            {
                floatArray[v * dimFloatArray + d] = m_quantFloatArray[v * dimFloatArray + d] * idelta[d] + minFloatArray[d];
            }
        }
        return O3DGC_OK;
    }
}
#endif // O3DGC_SC3DMC_DECODER_INL


