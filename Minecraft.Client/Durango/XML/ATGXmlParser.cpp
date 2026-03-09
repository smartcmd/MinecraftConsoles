// 4J-PB -
// The ATG Framework is a common set of C++ class libraries that is used by the samples in the XDK, and was developed by the Advanced Technology Group (ATG).
// The ATG Framework offers a clean and consistent format for the samples. These classes define functions used by all the samples.
// The ATG Framework together with the samples demonstrates best practices and innovative techniques for Xbox 360. There are many useful sections of code in the samples.
// You are encouraged to incorporate this code into your titles.


//-------------------------------------------------------------------------------------
//  AtgXmlParser.cpp
//
//  Simple callback non-validating XML parser implementation.
//
//  Xbox Advanced Technology Group.
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#include "stdafx.h"
#include "AtgXmlParser.h"

namespace ATG
{

//-------------------------------------------------------------------------------------
// Name: XMLParser::XMLParser
//-------------------------------------------------------------------------------------
XMLParser::XMLParser()
{
    m_pWritePtr = m_pWriteBuf;
    m_pReadPtr = m_pReadBuf;
    m_pISAXCallback = nullptr;
    m_hFile = INVALID_HANDLE_VALUE;
}

//-------------------------------------------------------------------------------------
// Name: XMLParser::~XMLParser
//-------------------------------------------------------------------------------------
XMLParser::~XMLParser()
{
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::FillBuffer
// Desc: Reads a block from the current open file
//-------------------------------------------------------------------------------------
VOID XMLParser::FillBuffer()
{
    DWORD NChars;

    m_pReadPtr = m_pReadBuf;

    if( m_hFile == nullptr )
    {
        if( m_uInXMLBufferCharsLeft > XML_READ_BUFFER_SIZE )
            NChars = XML_READ_BUFFER_SIZE;
        else
            NChars = m_uInXMLBufferCharsLeft;

        CopyMemory( m_pReadBuf, m_pInXMLBuffer, NChars );
        m_uInXMLBufferCharsLeft -= NChars;
        m_pInXMLBuffer += NChars;
    }
    else
    {
        if( !ReadFile( m_hFile, m_pReadBuf, XML_READ_BUFFER_SIZE, &NChars, nullptr ))
        {
            return;
        }
    }

    m_dwCharsConsumed += NChars;
    int64_t iProgress = m_dwCharsTotal ? (( static_cast<int64_t>(m_dwCharsConsumed) * 1000 ) / static_cast<int64_t>(m_dwCharsTotal)) : 0;
    m_pISAXCallback->SetParseProgress( static_cast<DWORD>(iProgress) );

    m_pReadBuf[ NChars ] = '\0';
    m_pReadBuf[ NChars + 1] = '\0';
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::SkipNextAdvance
// Desc: Puts the last character read back on the input stream
//-------------------------------------------------------------------------------------
VOID XMLParser::SkipNextAdvance()
{
    m_bSkipNextAdvance = TRUE;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::ConsumeSpace
// Desc: Skips spaces in the current stream
//-------------------------------------------------------------------------------------
HRESULT XMLParser::ConsumeSpace()
{
    HRESULT hr;

    // Skip spaces
    if( FAILED( hr = AdvanceCharacter() ) )
        return hr;

    while ( ( m_Ch == ' ' ) || ( m_Ch == '\t' ) ||
            ( m_Ch == '\n' ) || ( m_Ch == '\r' ) )
    {
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
    }
    SkipNextAdvance();
    return S_OK;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::ConvertEscape
// Desc: Copies and converts an escape sequence into m_pWriteBuf
//-------------------------------------------------------------------------------------
HRESULT XMLParser::ConvertEscape()
{
    HRESULT hr;
    WCHAR wVal = 0;

    if( FAILED( hr = AdvanceCharacter() ) )
        return hr;

    // all escape sequences start with &, so ignore the first character

    if( FAILED( hr = AdvanceCharacter() ) )
        return hr;

    if ( m_Ch == '#' )     // character as hex or decimal
    {
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        if ( m_Ch == 'x' )     // hex number
        {
            if( FAILED( hr = AdvanceCharacter() ) )
                return hr;

            while ( m_Ch != ';' )
            {
                wVal *= 16;

                if ( ( m_Ch >= '0' ) && ( m_Ch <= '9' ) )
                {
                    wVal += m_Ch - '0';
                }
                else if ( ( m_Ch >= 'a' ) && ( m_Ch <= 'f' ) )
                {
                    wVal += m_Ch - 'a' + 10;
                }
                else if ( ( m_Ch >= 'A' ) && ( m_Ch <= 'F' ) )
                {
                    wVal += m_Ch - 'A' + 10;
                }
                else
                {
                    Error( E_INVALID_XML_SYNTAX, "Expected hex digit as part of &#x escape sequence" );
                    return E_INVALID_XML_SYNTAX;
                }

                if( FAILED( hr = AdvanceCharacter() ) )
                    return hr;
            }
        }
        else                    // decimal number
        {
            while ( m_Ch != ';' )
            {
                wVal *= 10;

                if ( ( m_Ch >= '0' ) && ( m_Ch <= '9' ) )
                {
                    wVal += m_Ch - '0';
                }
                else
                {
                    Error( E_INVALID_XML_SYNTAX, "Expected decimal digit as part of &# escape sequence" );
                    return E_INVALID_XML_SYNTAX;
                }

                if( FAILED( hr = AdvanceCharacter() ) )
                    return hr;
            }
        }

        // copy character into the buffer
        m_Ch = wVal;

        return S_OK;
    }

    // must be an entity reference

    WCHAR *pEntityRefVal = m_pWritePtr;
    UINT EntityRefLen;

    SkipNextAdvance();
    if( FAILED( hr = AdvanceName() ) )
        return hr;

    EntityRefLen = static_cast<UINT>(m_pWritePtr - pEntityRefVal);
    m_pWritePtr = pEntityRefVal;

    if ( EntityRefLen == 0 )
    {
        Error( E_INVALID_XML_SYNTAX, "Expecting entity name after &" );
        return E_INVALID_XML_SYNTAX;
    }

    if( !wcsncmp( pEntityRefVal, L"lt", EntityRefLen ) )
        wVal = '<';
    else if( !wcsncmp( pEntityRefVal, L"gt", EntityRefLen ) )
        wVal = '>';
    else if( !wcsncmp( pEntityRefVal, L"amp", EntityRefLen ) )
        wVal = '&';
    else if( !wcsncmp( pEntityRefVal, L"apos", EntityRefLen ) )
        wVal = '\'';
    else if( !wcsncmp( pEntityRefVal, L"quot", EntityRefLen ) )
        wVal = '"';
    else
    {
        Error( E_INVALID_XML_SYNTAX, "Unrecognized entity name after & - (should be lt, gt, amp, apos, or quot)" );
        return E_INVALID_XML_SYNTAX;   // return false if unrecognized token sequence
    }

    if( FAILED( hr = AdvanceCharacter() ) )
        return hr;

    if( m_Ch != ';' )
    {
        Error( E_INVALID_XML_SYNTAX, "Expected terminating ; for entity reference" );
        return E_INVALID_XML_SYNTAX;   // malformed reference - needs terminating ;
    }

    m_Ch = wVal;
    return S_OK;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::AdvanceAttrVal
// Desc: Copies an attribute value into m_pWrite buf, skipping surrounding quotes
//-------------------------------------------------------------------------------------
HRESULT XMLParser::AdvanceAttrVal()
{
    HRESULT hr;
    WCHAR wQuoteChar;

    if( FAILED( hr = AdvanceCharacter() ) )
        return hr;

    if( ( m_Ch != '"' ) && ( m_Ch != '\'' ) )
    {
        Error( E_INVALID_XML_SYNTAX, "Attribute values must be enclosed in quotes" );
        return E_INVALID_XML_SYNTAX;
    }

    wQuoteChar = m_Ch;

    for( ;; )
    {
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        else if( m_Ch == wQuoteChar )
            break;
        else if( m_Ch == '&' )
        {
            SkipNextAdvance();
            if( FAILED( hr = ConvertEscape() ) )
                return hr;
        }
        else if( m_Ch == '<' )
        {
            Error( E_INVALID_XML_SYNTAX, "Illegal character '<' in element tag" );
            return E_INVALID_XML_SYNTAX;
        }

        // copy character into the buffer

        if( m_pWritePtr - m_pWriteBuf >= XML_WRITE_BUFFER_SIZE )
        {
            Error( E_INVALID_XML_SYNTAX, "Total element tag size may not be more than %d characters", XML_WRITE_BUFFER_SIZE );
            return E_INVALID_XML_SYNTAX;
        }

        *m_pWritePtr = m_Ch;
        m_pWritePtr++;
    }
    return S_OK;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::AdvanceName
// Desc: Copies a name into the m_pWriteBuf - returns TRUE on success, FALSE on failure
//       Ignores leading whitespace.  Currently does not support unicode names
//-------------------------------------------------------------------------------------
HRESULT XMLParser::AdvanceName()
{
    HRESULT hr;

    if( FAILED( hr = AdvanceCharacter() ) )
        return hr;

    if( ( ( m_Ch < 'A' ) || ( m_Ch > 'Z' ) ) &&
        ( ( m_Ch < 'a' ) || ( m_Ch > 'z' ) ) &&
        ( m_Ch != '_' ) && ( m_Ch != ':' ) )
    {
        Error( E_INVALID_XML_SYNTAX, "Names must start with an alphabetic character or _ or :" );
        return E_INVALID_XML_SYNTAX;
    }

    while( ( ( m_Ch >= 'A' ) && ( m_Ch <= 'Z' ) ) ||
           ( ( m_Ch >= 'a' ) && ( m_Ch <= 'z' ) ) ||
           ( ( m_Ch >= '0' ) && ( m_Ch <= '9' ) ) ||
           ( m_Ch == '_' ) || ( m_Ch == ':' ) ||
           ( m_Ch == '-' ) || ( m_Ch == '.' ) )
    {

        if( m_pWritePtr - m_pWriteBuf >= XML_WRITE_BUFFER_SIZE )
        {
            Error( E_INVALID_XML_SYNTAX, "Total element tag size may not be more than %d characters", XML_WRITE_BUFFER_SIZE );
            return E_INVALID_XML_SYNTAX;
        }

        *m_pWritePtr = m_Ch;
        m_pWritePtr++;

        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
    }

    SkipNextAdvance();
    return S_OK;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::AdvanceCharacter
// Desc: Copies the character at *m_pReadPtr to m_Ch
//       handling difference in UTF16 / UTF8, and big/little endian
//       and getting another chunk of the file if needed
//       Returns S_OK if there are more characters, E_ABORT for no characters to read
//-------------------------------------------------------------------------------------
HRESULT XMLParser::AdvanceCharacter( BOOL bOkToFail )
{
    if( m_bSkipNextAdvance )
    {
        m_bSkipNextAdvance = FALSE;
        return S_OK;
    }

    // If we hit EOF in the middle of a character,
    // it's ok-- we'll just have a corrupt last character
    // (the buffer is padded with double NULLs )

    if ( ( m_pReadPtr[0] == '\0' ) && ( m_pReadPtr[1] == '\0' ) )
    {
        // Read more from the file
        FillBuffer();

        // We are at EOF if it is still nullptr
        if ( ( m_pReadPtr[0] == '\0' ) && ( m_pReadPtr[1] == '\0' ) )
        {
            if( !bOkToFail )
            {
                Error( E_INVALID_XML_SYNTAX, "Unexpected EOF while parsing XML file" );
                return E_INVALID_XML_SYNTAX;
            }
            else
            {
                return E_FAIL;
            }
        }
    }

    if( m_bUnicode == FALSE )
    {
        m_Ch = *((CHAR *)m_pReadPtr);
        m_pReadPtr++;
    }
    else // if( m_bUnicode == TRUE )
    {
        m_Ch = *((WCHAR *)m_pReadPtr);

        if( m_bReverseBytes )
        {
            m_Ch = ( m_Ch << 8 ) + ( m_Ch >> 8 );
        }

        m_pReadPtr += 2;
    }

    if( m_Ch == '\n' )
    {
        m_pISAXCallback->m_LineNum++;
        m_pISAXCallback->m_LinePos = 0;
    }
    else if( m_Ch != '\r' )
        m_pISAXCallback->m_LinePos++;

    return S_OK;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::AdvanceElement
// Desc: Builds <element> data, calls callback
//-------------------------------------------------------------------------------------
HRESULT XMLParser::AdvanceElement()
{
    HRESULT hr;

    // write ptr at the beginning of the buffer
    m_pWritePtr = m_pWriteBuf;

    if( FAILED( hr = AdvanceCharacter() ) )
        return hr;

    // if first character wasn't '<', we wouldn't be here

    if( FAILED( hr = AdvanceCharacter() ) )
        return hr;

    if( m_Ch == '!' )
    {
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        if ( m_Ch == '-' )
        {
            if( FAILED( hr = AdvanceCharacter() ) )
                return hr;
            if( m_Ch != '-' )
            {
                Error( E_INVALID_XML_SYNTAX, "Expecting '-' after '<!-'" );
                return E_INVALID_XML_SYNTAX;
            }
            if( FAILED( hr = AdvanceComment() ) )
                return hr;
            return S_OK;
        }

        if( m_Ch != '[' )
        {
            Error( E_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return E_INVALID_XML_SYNTAX;
        }
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        if( m_Ch != 'C' )
        {
            Error( E_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return E_INVALID_XML_SYNTAX;
        }
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        if( m_Ch != 'D' )
        {
            Error( E_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return E_INVALID_XML_SYNTAX;
        }
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        if( m_Ch != 'A' )
        {
            Error( E_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return E_INVALID_XML_SYNTAX;
        }
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        if( m_Ch != 'T' )
        {
            Error( E_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return E_INVALID_XML_SYNTAX;
        }
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        if( m_Ch != 'A' )
        {
            Error( E_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return E_INVALID_XML_SYNTAX;
        }
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;
        if( m_Ch != '[' )
        {
            Error( E_INVALID_XML_SYNTAX, "Expecting '<![CDATA['" );
            return E_INVALID_XML_SYNTAX;
        }
        if( FAILED( hr = AdvanceCDATA() ) )
            return hr;
    }
    else if( m_Ch == '/' )
    {
        WCHAR *pEntityRefVal = m_pWritePtr;

        if( FAILED( hr = AdvanceName() ) )
            return hr;

        if (FAILED(m_pISAXCallback->ElementEnd( pEntityRefVal,
            static_cast<UINT>(m_pWritePtr - pEntityRefVal) )))
            return E_ABORT;

        if( FAILED( hr = ConsumeSpace() ) )
            return hr;

        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;

        if( m_Ch != '>' )
        {
            Error( E_INVALID_XML_SYNTAX, "Expecting '>' after name for closing entity reference" );
            return E_INVALID_XML_SYNTAX;
        }
    }
    else if( m_Ch == '?' )
    {
        // just skip any xml header tag since not really important after identifying character set
        for( ;; )
        {
            if( FAILED( hr = AdvanceCharacter() ) )
                return hr;

            if ( m_Ch == '>' )
                return S_OK;
        }
    }
    else
    {
        XMLAttribute   Attributes[ XML_MAX_ATTRIBUTES_PER_ELEMENT ];
        UINT           NumAttrs;

        WCHAR *pEntityRefVal = m_pWritePtr;
        UINT  EntityRefLen;

        NumAttrs = 0;

        SkipNextAdvance();

        // Entity tag
        if( FAILED( hr = AdvanceName() ) )
            return hr;

        EntityRefLen = static_cast<UINT>(m_pWritePtr - pEntityRefVal);

        if( FAILED( hr = ConsumeSpace() ) )
            return hr;

        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;

        // read attributes
        while( ( m_Ch != '>' ) && ( m_Ch != '/' ) )
        {
            SkipNextAdvance();

            if ( NumAttrs >= XML_MAX_ATTRIBUTES_PER_ELEMENT )
            {
                 Error( E_INVALID_XML_SYNTAX, "Elements may not have more than %d attributes", XML_MAX_ATTRIBUTES_PER_ELEMENT );
                return E_INVALID_XML_SYNTAX;
            }

            Attributes[ NumAttrs ].strName = m_pWritePtr;

            // Attribute name
            if( FAILED( hr = AdvanceName() ) )
                return hr;

            Attributes[ NumAttrs ].NameLen = static_cast<UINT>(m_pWritePtr - Attributes[NumAttrs].strName);

            if( FAILED( hr = ConsumeSpace() ) )
                return hr;

            if( FAILED( hr = AdvanceCharacter() ) )
                return hr;

            if( m_Ch != '=' )
            {
                Error( E_INVALID_XML_SYNTAX, "Expecting '=' character after attribute name" );
                return E_INVALID_XML_SYNTAX;
            }

            if( FAILED( hr = ConsumeSpace() ) )
                return hr;

            Attributes[ NumAttrs ].strValue = m_pWritePtr;

            if( FAILED( hr = AdvanceAttrVal() ) )
                return hr;

            Attributes[ NumAttrs ].ValueLen = static_cast<UINT>(m_pWritePtr -
                                                                Attributes[NumAttrs].strValue);

            ++NumAttrs;

            if( FAILED( hr = ConsumeSpace() ) )
                return hr;

            if( FAILED( hr = AdvanceCharacter() ) )
                return hr;
        }

        if( m_Ch == '/' )
        {
            if( FAILED( hr = AdvanceCharacter() ) )
                return hr;
            if( m_Ch != '>' )
            {
                Error( E_INVALID_XML_SYNTAX, "Expecting '>' after '/' in element tag" );
                return E_INVALID_XML_SYNTAX;
            }

            if( FAILED( m_pISAXCallback->ElementBegin( pEntityRefVal, EntityRefLen,
                                                       Attributes, NumAttrs ) ) )
                return E_ABORT;

            if( FAILED( m_pISAXCallback->ElementEnd( pEntityRefVal, EntityRefLen ) ) )
                return E_ABORT;
        }
        else
        {
            if( FAILED( m_pISAXCallback->ElementBegin( pEntityRefVal, EntityRefLen,
                                                       Attributes, NumAttrs ) ) )
                return E_ABORT;
        }
    }

    return S_OK;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::AdvanceCDATA
// Desc: Read a CDATA section
//-------------------------------------------------------------------------------------
HRESULT XMLParser::AdvanceCDATA()
{
    HRESULT hr;
    WORD wStage = 0;

    if( FAILED( m_pISAXCallback->CDATABegin() ) )
        return E_ABORT;

    for( ;; )
    {
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;

        *m_pWritePtr = m_Ch;
        m_pWritePtr++;

        if( ( m_Ch == ']' ) && ( wStage == 0 ) )
            wStage = 1;
        else if( ( m_Ch == ']' ) && ( wStage == 1 ) )
            wStage = 2;
        else if( ( m_Ch == '>' ) && ( wStage == 2 ) )
        {
            m_pWritePtr -= 3;
            break;
        }
        else
            wStage = 0;

        if( m_pWritePtr - m_pWriteBuf >= XML_WRITE_BUFFER_SIZE )
        {
            if( FAILED( m_pISAXCallback->CDATAData( m_pWriteBuf, static_cast<UINT>(m_pWritePtr - m_pWriteBuf), TRUE ) ) )
                return E_ABORT;
            m_pWritePtr = m_pWriteBuf;
        }
    }

    if( FAILED( m_pISAXCallback->CDATAData( m_pWriteBuf, static_cast<UINT>(m_pWritePtr - m_pWriteBuf), FALSE ) ) )
        return E_ABORT;

    m_pWritePtr = m_pWriteBuf;

    if( FAILED( m_pISAXCallback->CDATAEnd() ) )
        return E_ABORT;

    return S_OK;
}

//-------------------------------------------------------------------------------------
// Name: XMLParser::AdvanceComment
// Desk: Skips over a comment
//-------------------------------------------------------------------------------------
HRESULT XMLParser::AdvanceComment()
{
    HRESULT hr;
    WORD wStage;

    wStage = 0;
    for( ;; )
    {
        if( FAILED( hr = AdvanceCharacter() ) )
            return hr;

        if (( m_Ch == '-' ) && ( wStage == 0 ))
            wStage = 1;
        else if (( m_Ch == '-' ) && ( wStage == 1 ))
            wStage = 2;
        else if (( m_Ch == '>' ) && ( wStage == 2 ))
            break;
        else
            wStage = 0;
    }

    return S_OK;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::RegisterSAXCallbackInterface
// Desc: Registers callback interface
//-------------------------------------------------------------------------------------
VOID XMLParser::RegisterSAXCallbackInterface( ISAXCallback *pISAXCallback )
{
    m_pISAXCallback = pISAXCallback;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::GetSAXCallbackInterface
// Desc: Returns current callback interface
//-------------------------------------------------------------------------------------
ISAXCallback* XMLParser::GetSAXCallbackInterface()
{
    return m_pISAXCallback;
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::MainParseLoop
// Desc: Main Loop to Parse Data - source agnostic
//-------------------------------------------------------------------------------------
HRESULT XMLParser::MainParseLoop()
{
    BOOL bWhiteSpaceOnly = TRUE;
    HRESULT hr = S_OK;

    if( FAILED( m_pISAXCallback->StartDocument() ) )
        return E_ABORT;

    m_pWritePtr = m_pWriteBuf;

    FillBuffer();

    if ( *((WCHAR *) m_pReadBuf ) == 0xFEFF )
    {
        m_bUnicode = TRUE;
        m_bReverseBytes = FALSE;
        m_pReadPtr += 2;
    }
    else if ( *((WCHAR *) m_pReadBuf ) == 0xFFFE )
    {
        m_bUnicode = TRUE;
        m_bReverseBytes = TRUE;
        m_pReadPtr += 2;
    }
    else if ( *((WCHAR *) m_pReadBuf ) == 0x003C )
    {
        m_bUnicode = TRUE;
        m_bReverseBytes = FALSE;
    }
    else if ( *((WCHAR *) m_pReadBuf ) == 0x3C00 )
    {
        m_bUnicode = TRUE;
        m_bReverseBytes = TRUE;
    }
    else if ( m_pReadBuf[ 0 ] == 0x3C )
    {
        m_bUnicode = FALSE;
        m_bReverseBytes = FALSE;
    }
    else
    {
        Error( E_INVALID_XML_SYNTAX, "Unrecognized encoding (parser does not support UTF-8 language encodings)" );
        return E_INVALID_XML_SYNTAX;
    }

    for( ;; )
    {
        if( FAILED( AdvanceCharacter( TRUE ) ) )
        {
            if ( ( static_cast<UINT>(m_pWritePtr - m_pWriteBuf) != 0 ) && ( !bWhiteSpaceOnly ) )
            {
                if( FAILED( m_pISAXCallback->ElementContent( m_pWriteBuf, static_cast<UINT>(m_pWritePtr - m_pWriteBuf), FALSE ) ) )
                    return E_ABORT;
                bWhiteSpaceOnly = TRUE;
            }

            if( FAILED( m_pISAXCallback->EndDocument() ) )
                return E_ABORT;

            return S_OK;
        }

        if( m_Ch == '<' )
        {
\
            if( ( static_cast<UINT>(m_pWritePtr - m_pWriteBuf) != 0 ) && ( !bWhiteSpaceOnly ) )
            {
                if( FAILED( m_pISAXCallback->ElementContent( m_pWriteBuf, static_cast<UINT>(m_pWritePtr - m_pWriteBuf), FALSE ) ) )
                    return E_ABORT;

                bWhiteSpaceOnly = TRUE;
            }

            SkipNextAdvance();

            m_pWritePtr = m_pWriteBuf;

            if( FAILED( hr = AdvanceElement() ) )
                return hr;

            m_pWritePtr = m_pWriteBuf;
        }
        else
        {
            if( m_Ch == '&' )
            {
                SkipNextAdvance();
                if( FAILED( hr = ConvertEscape() ) )
                    return hr;
            }

            if( bWhiteSpaceOnly && ( m_Ch != ' ' ) && ( m_Ch != '\n' ) && ( m_Ch != '\r' ) &&
                                    ( m_Ch != '\t' ) )
            {
                bWhiteSpaceOnly = FALSE;
            }

            *m_pWritePtr = m_Ch;
            m_pWritePtr++;

            if( m_pWritePtr - m_pWriteBuf >= XML_WRITE_BUFFER_SIZE )
            {
                if( !bWhiteSpaceOnly )
                {
                    if( FAILED( m_pISAXCallback->ElementContent( m_pWriteBuf,
                        static_cast<UINT>(m_pWritePtr - m_pWriteBuf), TRUE ) ) )
                    {
                        return E_ABORT;
                    }
                }

                m_pWritePtr = m_pWriteBuf;
                bWhiteSpaceOnly = TRUE;
            }
        }
    }
}


//-------------------------------------------------------------------------------------
// Name: XMLParser::ParseXMLFile
// Desc: Builds element data
//-------------------------------------------------------------------------------------
HRESULT XMLParser::ParseXMLFile( CONST CHAR *strFilename )
{
    HRESULT hr;

    if( m_pISAXCallback == nullptr )
        return E_NOINTERFACE;

    m_pISAXCallback->m_LineNum = 1;
    m_pISAXCallback->m_LinePos = 0;
    m_pISAXCallback->m_strFilename = strFilename;  // save this off only while we parse the file

    m_bSkipNextAdvance = FALSE;
    m_pReadPtr = m_pReadBuf;

    m_pReadBuf[ 0 ] = '\0';

    m_pReadBuf[ 1 ] = '\0';

    m_pInXMLBuffer = nullptr;
    m_uInXMLBufferCharsLeft = 0;

	WCHAR wchFilename[ 64 ];

	swprintf_s(wchFilename,64,L"%s",strFilename);

    m_hFile = CreateFile(wchFilename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

    if( m_hFile == INVALID_HANDLE_VALUE )
    {
        Error( E_COULD_NOT_OPEN_FILE, "Error opening file" );
        hr = E_COULD_NOT_OPEN_FILE;

    }
    else
    {
        LARGE_INTEGER iFileSize;
        GetFileSizeEx( m_hFile, &iFileSize );
        m_dwCharsTotal = static_cast<DWORD>(iFileSize.QuadPart);
        m_dwCharsConsumed = 0;
        hr = MainParseLoop();
    }

    // Close the file
    if( m_hFile != INVALID_HANDLE_VALUE )
        CloseHandle( m_hFile );
    m_hFile = INVALID_HANDLE_VALUE;

    // we no longer own strFilename, so un-set it
    m_pISAXCallback->m_strFilename = nullptr;  
    return hr;
}

//-------------------------------------------------------------------------------------
// Name: XMLParser::ParseXMLFile
// Desc: Builds element data
//-------------------------------------------------------------------------------------
HRESULT XMLParser::ParseXMLBuffer( CONST CHAR *strBuffer, UINT uBufferSize )
{
    HRESULT hr;

    if( m_pISAXCallback == nullptr )
        return E_NOINTERFACE;

    m_pISAXCallback->m_LineNum = 1;
    m_pISAXCallback->m_LinePos = 0;
    m_pISAXCallback->m_strFilename = "";  // save this off only while we parse the file

    m_bSkipNextAdvance = FALSE;
    m_pReadPtr = m_pReadBuf;

    m_pReadBuf[ 0 ] = '\0';
    m_pReadBuf[ 1 ] = '\0';

    m_hFile = nullptr;
    m_pInXMLBuffer = strBuffer;
    m_uInXMLBufferCharsLeft = uBufferSize;
    m_dwCharsTotal = uBufferSize;
    m_dwCharsConsumed = 0;

    hr = MainParseLoop();

    // we no longer own strFilename, so un-set it
    m_pISAXCallback->m_strFilename = nullptr;  

    return hr;
}

//-------------------------------------------------------------------------------------
// XMLParser::Error()
//      Logs an error through the callback interface
//-------------------------------------------------------------------------------------
#ifdef  _Printf_format_string_  // VC++ 2008 and later support this annotation
VOID XMLParser::Error( HRESULT hErr, _In_z_ _Printf_format_string_ CONST CHAR* strFormat, ... )
#else
VOID XMLParser::Error( HRESULT hErr, CONST CHAR* strFormat, ... )
#endif
{
    CONST INT MAX_OUTPUT_STR = 160;
    CHAR strBuffer[ MAX_OUTPUT_STR ];
    va_list pArglist;
    va_start( pArglist, strFormat );

    vsprintf( strBuffer, strFormat, pArglist );

    m_pISAXCallback->Error( hErr, strBuffer );
    va_end( pArglist );
}

} // namespace ATG
