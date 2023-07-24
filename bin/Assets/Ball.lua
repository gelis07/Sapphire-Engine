local SaphireEngine = require("SaphireEngine")


local dir = {x=0, y=0}
speed = 3
function OnStart()
    dir.x = 0.5
    dir.y = -1
end
function OnUpdate()
    this:Move(speed, dir.x, dir.y)
end
function OnCollision()
    Cy = obj.position.y - obj.size.y/2
    Ay = obj.position.y + obj.size.y/2
    Ax = obj.position.x - obj.size.x/2
    Bx = obj.position.x + obj.size.x/2

    Xpos = SaphireEngine.Clamp(Ax, Bx);
    Ypos = SaphireEngine.Clamp(Cy, Ay);
    CollisionPoint = {x = Xpos, y = Ypos}
    SaphireEngine.Log(tostring(this.dir.x))
    SaphireEngine.Log(tostring(this.dir.y))
    if(CollisionPoint.y == Ay or CollisionPoint.y == Cy)
        then
            dir.x = dir.x * -1
            dir.y = dir.y * -1
            dir.x = dir.x * -1
            SaphireEngine.Log(tostring(this.dir.x))
            SaphireEngine.Log(tostring(this.dir.y))
        elseif(CollisionPoint.y ~= Ay or CollisionPoint.y ~= Cy)
            then
                dir.x = dir.x * -1
                dir.y = dir.y * -1
                dir.y = dir.y * -1
                SaphireEngine.Log(tostring(dir.x))
                SaphireEngine.Log(tostring(dir.y))
    end

end