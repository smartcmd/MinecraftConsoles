#include "stdafx.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\ClientConnection.h"
#include "..\..\Common\GameRules\ConsoleGameRules.h"
#include "XUI_DebugItemEditor.h"

#ifdef _DEBUG_MENUS_ENABLED
HRESULT CScene_DebugItemEditor::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	MapChildControls();

	ItemEditorInput *initData = static_cast<ItemEditorInput *>(pInitData->pvInitData);
	m_iPad = initData->iPad;
	m_slot = initData->slot;
	m_menu = initData->menu;
	if(m_slot != nullptr) m_item = m_slot->getItem();

	if(m_item!=nullptr)
	{
		m_icon->SetIcon(m_iPad, m_item->id,m_item->getAuxValue(),m_item->count,10,31,false,m_item->isFoil());
		m_itemName.SetText( app.GetString( Item::items[m_item->id]->getDescriptionId(m_item) ) );

		m_itemId		.SetText( std::to_wstring(m_item->id).c_str() );
		m_itemAuxValue	.SetText( std::to_wstring(m_item->getAuxValue()).c_str() );
		m_itemCount		.SetText( std::to_wstring(m_item->count).c_str() );
		m_item4JData	.SetText( std::to_wstring(m_item->get4JData()).c_str() );
	}

	m_itemId		.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_itemAuxValue	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_itemCount		.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);
	m_item4JData	.SetKeyboardType(C_4JInput::EKeyboardMode_Numeric);

	m_generatedXml.SetText( CollectItemRuleDefinition::generateXml(m_item).c_str() );

	delete initData;

	return S_OK;
}

HRESULT CScene_DebugItemEditor::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_PAD_START:
	case VK_PAD_BACK:
		// We need to send a packet to the server to update it's representation of this item
		if(m_slot != nullptr && m_menu != nullptr)
		{
			m_slot->set(m_item);

			Minecraft *pMinecraft = Minecraft::GetInstance();
			shared_ptr<MultiplayerLocalPlayer> player = pMinecraft->localplayers[m_iPad];
			if(player != nullptr && player->connection) player->connection->send(std::make_shared<ContainerSetSlotPacket>(m_menu->containerId, m_slot->index, m_item));
		}
		// kill the crafting xui
		app.NavigateBack(m_iPad);

		rfHandled = TRUE;

		break;

	}

	return S_OK;
}

HRESULT CScene_DebugItemEditor::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled)
{
	if(m_item == nullptr) m_item = std::make_shared<ItemInstance>(0, 1, 0);
	if(hObjSource == m_itemId)
	{
		int id = 0;
		wstring value = m_itemId.GetText();
		if(!value.empty()) id = _fromString<int>( value );

		// TODO Proper validation of the valid item ids
		if(id > 0 && Item::items[id] != nullptr) m_item->id = id;
	}
	else if(hObjSource == m_itemAuxValue)
	{
		int auxVal = 0;
		wstring value = m_itemAuxValue.GetText();
		if(!value.empty()) auxVal = _fromString<int>( value );
		if(auxVal >= 0) m_item->setAuxValue( auxVal );
	}
	else if(hObjSource == m_itemCount)
	{
		int count = 0;
		wstring value = m_itemCount.GetText();
		if(!value.empty()) count = _fromString<int>( value );
		if(count > 0 && count <= Item::items[m_item->id]->getMaxStackSize()) m_item->count = count;
	}
	else if(hObjSource == m_item4JData)
	{
		int data = 0;
		wstring value = m_item4JData.GetText();
		if(!value.empty()) data = _fromString<int>( value );
		m_item->set4JData(data);
	}

	m_icon->SetIcon(m_iPad, m_item->id,m_item->getAuxValue(),m_item->count,10,31,false,m_item->isFoil());

	m_itemName.SetText( app.GetString( Item::items[m_item->id]->getDescriptionId(m_item) ) );

	m_generatedXml.SetText( CollectItemRuleDefinition::generateXml(m_item).c_str() );
	return S_OK;
}
#endif