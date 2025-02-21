A stripped down version of HolyLib's gameserver module to allow for a queue system.  

# API  
This is the Lua API that is provided.  

### Functions

#### CGameClient gameserver.GetClient(number playerSlot)
Returns the CGameClient at that player slot or `nil` on failure.  

#### table gameserver.GetAll()
Returns a table that contains all game clients. It will return `nil` on failure.  

### CBaseClient
This class represents a client.

#### CBaseClient:\_\_newindex(string key, any value)
Internally implemented and will set the values into the lua table.  

#### any CBaseClient:\_\_index(string key)
Internally seaches first in the metatable table for the key.  
If it fails to find it, it will search in the lua table before returning.  
If you try to get multiple values from the lua table, just use `CBaseClient:GetTable()`.  

#### table CBaseClient:GetTable()
Returns the lua table of this object.  
You can store variables into it.  

#### number CBaseClient:GetPlayerSlot()
Returns the slot of the client. Use this for `sourcetv.GetClient`.  

#### number CBaseClient:GetUserID()
Returns the userid of the client.  

#### string CBaseClient:GetName()
Returns the name of the client.

#### string CBaseClient:GetSteamID()
Returns the steamid of the client.

#### CBaseClient:Reconnect()
Reconnects the client.

#### CBaseClient:ClientPrint(string message)
Prints the given message into the client's console.  

> [!NOTE]
> It **won't** add `\n` to the end, you have to add it yourself. 

#### bool CBaseClient:IsValid()
Returns `true` if the client is still valid.  

#### CBaseClient:Disconnect(string reason)
Disconnects the client.  

#### bool CBaseClient:IsConnected()

#### bool CBaseClient:IsSpawned()

#### bool CBaseClient:IsActive()

#### number CBaseClient:GetSignonState()

#### bool CBaseClient:IsFakeClient()

#### bool CBaseClient:SetSignonState(number signOnState, number spawnCount = 0, bool rawSet = false)
Sets the SignOnState for the given client.  
Returns `true` on success.  

> [!NOTE]
> This function does normally **not** directly set the SignOnState.  
> Instead it calls the responsible function for the given SignOnState like for `SIGNONSTATE_PRESPAWN` it will call `SpawnPlayer` on the client.  
> Set the `rawSet` to `true` if you want to **directly** set the SignOnState.  

> [!NOTE]
> This function was formerly known as `HolyLib.SetSignOnState`

### CGameClient
This class inherits CBaseClient.

#### string CGameClient:\_\_tostring()
Returns the a formated string.  
Format: `CGameClient [%i][%s]`  
`%i` -> UserID  
`%s` -> ClientName  

### Hooks

#### bool HolyLib:OnSetSignonState(CGameClient client, number state, number spawnCount)
Called when the engine is about to change the client's SignonState.  
Return `true` to stop the engine.  

#### HolyLib:OnPlayerChangedSlot(number oldPlayerSlot, number newPlayerSlot)
Called **after** a player was moved to a different slot.  
This happens when a player on a player slot above 128 tries to spawn.  

Why is this done? Because any player above 128 is utterly unstable and can only stabily exist as a CGameClient.  
if a CBasePlayer entity is created on a slot above 128 expect stability issues!

#### HolyLib:OnClientDisconnect(CGameClient client)
Called when a client disconnects.

### Player Queue System
Using this module's functionality you can implement a player queue were players wait in the loading screen until they spawn when a slot gets free.

Example implementation:
```lua
playerQueue = playerQueue or {
	count = 0
}

hook.Add("HolyLib:OnSetSignonState", "Example", function(cl, state, c)
	print(cl, state, c)

	local fullServer = #player.GetAll() >= 128 -- Can't exceed 128 players.
	if fullServer and state == SIGNONSTATE_PRESPAWN then -- REQUIRED to be SIGNONSTATE_PRESPAWN
		if not playerQueue[cl] then
			playerQueue[cl] = true
			playerQueue.count = playerQueue.count + 1
			playerQueue[playerQueue.count] = cl
		end

		return true -- Stop the engine from continuing/spawning the player
	end
end)

hook.Add("HolyLib:OnClientDisconnect", "Example", function(client)
	timer.Simple(0, function() -- Just to be sure that the client was really disconnected.
		if playerQueue.count <= 0 then return end

		if client:IsValid() then
			print("Client isn't empty?!? client: " .. client)
			return
		end

		local nextPlayer = playerQueue[1]
		playerQueue[nextPlayer] = nil
		table.remove(playerQueue, 1)
		playerQueue.count = playerQueue.count - 1

		nextPlayer:SpawnPlayer() -- Spawn the client, HolyLib handles the moving of the client.
	end)
end)

hook.Add("HolyLib:OnPlayerChangedSlot", "Example", function(oldPlayerSlot, newPlayerSlot)
	print("Client was moved from slot " .. oldPlayerSlot .. " to slot " .. newPlayerSlot)
end)
```