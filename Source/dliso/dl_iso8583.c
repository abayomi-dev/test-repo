/******************************************************************************/
/*                                                                            */
/* Copyright (C) 2005-2007 Oscar Sanderson                                    */
/*                                                                            */
/* This software is provided 'as-is', without any express or implied          */
/* warranty.  In no event will the author(s) be held liable for any damages   */
/* arising from the use of this software.                                     */
/*                                                                            */
/* Permission is granted to anyone to use this software for any purpose,      */
/* including commercial applications, and to alter it and redistribute it     */
/* freely, subject to the following restrictions:                             */
/*                                                                            */
/* 1. The origin of this software must not be misrepresented; you must not    */
/*    claim that you wrote the original software. If you use this software    */
/*    in a product, an acknowledgment in the product documentation would be   */
/*    appreciated but is not required.                                        */
/*                                                                            */
/* 2. Altered source versions must be plainly marked as such, and must not be */
/*    misrepresented as being the original software.                          */
/*                                                                            */
/* 3. This notice may not be removed or altered from any source distribution. */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* dl_iso8583.c - An implementation of the ISO-8583 message protocol          */
/*                                                                            */
/******************************************************************************/

#include "dl_iso8583.h"
//#include "../../upsl-pos/pal/Pal.h"
/******************************************************************************/

void DL_ISO8583_MSG_Init ( DL_UINT8       *_iStaticBuf,
				           DL_UINT16       _iStaticBufSize,
				           DL_ISO8583_MSG *ioMsg )
{
	/* init structure */
	DL_MEM_memset((void*)ioMsg,0,sizeof(DL_ISO8583_MSG));

	/* set mode to 'static' if required */
	if ( _iStaticBuf )
	{
		ioMsg->sPtrNext = _iStaticBuf;
		ioMsg->sPtrEnd  = (ioMsg->sPtrNext) + _iStaticBufSize;
	}

	return;
}

/******************************************************************************/

void DL_ISO8583_MSG_Free ( DL_ISO8583_MSG *ioMsg )
{
	if ( NULL == ioMsg->sPtrNext ) /* dynamic mode */
	{
		int i;

		for ( i=0; i<=kDL_ISO8583_MAX_FIELD_IDX ; i++ )
		{
			/* free memory (if allocated) */
			DL_MEM_free(ioMsg->field[i].ptr);
		} /* end-for(i) */
	}

	/* init structure */
	DL_MEM_memset((void*)ioMsg,0,sizeof(DL_ISO8583_MSG));

	return;
}

/******************************************************************************/

// allocates and sets the specified (string) field
// NB iField range is 0..kDL_ISO8583_MAX_FIELD_IDX
// returns: error code
DL_ERR DL_ISO8583_MSG_SetField_Str ( DL_UINT16       iField,
					                 const DL_UINT8 *iDataStr,
						             DL_ISO8583_MSG *ioMsg )
{
	DL_ERR err = kDL_ERR_NONE;
	//ShowLogs(1, "EOL: Field: %d", iField);
	//ShowLogs(1, "EOL: Content: %s", iDataStr);
	err = DL_ISO8583_MSG_SetField_Bin(iField,
									  iDataStr,
									  (DL_UINT16)DL_STR_StrLen(iDataStr),
									  ioMsg);
	//ShowLogs(1, "EOL: Return: %i", err);
	return err;
}

/******************************************************************************/

// allocates and sets the specified field
// NB iField range is 0..kDL_ISO8583_MAX_FIELD_IDX
// NB iDataLen indicates the length of the data (0..n)
// NB iData can be NULL if iDataLen is 0
// returns: error code
DL_ERR DL_ISO8583_MSG_SetField_Bin ( DL_UINT16       iField,
					                 const DL_UINT8 *_iData,
					                 DL_UINT16       _iDataLen,
					                 DL_ISO8583_MSG *ioMsg )
{
	DL_ERR    err = kDL_ERR_NONE;
	DL_UINT8 *ptr = NULL;

	if ( iField > kDL_ISO8583_MAX_FIELD_IDX )
	{
		//ShowLogs(1, "DL_ISO8583_MSG_SetField_Bin Error 1");
		return kDL_ERR_OTHER;
	}

	/* allocate memory for the field content */
	err = _DL_ISO8583_MSG_AllocField(iField,_iDataLen,ioMsg,&ptr);
	//ShowLogs(1, "_DL_ISO8583_MSG_AllocField Ret: %i", err);

	if ( !err )
	//if(1)
	{
		//ShowLogs(1, "DL_ISO8583_MSG_SetField_Bin NULL TERMINATE");
		DL_MEM_memcpy(ptr,_iData,_iDataLen);
		/* null terminate */
		ptr[_iDataLen] = kDL_ASCII_NULL;
	}
	//ShowLogs(1, "DL_ISO8583_MSG_SetField_Bin End: %i", err);
	return err;
}

/******************************************************************************/

// NB iField range is 0..kDL_ISO8583_MAX_FIELD_IDX
// returns: 1 if the field is set / 0 otherwise
int DL_ISO8583_MSG_HaveField ( DL_UINT16             iField,
					           const DL_ISO8583_MSG *iMsg )
{
	if ( (NULL != iMsg) &&
		 (iField <= kDL_ISO8583_MAX_FIELD_IDX) &&
		 (NULL != iMsg->field[iField].ptr) )
	{
		return 1;
	}

	return 0;
}

/******************************************************************************/

// NB iField range is 0..kDL_ISO8583_MAX_FIELD_IDX
// outputs: oPtr - static pointer to field data
// returns: error code
DL_ERR DL_ISO8583_MSG_GetField_Str ( DL_UINT16              iField,
							         const DL_ISO8583_MSG  *iMsg,
									 DL_UINT8             **oPtr )
{
	DL_ERR err = kDL_ERR_OTHER; /* assume error */

	/* init outputs */
	*oPtr = NULL;

	if ( (NULL != iMsg) &&
		 (iField <= kDL_ISO8583_MAX_FIELD_IDX) &&
		 (NULL != iMsg->field[iField].ptr) )
	{
		err   = kDL_ERR_NONE;
		*oPtr = iMsg->field[iField].ptr;
	}

	return err;
}

/******************************************************************************/

// NB iField range is 0..kDL_ISO8583_MAX_FIELD_IDX
// outputs: oPtr     - static pointer to field data
//			oByteLen - byte length of field data
// returns: error code
DL_ERR DL_ISO8583_MSG_GetField_Bin ( DL_UINT16              iField,
								     const DL_ISO8583_MSG  *iMsg,
									 DL_UINT8             **oPtr,
									 DL_UINT16             *oByteLen )
{
	DL_ERR err = kDL_ERR_OTHER; /* assume error */

	/* init outputs */
	*oPtr     = NULL;
	*oByteLen = 0;

	if ( (NULL != iMsg) &&
		 (iField <= kDL_ISO8583_MAX_FIELD_IDX) &&
		 (NULL != iMsg->field[iField].ptr) )
	{
		err       = kDL_ERR_NONE;
		*oPtr     = iMsg->field[iField].ptr;
		*oByteLen = iMsg->field[iField].len;
	}

	return err;
}

/******************************************************************************/

DL_ERR DL_ISO8583_MSG_Pack ( const DL_ISO8583_HANDLER *iHandler,
					         const DL_ISO8583_MSG     *iMsg,
		                     DL_UINT8                 *ioByteArr,
			                 DL_UINT16                *oNumBytes )
{
	DL_ERR     err       = kDL_ERR_NONE;
	DL_UINT8  *curPtr    = ioByteArr;
	DL_UINT16  fieldIdx;

	/* init outputs */
	*oNumBytes = 0;

	/* pack any fields that are present */
	for ( fieldIdx=0 ; (fieldIdx<MIN(iHandler->fieldItems,kDL_ISO8583_MAX_FIELD_IDX+1)) && !err ; fieldIdx++ )
	{
		if ( (NULL != iMsg->field[fieldIdx].ptr) || // present
			 DL_ISO8583_IS_BITMAP(iHandler->fieldArr[fieldIdx].fieldType) ) // bitmap
		{
			/* pack field */
			err = _DL_ISO8583_FIELD_Pack(fieldIdx,iMsg,iHandler,&curPtr);
			if(err)
			{
				ShowLogs(1, "Field with error %d\n", fieldIdx);
				ShowLogs(1, "ERROR FROM ISO PACK: %d\n", err);
				//LogMessage(LogVerbose, "Field with error %d", fieldIdx);
				//LogMessage(LogVerbose, "ERROR FROM ISO PACK: %d", err);
			}
		}
	} /* end-for(i) */

	if ( !err )
	{
		/* determine the number of bytes output */
		*oNumBytes = (int)(curPtr - ioByteArr);
	}
	ShowLogs(1, "DL_ISO8583_MSG_Pack: %i", err);
	return err;
}

/******************************************************************************/

DL_ERR DL_ISO8583_MSG_Unpack ( const DL_ISO8583_HANDLER *iHandler,
					           const DL_UINT8           *iByteArr,
			                   DL_UINT16                 iByteArrSize,
			                   DL_ISO8583_MSG           *ioMsg )
{
	DL_ERR     err         = kDL_ERR_NONE;
	DL_UINT8  *curPtr      = (DL_UINT8*)iByteArr;
	DL_UINT8  *endPtr      = curPtr + iByteArrSize;
	DL_UINT16  curFieldIdx = 0;
	DL_UINT16  maxFieldIdx = MIN(kDL_ISO8583_MAX_FIELD_IDX,(iHandler->fieldItems)-1);
	int        haveBitmap  = 0;

	/* unpack all fields until we've encountered a bitmap field */
	while ( !err && (curFieldIdx <= maxFieldIdx) && (curPtr < endPtr) && !haveBitmap )
	{
		err = _DL_ISO8583_FIELD_Unpack(curFieldIdx,ioMsg,iHandler,&curPtr);

		if ( DL_ISO8583_IS_BITMAP(iHandler->fieldArr[curFieldIdx].fieldType) )
			haveBitmap = 1;

		curFieldIdx++;

	} /* end-while */

	/* unpack only present fields (if any) after bitmap field */
	while ( !err && (curFieldIdx <= maxFieldIdx) && (curPtr < endPtr) )
	{
		if ( 0 != ioMsg->field[curFieldIdx].len ) /* present */
		{
			err = _DL_ISO8583_FIELD_Unpack(curFieldIdx,ioMsg,iHandler,&curPtr);
		}

		curFieldIdx++;
	} /* end-while */

	/* check for under/over read condition */
	if ( !err && (curPtr != endPtr) )
		err = kDL_ERR_OTHER;

	return err;
}

/******************************************************************************/

void DL_ISO8583_MSG_Dump ( const char  *_iEolStr,
					       const DL_ISO8583_HANDLER *iHandler,
					       const DL_ISO8583_MSG     *iMsg )
{
	DL_UINT16 i;
	const char     *tmpEOL = _iEolStr == NULL ? "\n" : _iEolStr;

	//LogMessage(LogVerbose, "no of field items %i", iHandler->fieldItems);
	ShowLogs(1, "no of field items %i\n", iHandler->fieldItems);

	//LogMessage(LogVerbose,"%s--------------- ISO8583 MSG DUMP ---------------%s", tmpEOL,tmpEOL);
	ShowLogs(1, "%s--------------- ISO8583 MSG DUMP ---------------%s", tmpEOL,tmpEOL);
	
	/* for each field */
	for ( i=0 ; i<(iHandler->fieldItems) ; i++ )
	{
		if ( NULL != iMsg->field[i].ptr ) /* present */{
			DL_ISO8583_FIELD_DEF *fieldDef = DL_ISO8583_GetFieldDef(i,iHandler);
			//LogMessage(LogVerbose,"Field %03i, %s%s",(int)i, iMsg->field[i].ptr,tmpEOL);
			ShowLogs(1, "Field %03i, %s%s",(int)i, iMsg->field[i].ptr,tmpEOL);
		}

	} /* end-for(i) */

	//LogMessage(LogVerbose,"------------------------------------------------%s%s", tmpEOL,tmpEOL);
	ShowLogs(1, "------------------------------------------------%s%s\n", tmpEOL,tmpEOL);
	
	return;
}

/******************************************************************************/
