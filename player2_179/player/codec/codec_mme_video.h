/************************************************************************
COPYRIGHT (C) SGS-THOMSON Microelectronics 2007

Source file name : codec_mme_video.h
Author :           Nick

Definition of the codec video class implementation for player 2.


Date        Modification                                    Name
----        ------------                                    --------
20-Jan-07   Created                                         Nick

************************************************************************/

#ifndef H_CODEC_MME_VIDEO
#define H_CODEC_MME_VIDEO

// /////////////////////////////////////////////////////////////////////
//
//      Include any component headers

#include "VideoCompanion.h"
#include "codec_mme_base.h"

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined constants
//

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined structures
//

// /////////////////////////////////////////////////////////////////////////
//
// The class definition
//

class Codec_MmeVideo_c : public Codec_MmeBase_c
{
protected:

    // Data

    VideoOutputSurfaceDescriptor_t       *VideoOutputSurface;

    ParsedVideoParameters_t              *ParsedVideoParameters;
    bool                                  KnownLastSliceInFieldFrame;

    // Functions

public:

    //
    // Overrides for component base class functions
    //

    CodecStatus_t   Halt(       void );
    CodecStatus_t   Reset(      void );

    //
    // Codec class functions
    //

    CodecStatus_t   RegisterOutputBufferRing(   Ring_t            Ring );
    CodecStatus_t   Input(                      Buffer_t          CodedBuffer );

    //
    // Extension to base functions
    //

    CodecStatus_t   InitializeDataTypes(        void );

    //
    // Implementation of fill out function for generic video,
    // may be overidden if necessary.
    //

    virtual CodecStatus_t   FillOutDecodeBufferRequest(	BufferStructure_t	 *Request );
};
#endif
