/************************************************************************
COPYRIGHT (C) ST Microelectronics 2009

Source file name : frame_parser_audio_avs.h
Author :           Andy

Definition of the frame parser audio AVS class implementation for player 2.


Date        Modification                                    Name
----        ------------                                    --------
30-Mar-07   Created (from frame_parser_video_mpeg2.h)       Daniel

************************************************************************/

#ifndef H_FRAME_PARSER_AUDIO_AVS
#define H_FRAME_PARSER_AUDIO_AVS

// /////////////////////////////////////////////////////////////////////
//
//	Include any component headers

#include "avs_audio.h"
#include "frame_parser_audio.h"

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined structures
//

// /////////////////////////////////////////////////////////////////////////
//
// The class definition
//

class FrameParser_AudioAvs_c : public FrameParser_Audio_c
{
private:

    // Data
    
    AvsAudioParsedFrameHeader_t  ParsedFrameHeader;
    
    AvsAudioStreamParameters_t  *StreamParameters;
    AvsAudioStreamParameters_t   CurrentStreamParameters;
    AvsAudioFrameParameters_t   *FrameParameters;

    // Functions

public:

    //
    // Constructor function
    //

    FrameParser_AudioAvs_c( void );
    ~FrameParser_AudioAvs_c( void );

    //
    // Overrides for component base class functions
    //

    FrameParserStatus_t   Reset( void );

    //
    // FrameParser class functions
    //

    FrameParserStatus_t   RegisterOutputBufferRing( Ring_t Ring );

    //
    // Stream specific functions
    //

    FrameParserStatus_t   ReadHeaders( void );
    FrameParserStatus_t   ResetReferenceFrameList( void );
    FrameParserStatus_t   PurgeQueuedPostDecodeParameterSettings( void );
    FrameParserStatus_t   PrepareReferenceFrameList( void );
    FrameParserStatus_t   ProcessQueuedPostDecodeParameterSettings( void );
    FrameParserStatus_t   GeneratePostDecodeParameterSettings( void );
    FrameParserStatus_t   UpdateReferenceFrameList( void );

    FrameParserStatus_t   ProcessReverseDecodeUnsatisfiedReferenceStack(void );
    FrameParserStatus_t   ProcessReverseDecodeStack( void );
    FrameParserStatus_t   PurgeReverseDecodeUnsatisfiedReferenceStack( void );
    FrameParserStatus_t   PurgeReverseDecodeStack( void );
    FrameParserStatus_t   TestForTrickModeFrameDrop( void );

    static FrameParserStatus_t ParseFrameHeader( unsigned char *FrameHeader, AvsAudioParsedFrameHeader_t *ParsedFrameHeader );
    static FrameParserStatus_t ParseExtensionHeader( unsigned char *ExtensionHeader, unsigned int *ExtensionLength );
};

#endif /* H_FRAME_PARSER_AUDIO_AVS */

