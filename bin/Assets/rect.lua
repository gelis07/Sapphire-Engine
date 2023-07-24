local SaphireEngine = require("SaphireEngine")

aha = "hello"
speed = 3.0
ForcePower = 10.0
-- hi = ":)"
local dir = {x=0, y=0}

function OnStart()

end


function OnUpdate()
    if(SaphireEngine.KeyPress("right"))
    then
        dir.x = 1
    elseif(SaphireEngine.KeyPress("left"))
        then
        dir.x = -1
    else
        dir.x = 0
    end
    if(SaphireEngine.KeyPress("up"))
    then
        dir.y = 1
    elseif(SaphireEngine.KeyPress("down"))
        then
        dir.y = -1
    else
        dir.y = 0
    end

    this:Move(speed, dir.x, dir.y)
end


function OnCollision()
    -- SaphireEngine.Log("test")
end