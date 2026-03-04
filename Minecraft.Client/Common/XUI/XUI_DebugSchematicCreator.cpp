#include "stdafx.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "XUI_DebugSchematicCreator.h"
#include "..\..\..\Minecraft.World\ChunkSource.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"

#ifndef _CONTENT_PACKAGE
HRESULT CScene_DebugSchematicCreator::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	MapChildControls();

	m_startX	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_startY	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_startZ	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_endX		.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_endY		.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_endZ		.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);

	m_data = new ConsoleSchematicFile::XboxSchematicInitParam();

	return S_OK;
}

HRESULT CScene_DebugSchematicCreator::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if ( hObjPressed == m_createButton )
	{
		// We want the start to be even
		if(m_data->startX > 0 && m_data->startX%2 != 0)
			m_data->startX-=1;
		else if(m_data->startX < 0 && m_data->startX%2 !=0)
			m_data->startX-=1;
		if(m_data->startY < 0) m_data->startY = 0;
		else if(m_data->startY > 0 && m_data->startY%2 != 0)
			m_data->startY-=1;
		if(m_data->startZ > 0 && m_data->startZ%2 != 0)
			m_data->startZ-=1;
		else if(m_data->startZ < 0 && m_data->startZ%2 !=0)
			m_data->startZ-=1;
	
		// We want the end to be odd to have a total size that is even
		if(m_data->endX > 0 && m_data->endX%2 == 0)
			m_data->endX+=1;
		else if(m_data->endX < 0 && m_data->endX%2 ==0)
			m_data->endX+=1;
		if(m_data->endY > Level::maxBuildHeight)
			m_data->endY = Level::maxBuildHeight;
		else if(m_data->endY > 0 && m_data->endY%2 == 0)
			m_data->endY+=1;
		else if(m_data->endY < 0 && m_data->endY%2 ==0)
			m_data->endY+=1;
		if(m_data->endZ > 0 && m_data->endZ%2 == 0)
			m_data->endZ+=1;
		else if(m_data->endZ < 0 && m_data->endZ%2 ==0)
			m_data->endZ+=1;

		wstring value = m_name.GetText();
		if(!value.empty())
		{
			swprintf(m_data->name,64,L"%ls", value.c_str());
		}
		else
		{
			swprintf(m_data->name,64,L"schematic");
		}

		m_data->bSaveMobs = m_saveMobs.IsChecked();

#ifdef _XBOX
		if (m_useXboxCompr.IsChecked())
			m_data->compressionType = Compression::eCompressionType_LZXRLE;
		else
#endif
			m_data->compressionType = Compression::eCompressionType_RLE;

		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(), eXuiServerAction_ExportSchematic, (void *)m_data);
		
		NavigateBack();
		rfHandled = TRUE;
	}
	return S_OK;
}

HRESULT CScene_DebugSchematicCreator::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_PAD_START:
	case VK_PAD_BACK:
		NavigateBack();

		rfHandled = TRUE;

		break;

	}

	return S_OK;
}

HRESULT CScene_DebugSchematicCreator::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled)
{
	if(hObjSource == m_startX)
	{
		int iVal = 0;
		wstring value = m_startX.GetText();
		if(!value.empty()) iVal = _fromString<int>( value );

		if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
		{
			m_data->startX = iVal;
		}
	}
	else if(hObjSource == m_startY)
	{
		int iVal = 0;
		wstring value = m_startY.GetText();
		if(!value.empty()) iVal = _fromString<int>( value );

		if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
		{
			m_data->startY = iVal;
		}
	}
	else if(hObjSource == m_startZ)
	{
		int iVal = 0;
		wstring value = m_startZ.GetText();
		if(!value.empty()) iVal = _fromString<int>( value );

		if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
		{
			m_data->startZ = iVal;
		}
	}
	else if(hObjSource == m_endX)
	{
		int iVal = 0;
		wstring value = m_endX.GetText();
		if(!value.empty()) iVal = _fromString<int>( value );

		if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
		{
			m_data->endX = iVal;
		}
	}
	else if(hObjSource == m_endY)
	{
		int iVal = 0;
		wstring value = m_endY.GetText();
		if(!value.empty()) iVal = _fromString<int>( value );

		if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
		{
			m_data->endY = iVal;
		}
	}
	else if(hObjSource == m_endZ)
	{
		int iVal = 0;
		wstring value = m_endZ.GetText();
		if(!value.empty()) iVal = _fromString<int>( value );

		if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
		{
			m_data->endZ = iVal;
		}
	}
	return S_OK;
}
#endif