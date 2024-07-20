# API  
This is the Lua API that is provided.  

### Hooks
All hooks call hook.Run. not gamemode.Call!  

#### bool `PlayerQueue:OnSetSignonState`( number slot, number state, number spawncount )
number slot - The Slot which you can use to call `PlayerQueue.SetSignOnState`.  
Its a number that counts up with each hook call and will become invalid if we don't return true.  

number state - The SignOnState -> https://wiki.facepunch.com/gmod/Enums/SIGNONSTATE  
number spawncount - I have no idea :/  

### Library functions ( PlayerQueue )  

#### bool PlayerQueue.SetSignOnState( number slot, number state )  
number slot - The slot that `PlayerQueue:OnSetSignonState` provided.  
number state - The SignOnState we want to set.  

returns:  
bool success - `true` If the SignOnState was successfully set  

#### string PlayerQueue.GetSteamID( number slot )  
number slot - The slot that `PlayerQueue:OnSetSignonState` provided.  

returns:  
string steamid - The steamid of that client. or nil on failure.