#include "netmessages.h"
#include "bitbuf.h"
#include "tier0/vprof.h"
#include "net.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static char s_text[1024];

bool SVC_ServerInfo::WriteToBuffer( bf_write &buffer )
{
	buffer.WriteUBitLong( GetType(), NETMSG_TYPE_BITS );
	buffer.WriteShort ( m_nProtocol );
	buffer.WriteLong  ( m_nServerCount );
	buffer.WriteOneBit( m_bIsHLTV?1:0);
	buffer.WriteOneBit( m_bIsDedicated?1:0);
	buffer.WriteLong  ( 0xffffffff );  // Used to be client.dll CRC.  This was far before signed binaries, VAC, and cross-platform play
	buffer.WriteWord  ( m_nMaxClasses );
	buffer.WriteBytes( m_nMapMD5.bits, MD5_DIGEST_LENGTH );		// To prevent cheating with hacked maps
	buffer.WriteByte  ( m_nPlayerSlot );
	buffer.WriteByte  ( m_nMaxClients );
	buffer.WriteFloat ( m_fTickInterval );
	buffer.WriteChar  ( m_cOS );
	buffer.WriteString( m_szGameDir );
	buffer.WriteString( m_szMapName );
	buffer.WriteString( m_szSkyName );
	buffer.WriteString( m_szHostName );

#if defined( REPLAY_ENABLED )
	buffer.WriteOneBit( m_bIsReplay?1:0);
#endif

	return !buffer.IsOverflowed();
}

bool SVC_ServerInfo::ReadFromBuffer( bf_read &buffer )
{
	VPROF( "SVC_ServerInfo::ReadFromBuffer" );

	m_szGameDir = m_szGameDirBuffer;
	m_szMapName = m_szMapNameBuffer;
	m_szSkyName = m_szSkyNameBuffer;
	m_szHostName = m_szHostNameBuffer;

	m_nProtocol		= buffer.ReadShort();
	m_nServerCount	= buffer.ReadLong();
	m_bIsHLTV		= buffer.ReadOneBit()!=0;
	m_bIsDedicated	= buffer.ReadOneBit()!=0;
	buffer.ReadLong();  // Legacy client CRC.
	m_nMaxClasses	= buffer.ReadWord();

	// Prevent cheating with hacked maps
	if ( m_nProtocol > PROTOCOL_VERSION_17 )
	{
		buffer.ReadBytes( m_nMapMD5.bits, MD5_DIGEST_LENGTH );
	}
	else
	{
		m_nMapCRC	= buffer.ReadLong();
	}

	m_nPlayerSlot	= buffer.ReadByte();
	m_nMaxClients	= buffer.ReadByte();
	m_fTickInterval	= buffer.ReadFloat();
	m_cOS			= buffer.ReadChar();
	buffer.ReadString( m_szGameDirBuffer, sizeof(m_szGameDirBuffer) );
	buffer.ReadString( m_szMapNameBuffer, sizeof(m_szMapNameBuffer) );
	buffer.ReadString( m_szSkyNameBuffer, sizeof(m_szSkyNameBuffer) );
	buffer.ReadString( m_szHostNameBuffer, sizeof(m_szHostNameBuffer) );

#if defined( REPLAY_ENABLED )
	// Only attempt to read the 'replay' bit if the net channel's protocol
	// version is greater or equal than the protocol version for replay's release.
	// INetChannel::GetProtocolVersion() will return PROTOCOL_VERSION for
	// a regular net channel, or the network protocol version from the demo
	// file, if we're playing back a demo.
	if ( m_NetChannel->GetProtocolVersion() >= PROTOCOL_VERSION_REPLAY )
	{
		m_bIsReplay = buffer.ReadOneBit() != 0;
	}
#endif

	return !buffer.IsOverflowed();
}

const char *SVC_ServerInfo::ToString(void) const
{
	Q_snprintf(s_text, sizeof(s_text), "%s: game \"%s\", map \"%s\", max %i", GetName(), m_szGameDir, m_szMapName, m_nMaxClients );
	return s_text;
}