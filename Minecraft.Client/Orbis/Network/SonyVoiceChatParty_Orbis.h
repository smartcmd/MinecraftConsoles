#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <np\np_party.h>

class SonyVoiceChatParty_Orbis
{
public:

	static void init();
	static void shutdown();
	static void tick();
	static void setEnabled(bool bEnabled);
	static bool hasMicConnected(const PlayerUID& memberUID);
	static bool isTalking( const PlayerUID& memberUID);
	static void mute(bool bMute);			//Turn chat audio on or off
	static void mutePlayer(const SceNpMatching2RoomMemberId member_id, bool bMute);		//Turn chat audio from a specified player on or off;
	static void muteLocalPlayer(bool bMute); //Turn microphone input on or off;

	static bool isMuted();
	static bool isMutedPlayer(const PlayerUID& memberUID);
	static bool isMutedLocalPlayer(); //Turn microphone input on or off;


private:
	//NP Party Event Handlers
	static void partyRoomEventHandler(SceNpPartyRoomEventType eventType,const void* data,	void* userdata);
	static void partyVoiceEventHandler(const SceNpPartyMemberVoiceInfo* memberVoiceInfo,void* userdata);
	static void partyBinaryMessageEventHandler(SceNpPartyBinaryMessageEvent event, const void *data, void * userdata);

	class MemberInfo
	{
	public:
		SceNpId					m_NpId;
		SceNpPartyRoomMemberId  m_roomID;
		bool					m_voiceConnected;
		bool					m_voiceMuted;
		DWORD					m_lastTimeTalking;
		bool					m_localUser;
		bool					m_partyLeader;
		MemberInfo() 
		{
			m_voiceConnected = false;
			m_voiceMuted = false;
			m_lastTimeTalking = 0;
			m_localUser = false;
			m_partyLeader = false;
		}
	};
	class PartyInfo
	{
	public:
		int						m_numMembers;
		MemberInfo				m_members[SCE_NP_PARTY_MEMBER_NUM_MAX];
		bool					m_privateParty;

		MemberInfo*	getMember(SceNpPartyRoomMemberId roomID) 
		{
			for(int i=0;i<m_numMembers;i++)
			{
				if(m_members[i].m_roomID == roomID)
					return &m_members[i];
			}
			return NULL;
		}
		MemberInfo*	getMember(const PlayerUID& memberUID) 
		{
			for(int i=0;i<m_numMembers;i++)
			{
				if(strcmp(m_members[i].m_NpId.handle.data, memberUID.getOnlineID()) == 0)
					return &m_members[i];
			}
			return NULL;
		}


		PartyInfo() : m_numMembers(0) {}
		void addMember(SceNpPartyRoomMemberId roomID, SceNpId npID, bool local, bool leader)
		{
			m_members[m_numMembers].m_roomID = roomID;
			m_members[m_numMembers].m_NpId = npID;
			m_members[m_numMembers].m_localUser = local;
			m_members[m_numMembers].m_partyLeader = leader;
			m_numMembers++;
		}

		void removeMember(SceNpPartyRoomMemberId roomID)
		{
			// find the index of this member first 
			int index = -1;
			for(int i=0;i<m_numMembers;i++)
			{
				if(roomID == m_members[i].m_roomID)
				{
					index = i;
					break;
				}
			}
			assert(index >= 0);

			m_numMembers--;
			if(m_numMembers > 0)
				m_members[index] = m_members[m_numMembers];
		}
		void clear() { m_numMembers = 0;}
	};
	static PartyInfo m_partyInfo;
};